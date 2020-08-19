#include <iostream>
#include <libwyvern/wyvern.hpp>
#include <nlohmann/json.hpp>
#include <libbutl/process.mxx>
#include <libbutl/path.mxx>
#include <fmt/core.h>

using json = nlohmann::json;

namespace wyvern {

  struct Logger
  {
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    template<class Arg>
    friend auto operator<<(Logger&& logger, Arg&& to_log)
      -> Logger&&
    {
      std::cout << fmt::format("wyvern: {}", std::forward<Arg>(to_log));
      return std::move(logger);
    }

    ~Logger()
    {
      std::cout << std::endl;
    }
  };

  auto log() -> Logger { return {}; }

}

namespace wyvern::cmake {

  enum class cmakefile_mode
  {
    without_dependencies,
    with_dependencies,
  };

  auto generate_cmakefile_code(const Configuration& cmake_config, cmakefile_mode mode)
    -> std::string // content of the CMakeFile.txt
  {
    return "empty!";
  }

  auto create_cmake_project(std::string directory_path, const Configuration& cmake_config, cmakefile_mode mode)
    -> void
  {
    // 1. create the cmakefile with the right content
    // 2. create main.cpp and header.hpp
  }

  constexpr auto cmake_file_api_query_json = R"JSON(
{
  "requests" : [
      {
          "kind": "codemodel",
          "version": { "major": 2 }
      }
  ]
}
)JSON";

  auto invoke_cmake(std::vector<std::string> args) -> void
  {
    // const auto cmake_command = fmt::format("cmake {}", directory_path);
    // run the command cmake
    // throw if any error is found
  }

  struct CodeModel
  {
    json index;
    std::map<std::string, json> targets;
  };

  auto query_cmake_file_api(std::string build_directory_path)
    -> CodeModel
  {
    // 1. write the query file in the build directory
    // 2. invoke cemake in that directory
    invoke_cmake({ build_directory_path });
    // 3. retrieve the JSON information
    return {};
  }

  auto configure_project(std::string project_path, std::string build_path, const Configuration& cmake_config)
    -> void
  {
    const auto source_arg = fmt::format("-S {}", project_path);
    const auto build_dir_arg = fmt::format("-B {}", build_path);

    auto args = std::vector{ source_arg, build_dir_arg };
    // add args from config
    invoke_cmake(args);
  }


}


namespace wyvern
{
  class scoped_temp_dir
  {
    std::string path_;
  public:
    scoped_temp_dir(const scoped_temp_dir&) = delete;
    scoped_temp_dir& operator=(const scoped_temp_dir&) = delete;

    scoped_temp_dir()
      : path_("~/tmp/gjisgnoefnv")
    {
      // create the temp dir here
    }

    ~scoped_temp_dir()
    {
      // delete the temp dir here
    }

    const std::string& path() const { return this->path_; }

  };


  auto normalize_name(std::string name) -> std::string
  {
    // ...
    return name;
  }

  auto extract_codemodel(const cmake::Configuration& config, cmake::cmakefile_mode mode)
    -> cmake::CodeModel
  {
    // 1. Create a temporary cmake project with CMakeFiles.txt and an c++ source file.
    //    It should have no dependencies at all, just as many executable targets as
    //    the number of targets in the provided configuration.
    const scoped_temp_dir project_dir;
    cmake::create_cmake_project(project_dir.path(), config, mode);

    // 2. Invoke CMake for that project, creating a build directory (without the options
    //    specified in the provided configuration?).
    const auto build_dir_path = fmt::format("{}/{}", project_dir.path(), normalize_name(config.generator));
    cmake::configure_project(project_dir.path(), build_dir_path, config);

    // 3. Invoke CMake file-api in the resulting build directory to extract and store
    //    JSON information -> A.
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
    log() << "Begin cmake dependencies extraction";

    const auto control_codemodel = extract_codemodel(config, cmake::cmakefile_mode::without_dependencies);

    // 4. Modify the CMakeLists.txt to add:
    //    - `find_package()` calls for each packages of the configuration provided;
    //    - each executable target should depend on one associated CMake target from the configuration.
    // 5. Invoke CMake again to create a different build directory, using options/variables
    //    from the configuration.
    // 6. Invoke CMake file-api on that new configuration and extract and store the
    //    JSON information -> B.
    const auto codemodel = extract_codemodel(config, cmake::cmakefile_mode::with_dependencies);

    // 7. Compare A and B, find what's in B that was not in B.
    // Return the result of that comparison.
    const auto dependencies = compare_dependencies(control_codemodel, codemodel);

    log() << "End cmake dependencies extraction";

    return {};
  }
}
