#include <libwyvern/wyvern.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <random>
#include <regex>

#include <nlohmann/json.hpp>
#include <libbutl/process.mxx>
#include <libbutl/path.mxx>
#include <libbutl/filesystem.mxx>
#include <libbutl/fdstream.mxx>
#include <fmt/format.h>

using json = nlohmann::json;
using path = butl::path; // File path
using dir_path = butl::dir_path; // Directory path

namespace wyvern {
namespace {

  struct failure : std::runtime_error
  {
    using std::runtime_error::runtime_error;
  };

  struct Logger
  {
    std::stringstream logged;

    Logger() { logged << "wyvern: "; }
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    template<class Arg>
    friend auto operator<<(Logger&& logger, Arg&& to_log)
      -> Logger&&
    {
      logger.logged << std::forward<Arg>(to_log);
      return std::move(logger);
    }

    ~Logger()
    {
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

    static const auto regex_string = R"regex([\s | - | \. | \:]+)regex";
    static const auto replacement = R"r(_)r";
    static const std::regex to_replace(regex_string);

    const auto normalized_name = std::regex_replace(lowercase_name, to_replace, replacement);
    log() << fmt::format("normalized \"{}\" to \"{}\"", name, normalized_name);
    return normalized_name;
  }

  auto write_to_file(path file_path, const std::string& content) -> void
  {
    log() << fmt::format("writing into file {}", file_path.complete().string());
    using namespace butl;
    static const auto open_mode = fdopen_mode::truncate | fdopen_mode::create;
    ofdstream file { file_path, open_mode };
    file << content; // Assuming we are in text mode.
    file.close(); // Throws exceptions if there have been errors while writing.
  }

  auto create_directories(dir_path directory_path) -> void
  {
    log() << fmt::format("creating directories {}", directory_path.complete().string());
    const auto result = butl::try_mkdir_p(directory_path);
    if(result != butl::mkdir_status::success){
      throw failure(fmt::format("Failed to create directory {}", directory_path.complete().string()));
    }
  }

}}

namespace wyvern::cmake {

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
    code << fmt::format("cmake_minimum_required(VERSION {})\n\n", minimum_cmake_version);
    code << fmt::format("project({})\n\n", random_int(0, 99999999));

    for(const auto& target : cmake_config.targets)
    {
      const auto suffix = normalize_name(target);
      code << fmt::format("add_executable(wyvern_{} main.cpp header.hpp)\n", suffix);
      if(mode == cmakefile_mode::with_dependencies)
      {
        code << fmt::format("target_link_libraries(wyvern_{} PRIVATE {})\n", suffix, target);
      }
    }
    code << "\n\n";
    return code.str();
  }

  auto create_cmake_project(dir_path directory_path, const Configuration& cmake_config, cmakefile_mode mode)
    -> void
  {
    // 1. create main.cpp and header.hpp
    static constexpr auto main_content = R"cpp(
#include "header.hpp"
int main() { }
    )cpp";

    static constexpr auto header_content = R"cpp(
#pragma once
// This is a header to check the output with a header file (which should not be compiled).
    )cpp";

    const auto main_cpp_path = directory_path / path("main.cpp");
    const auto header_hpp_path = directory_path / path("header.hpp");

    write_to_file(main_cpp_path, main_content);
    write_to_file(header_hpp_path, header_content);

    // 2. create the cmakefile with the right content
    const auto cmakefile_path = directory_path / path("CMakeFiles.txt");
    const auto cmakefile_content = generate_cmakefile_code(cmake_config, mode);
    write_to_file(cmakefile_path, cmakefile_content);
  }

  auto invoke_cmake(std::vector<std::string> args) -> void
  {
    // const auto cmake_command = fmt::format("cmake {}", directory_path);
    // run the command cmake
    // throw if any error is found
    log() << fmt::format("cmake {}", fmt::join(args, " "));
  }

  struct CodeModel
  {
    json index;
    std::map<std::string, json> targets;
  };

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
    invoke_cmake({ build_directory_path.complete().string() });

    // 3. retrieve the JSON information
    return {};
  }

  auto configure_project(dir_path project_path, dir_path build_path, const Configuration& cmake_config)
    -> void
  {
    const auto source_arg = fmt::format("-S {}", project_path.complete().string());
    const auto build_dir_arg = fmt::format("-B {}", build_path.complete().string());
    auto args = std::vector{ source_arg, build_dir_arg };
    // TODO: add args from config
    invoke_cmake(args);
  }


}


namespace wyvern
{
  class scoped_temp_dir
  {
    dir_path path_ = dir_path::temp_path("wyvern").complete();
  public:
    scoped_temp_dir(const scoped_temp_dir&) = delete;
    scoped_temp_dir& operator=(const scoped_temp_dir&) = delete;

    scoped_temp_dir()
    {
      create_directories(path_);
    }

    ~scoped_temp_dir()
    {
      butl::rmdir_r(path_);
    }

    const dir_path& path() const { return this->path_; }

  };


  auto extract_codemodel(const cmake::Configuration& config, cmake::cmakefile_mode mode)
    -> cmake::CodeModel
  {
    // step 1
    const scoped_temp_dir project_dir;
    cmake::create_cmake_project(project_dir.path(), config, mode);

    // step 2
    const auto build_dir_name = fmt::format("build-{}", normalize_name(config.generator));
    const auto build_dir_path = project_dir.path() / dir_path(build_dir_name);
    cmake::configure_project(project_dir.path(), build_dir_path, config);

    // step 3
    const auto codemodel = cmake::query_cmake_file_api(build_dir_path);

    return codemodel;
  }

  auto compare_dependencies(const cmake::CodeModel& control_codemodel, const cmake::CodeModel& project_codemodel)
    -> DependenciesInfo
  {
    return {};
  }

  auto extract_dependencies(const cmake::Configuration& config)
    -> DependenciesInfo
  {
    log() << "Begin cmake dependencies extraction" << " now";


    // 1. Create a temporary cmake project with CMakeFiles.txt and an c++ source file.
    //    It should have no dependencies at all, just as many executable targets as
    //    the number of targets in the provided configuration.
    // 2. Invoke CMake for that project, creating a build directory (without the options
    //    specified in the provided configuration?).
    // 3. Invoke CMake file-api in the resulting build directory to extract and store
    //    JSON information -> A.
    const auto control_codemodel = extract_codemodel(config, cmake::cmakefile_mode::without_dependencies);

    // 4. Modify the CMakeLists.txt to add:
    //    - `find_package()` calls for each packages of the configuration provided;
    //    - each executable target should depend on one associated CMake target from the configuration.
    // 5. Invoke CMake again to create a different build directory, using options/variables
    //    from the configuration.
    // 6. Invoke CMake file-api on that new configuration and extract and store the
    //    JSON information -> B.
    const auto dependencies_codemodel = extract_codemodel(config, cmake::cmakefile_mode::with_dependencies);

    // 7. Compare A and B, find what's in B that was not in B.
    // Return the result of that comparison.
    const auto dependencies = compare_dependencies(control_codemodel, dependencies_codemodel);

    log() << "End cmake dependencies extraction" << " here";

    return {};
  }
}
