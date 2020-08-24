#include <libwyvern/wyvern.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <random>
#include <regex>
#include <atomic>
#include <variant>

#include <nlohmann/json.hpp>
#include <libbutl/process.mxx>
#include <libbutl/filesystem.mxx>
#include <libbutl/fdstream.mxx>
#include <libbutl/string-parser.mxx>
#include <fmt/format.h>

using json = nlohmann::json;
using fmt::format; // could be std::format if support is available

namespace wyvern {
namespace {

  const auto target_prefix = "wyvern_";


  struct failure : std::runtime_error
  {
    using std::runtime_error::runtime_error;
  };


  std::atomic<bool> is_logging_enabled{ false };

  struct Logger
  {
    std::stringstream logged;

    Logger()
    {
      if(is_logging_enabled)
        logged << "wyvern: ";
    }
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = default;
    Logger& operator=(Logger&&) = default;

    template<class Arg>
    friend auto operator<<(Logger&& logger, Arg&& to_log)
      -> Logger&&
    {
      if(is_logging_enabled)
        logger.logged << std::forward<Arg>(to_log);
      return std::move(logger);
    }

    ~Logger()
    {
      if(is_logging_enabled)
        std::cout << logged.str() <<std::endl;
    }
  };

  auto log() -> Logger { return {}; }

  int random_int(int min_value, int max_value){
    static std::random_device device;
    static std::seed_seq seed{device(), device(), device(), device(), device(), device(), device(), device()};
    static std::default_random_engine engine(seed);
    std::uniform_int_distribution<int> distribution(min_value, max_value);
    return distribution(engine);
  }

  std::string to_lower_case(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](auto c){ return std::tolower(c); });
    return s;
}

  auto normalize_name(const std::string& name) -> std::string
  {
    // NOTE: will not work with unicode.....
    const auto lowercase_name = to_lower_case(name);

    static const auto regex_string = R"regex([\s | - | \. | \: | \( | \) )]+)regex";
    static const auto replacement = "_";
    static const std::regex to_replace(regex_string);

    const auto normalized_name = std::regex_replace(lowercase_name, to_replace, replacement);
    log() << format("normalized \"{}\" to \"{}\"", name, normalized_name);
    return normalized_name;
  }

  auto parse_values(const std::string& values) -> std::vector<std::string>
  {
    return butl::string_parser::parse_quoted(values, true);
  }

  template<class T>
  void append(std::vector<T>& values, const std::vector<T>& other_values)
  {
    values.insert(values.end(), other_values.begin(), other_values.end());
  }

  template<class Range>
  auto sort(Range& range)
  {
    using std::begin;
    using std::end;
    return std::sort(begin(range), end(range));
  }

  auto difference(const std::vector<std::string>& left, const std::vector<std::string>& right)
  {
    std::vector<std::string> diff;
    std::set_difference(left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(diff));
    return diff;
  }

  auto escape_braces(std::string text) -> std::string
  {
    static const auto left_brace_regex = std::regex("[{]");
    static const auto right_brace_regex = std::regex("[}]");
    text = std::regex_replace(text, left_brace_regex, "{{");
    text = std::regex_replace(text, right_brace_regex, "}}");
    return text;
  }

  auto write_to_file(path file_path, const std::string& content) -> void
  {
    log() << format("writing into file {}", file_path.normalize(true, true).string());
    using namespace butl;
    static const auto open_mode = fdopen_mode::truncate | fdopen_mode::create;
    ofdstream file { file_path, open_mode };
    file << content; // Assuming we are in text mode.
    file.close(); // Throws exceptions if there have been errors while writing.
  }

  auto create_directories(dir_path directory_path) -> void
  {
    log() << format("creating directories {}", directory_path.normalize(true, true).string());
    const auto result = butl::try_mkdir_p(directory_path);
    if(result != butl::mkdir_status::success){
      throw failure(format("Failed to create directory {}", directory_path.normalize(true, true).string()));
    }
  }

  json read_json_file(path file_path)
  {
    using namespace butl;
    ifdstream file { file_path };
    const json json_content = json::parse(file.read_text());
    return json_content;
  }

}}

namespace wyvern::cmake {

  auto invoke_cmake(const std::vector<std::string>& args) -> void
  {
    // run the command cmake
    // throw if any error is found
    log() << format("cmake {}", fmt::join(args, " "));
    std::vector<const char*> command{ "cmake" };
    for(const auto& arg : args)
    {
      command.push_back(arg.c_str());
    }
    command.push_back(nullptr);

    struct Pipes{
      int in = 0;
      int out = 1;
      int err = 2;
    };
    auto pipes = []()->Pipes{
      if(is_logging_enabled)
        return {};
      else
        return { 0, -2, -2 };
    }();

    butl::process cmake_process(command.data(), pipes.in, pipes.out, pipes.err);
    if(!cmake_process.wait())
    {
      throw failure("CMake process failed");
    }
  }
namespace {
  constexpr auto minimum_cmake_version = "3.10";

  enum class cmakefile_mode
  {
    without_dependencies,
    with_dependencies,
  };

  auto generate_cmakefile_code(const Configuration& cmake_config, cmakefile_mode mode)
    -> std::string // content of the CMakeFile.txt
  {
    // TODO: replace by fmt::printf(filedesc, "...", ...);
    std::stringstream code;
    code << format("cmake_minimum_required(VERSION {})\n\n", minimum_cmake_version);
    code << format("project(wyvern_{})\n\n", random_int(0, 99999999));

    if(mode == cmakefile_mode::with_dependencies)
    {
      for(const auto& package : cmake_config.packages)
      {
        code << format("find_package({} {} REQUIRED {})\n\n", package.name, package.version, fmt::join(package.constraints, " "));
      }
    }

    for(const auto& target : cmake_config.targets)
    {
      const auto suffix = normalize_name(target);
      const auto target_name = format("{}{suffix}", target_prefix, fmt::arg("suffix", suffix));
      code << format("add_executable({} main_{suffix}.cpp header_{suffix}.hpp)\n", target_name, fmt::arg("suffix", suffix));
      if(mode == cmakefile_mode::with_dependencies)
      {
        code << format("target_link_libraries({} PRIVATE {})\n", target_name, target);
      }
    }
    code << "\n\n";
    return code.str();
  }

  auto create_cmake_project(dir_path directory_path, const Configuration& cmake_config, cmakefile_mode mode, std::string test_code_format)
    -> void
  {
    // 1. create main.cpp and header.hpp
    static constexpr auto main_content = R"cpp(
#include "header_{target_name}.hpp"
{}
int main() {{  }}
    )cpp";

    static constexpr auto header_content = R"cpp(
#pragma once
// This is a header to check the output with a header file (which should not be compiled).
    )cpp";

    for(const auto& target : cmake_config.targets){
      const auto target_name = normalize_name(target);
      const auto main_cpp_path = directory_path / path(format("main_{}.cpp", target_name));
      const auto header_hpp_path = directory_path / path(format("header_{}.hpp", target_name));

      const auto code_to_inject = [&]()-> std::string {
        if(mode == cmakefile_mode::without_dependencies)
          return {};

        auto code = test_code_format;
        if(!code.empty())
        {
          code = format(code, fmt::arg("target_name", target_name));
          code = escape_braces(code);
        }
        return code;
      }();

      const auto main_cpp_code = format(main_content, code_to_inject, fmt::arg("target_name", target_name));
      write_to_file(main_cpp_path, main_cpp_code);
      write_to_file(header_hpp_path, header_content);
    }

    // 2. create the cmakefile with the right content
    const auto cmakefile_path = directory_path / path("CMakeLists.txt");
    const auto cmakefile_content = generate_cmakefile_code(cmake_config, mode);
    write_to_file(cmakefile_path, cmakefile_content);
  }

  struct CodeModel
  {
    json index;
    struct Configuration
    {
      std::map<std::string, json> targets;
    };
    std::map<std::string, Configuration> configs;
  };

  auto find_index_file_path(dir_path reply_dir)
  {
    // There can be only 1 json file starting with "index" in that directory.
    path found_path;
    butl::path_search(path("index-*.json"), [&](path path, const std::string&, bool){
      found_path = path;
      return false;
    }, reply_dir);

    if(found_path.empty())
      throw failure(format("Failed to find index json file in {}", reply_dir.normalize(true, true).string()));
    return (reply_dir / found_path).normalize(true, true);
  }

  auto read_cmake_api_reply_json(dir_path reply_dir)
    -> CodeModel
  {
    CodeModel codemodel;
    // 1. read the index to find the right codemodel file
    const auto index_path = find_index_file_path(reply_dir);
    codemodel.index = read_json_file(index_path);
    // log() << "INDEX : " << codemodel.index;

    // 2. read the codemodel file to find the right target files
    const auto codemodel_filename = codemodel.index["reply"]["client-wyvern"]["query.json"]["responses"][0]["jsonFile"];
    const auto codemodel_path = reply_dir / path(codemodel_filename);
    // log() << "CodeModel file : " << codemodel_path.string();
    const auto codemodel_info = read_json_file(codemodel_path);


    // 3. gather information about each target.
    for(const auto& config : codemodel_info["configurations"])
    {
      CodeModel::Configuration config_info;
      const auto config_name = config["name"];
      for(const auto& target : config["targets"])
      {
        auto target_name = target["name"];
        if(target_name == "ALL_BUILD" || target_name == "ZERO_CHECK") // Skip targets generated by CMake for convenience.
          continue;

        const auto target_file = target["jsonFile"];
        const auto target_path = reply_dir / path(target_file);
        auto target_json = read_json_file(target_path);
        config_info.targets[target_name] = std::move(target_json);
      }
      codemodel.configs[config_name] = std::move(config_info);
    }
    return codemodel;
  }

  auto query_cmake_file_api(dir_path build_directory_path)
    -> CodeModel
  {

    static constexpr auto cmake_file_api_query_json = R"JSON(
{
  "requests" : [
      {
          "kind": "codemodel",
          "version": { "major": 2 }
      }
  ]
}
    )JSON";

    // 1. write the query file in the build directory
    const dir_path query_directory_path = build_directory_path / dir_path(".cmake/api/v1/query/client-wyvern/");
    create_directories(query_directory_path);

    const path query_file_path = query_directory_path / "query.json";
    write_to_file(query_file_path, cmake_file_api_query_json);

    // 2. invoke cmake in that directory
    invoke_cmake({ build_directory_path.normalize(true, true).string() });
    invoke_cmake({ "--build", build_directory_path.normalize(true, true).string() }); // Build to be sure it works

    // 3. retrieve the JSON information
    const dir_path reply_directory_path = build_directory_path / dir_path(".cmake/api/v1/reply/");
    return read_cmake_api_reply_json(reply_directory_path);
  }

  auto configure_project(dir_path project_path, dir_path build_path, const Configuration& cmake_config)
    -> void
  {
    const auto source_arg = project_path.normalize(true, true).string();
    const auto build_dir_arg = build_path.normalize(true, true).string();

    auto args = cmake_config.args;
    for(const auto& option : cmake_config.options)
    {
      args.push_back(format("-D{}={}", option.first, option.second));
    }

    if(!cmake_config.generator.empty())
    {
      args.insert(args.end(), { "-G", cmake_config.generator } );
    }

    args.insert(args.end(), { "-S", source_arg, "-B", build_dir_arg });


    invoke_cmake(args);
  }


}}

namespace wyvern
{
  scoped_temp_dir::scoped_temp_dir(bool keep_directory)
      : path_(dir_path::temp_path("wyvern").normalize(true, true)), keep_directory(keep_directory)
  {
    create_directories(path_);
  }

  scoped_temp_dir::~scoped_temp_dir()
  {
    if (!path_.empty())
    {
      if (keep_directory)
      {
        log() << format("DIRECTORY NOT DELETED: {}", path_.normalize(true, true).string());
      }
      else
      {
        butl::rmdir_r(path_);
        log() << format("Deleted directory {}", path_.normalize(true, true).string());
      }
    }
  }

  scoped_temp_dir::scoped_temp_dir(scoped_temp_dir &&other)
      : path_(std::move(other.path_))
  {
    other.path_.clear();
  }

  scoped_temp_dir &scoped_temp_dir::operator=(scoped_temp_dir &&other)
  {
    this->path_ = std::move(other.path_);
    other.path_.clear();
    return *this;
  }


  std::ostream& operator<<(std::ostream& out, const DependenciesInfo& deps)
  {
    out << "Dependencies Info:\n";
    for(const auto& [config_name, config] : deps.configurations)
    {
      out << format("  Configuration: {}\n", config_name);
      for(const auto& [target_name, target] : config.targets)
      {
        out << format("      Target: {}\n", target_name);
        for(const auto& [language, compilation]: target.language_compilation)
        {
          out << format("        -> {}\n", language);
          for(const auto& include_dir : compilation.include_directories)
          {
            out << format("          include dir: {}\n", include_dir);
          }
          for(const auto& define : compilation.defines)
          {
            out << format("          define: {}\n", define);
          }
          for(const auto& flag : compilation.compilation_flags)
          {
            out << format("          compile flag: {}\n", flag);
          }
          for(const auto& source : compilation.source_files)
          {
            out << format("          source: {}\n", source);
          }
        }
        for(const auto& lib_dir : target.libraries_directories)
        {
          out << format("        library dir: {}\n", lib_dir);
        }
        for(const auto& lib : target.link_libraries)
        {
          out << format("        library: {}\n", lib);
        }
        for(const auto& flag : target.link_flags)
        {
          out << format("        link flag: {}\n", flag);
        }
      }
    }

    return out;
  }

  namespace
  {

    auto extract_codemodel(const cmake::Configuration& config, cmake::cmakefile_mode mode, Options options)
        -> cmake::CodeModel
    {
      // step 1
      const scoped_temp_dir project_dir{options.keep_generated_projects};
      cmake::create_cmake_project(project_dir.path(), config, mode, options.code_format_to_inject_in_client);

      // step 2
      const auto generator_name = config.generator.empty() ? std::string("default-generator") : normalize_name(config.generator);
      const auto build_dir_name = format("build-{}", generator_name);
      const auto build_dir_path = (project_dir.path() / dir_path(build_dir_name)).normalize(true, true);
      cmake::configure_project(project_dir.path(), build_dir_path, config);

      // step 3
      const auto codemodel = cmake::query_cmake_file_api(build_dir_path);

      return codemodel;
    }

    auto log_codemodel(std::string_view name, const cmake::CodeModel& codemodel) -> void
    {
      log() << format("#### CODEMODEL {} : ####", name);
      for (auto [config_name, config] : codemodel.configs)
      {
        log() << format(" - Configuration : {}", config_name);
        for (auto [target_name, target_json] : config.targets)
        {
          log() << format("   - Target: {}", target_name);
          auto compilation_groups = target_json["compileGroups"];
          for (auto compile_info : compilation_groups)
          {
            log() << format("    => {}", compile_info["language"]);
            auto compile_fragments = compile_info["compileCommandFragments"];
            if (!compile_fragments.is_null())
              for (auto compile_flags : compile_fragments)
              {
                log() << format("       compilation flags: {}", compile_flags["fragment"]);
              }

            auto defines_fragments = compile_info["defines"];
            if (!defines_fragments.is_null())
              for (auto define : defines_fragments)
              {
                log() << format("       define: {}", define["define"]);
              }

            auto include_dirs = compile_info["includes"];
            if (!include_dirs.is_null())
              for (auto include_dir : include_dirs)
              {
                const bool is_system = include_dir["isSystem"];
                const auto path = include_dir["path"];
                log() << format("       include dir{}: {}", (is_system ? " (system)" : ""), path);
              }
          }

          const auto &link_info = target_json["link"];
          if (!link_info.is_null())
          {
            const auto &link_cmd_fragments = link_info["commandFragments"];
            if (!link_cmd_fragments.is_null())
              for (const auto &link_flags : link_cmd_fragments)
              {
                const auto &role = link_flags["role"];
                if (role == "flags")
                {
                  log() << format("       link flags: {}", link_flags["fragment"]);
                }
                else if (role == "libraries")
                {
                  log() << format("       link libraries: {}", link_flags["fragment"]);
                }
                else
                {
                  log() << format("       failed to read link info (unknown role): {}", link_flags.dump());
                }
              }
          }
        }
      }
    }

    auto extract_compilation(json compile_info) -> Compilation
    {
      Compilation compilation;
      auto compile_fragments = compile_info["compileCommandFragments"];
      if (!compile_fragments.is_null())
      {
        for (auto compile_flags : compile_fragments)
        {
          const auto flags = parse_values(compile_flags["fragment"]);
          append(compilation.compilation_flags, flags);
        }
        sort(compilation.compilation_flags);
      }


      auto defines_fragments = compile_info["defines"];
      if (!defines_fragments.is_null())
      {
        for (auto define : defines_fragments)
        {
          compilation.defines.push_back(define["define"]);
        }
        sort(compilation.defines);
      }

      auto include_dirs = compile_info["includes"];
      if (!include_dirs.is_null())
      {
        for (auto include_dir : include_dirs)
        {
          const bool is_system = include_dir["isSystem"]; // TODO: decide if we need to keep that info
          const auto path = include_dir["path"];
          compilation.include_directories.push_back(path);
        }
        sort(compilation.include_directories);
      }

      return compilation;
    }

    auto extract_target(json target_json) -> Target
    {
      Target target;
      target.name = target_json["name"];
      auto compilation_groups = target_json["compileGroups"];
      for (auto compile_info : compilation_groups)
      {
        const auto compile_language = compile_info["language"];
        target.language_compilation[compile_language] = extract_compilation(compile_info);
      }

      const auto &link_info = target_json["link"];
      if (!link_info.is_null())
      {
        const auto &link_cmd_fragments = link_info["commandFragments"];
        if (!link_cmd_fragments.is_null())
        {
          for (const auto &link_flags : link_cmd_fragments)
          {
            const auto &role = link_flags["role"];
            if (role == "flags")
            {
              const auto flags = parse_values(link_flags["fragment"]);
              append(target.link_flags, flags);
            }
            else if (role == "libraries")
            {
              const auto libs = parse_values(link_flags["fragment"]);
              append(target.link_libraries, libs);
            } // TODO: for some reason there is no library directories, the paths to the libraries is always complete?
            else
            {
              throw failure(format("failed to read link info (unknown role): {}", link_flags.dump()));
            }
          }
          sort(target.link_flags);
          sort(target.link_libraries);
          sort(target.libraries_directories);
        }
      }

      return target;
    }

    auto extract_config(const std::string& name, const cmake::CodeModel::Configuration& codemodel_config) -> Configuration
    {
      Configuration config;
      config.name = name;
      for (auto [target_name, target_json] : codemodel_config.targets)
      {
        config.targets[target_name] = extract_target(target_json);
      }
      return config;
    }

    auto extract_dependencies(const cmake::CodeModel& codemodel) -> DependenciesInfo
    {
      DependenciesInfo dependencies;
      for (auto [config_name, config] : codemodel.configs)
      {
        dependencies.configurations[config_name] = extract_config(config_name, config);
      }
      return dependencies;
    }



    auto differences(const Compilation& left, const Compilation& right) -> Compilation
    {
      Compilation different;
      different.compilation_flags = difference(left.compilation_flags, right.compilation_flags);
      different.defines = difference(left.defines, right.defines);
      different.include_directories = difference(left.include_directories, right.include_directories);
      different.source_files = difference(left.source_files, right.source_files);
      return different;
    }

    auto differences(const Target& left, const Target& right) -> Target
    {
      Target different;
      different.libraries_directories = difference(left.libraries_directories, right.libraries_directories);
      different.link_flags = difference(left.link_flags, right.link_flags);
      different.link_libraries = difference(left.link_libraries, right.link_libraries);

      for(const auto& [ language_name, left_compilation ] : left.language_compilation)
      {
        const auto& right_compilation = right.language_compilation.find(language_name)->second;
        different.language_compilation[language_name] = differences(left_compilation, right_compilation);
      }

      return different;
    }

    auto differences(const Configuration& left, const Configuration& right) -> Configuration
    {
      Configuration different;
      for(const auto& [target_name, left_target] : left.targets)
      {
        const auto& right_target = right.targets.find(target_name)->second;
        static const std::regex to_remove(wyvern::target_prefix);
        const auto dependency_name = std::regex_replace(target_name, to_remove, ""); // Deduce the dependency name by removing the of our test target
        different.targets[dependency_name] = differences(left_target, right_target);
      }
      return different;
    }

    auto compare_dependencies(const cmake::CodeModel& control_codemodel, const cmake::CodeModel& dependent_codemodel)
        -> DependenciesInfo
    {
      log_codemodel("control", control_codemodel);
      log_codemodel("project", dependent_codemodel);

      DependenciesInfo diff;
      DependenciesInfo control = extract_dependencies(control_codemodel);
      DependenciesInfo dependent = extract_dependencies(dependent_codemodel);

      for(const auto& [config_name, config] : dependent.configurations)
      {
        diff.configurations[config_name] = differences(config, control.configurations[config_name]);
      }

      return diff;
    }
  } // namespace

  auto extract_dependencies(const cmake::Configuration& config, Options options)
    -> DependenciesInfo
  {
    is_logging_enabled = options.enable_logging;

    log() << "Begin cmake dependencies extraction" << " now";


    // 1. Create a temporary cmake project with CMakeFiles.txt and an c++ source file.
    //    It should have no dependencies at all, just as many executable targets as
    //    the number of targets in the provided configuration.
    // 2. Invoke CMake for that project, creating a build directory (without the options
    //    specified in the provided configuration?).
    // 3. Invoke CMake file-api in the resulting build directory to extract and store
    //    JSON information -> A.
    log() << "==== Extracting Control Information ====";
    const auto control_codemodel = extract_codemodel(config, cmake::cmakefile_mode::without_dependencies, options);

    // 4. Modify the CMakeLists.txt to add:
    //    - `find_package()` calls for each packages of the configuration provided;
    //    - each executable target should depend on one associated CMake target from the configuration.
    // 5. Invoke CMake again to create a different build directory, using options/variables
    //    from the configuration.
    // 6. Invoke CMake file-api on that new configuration and extract and store the
    //    JSON information -> B.
    log() << "==== Extracting Dependencies Information ====";
    const auto dependent_codemodel = extract_codemodel(config, cmake::cmakefile_mode::with_dependencies, options);

    // 7. Compare A and B, find what's in B that was not in B.
    // Return the result of that comparison.
    log() << "==== Comparing Control & Dependencies Information ====";
    const auto dependencies = compare_dependencies(control_codemodel, dependent_codemodel);

    log() << "End cmake dependencies extraction" << " here";

    return dependencies;
  }
} // namespace wyvern
