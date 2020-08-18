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

  auto query_cmake_file_api(std::string build_directory_path)
    -> json
  {
    // 1. write the query file in the build directory
    // 2. invoke cemake in that directory
    invoke_cmake({ build_directory_path });
    return {};
  }

  auto configure_project(std::string project_path, std::string build_path, const Configuration& cmake_config)
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

  DependenciesInfo extract_dependencies(const cmake::Configuration& config)
  {
    log() << "Begin cmake dependencies extraction";
    // 1. Create a temporary cmake project with CMakeFiles.txt and an c++ source file.
    //    It should have no dependencies at all, just as many executable targets as
    //    the number of targets in the provided configuration.
    // 2. Invoke CMake for that project, creating a build directory (without the options
    //    specified in the provided configuration?).
    // 3. Invoke CMake file-api in the resulting build directory to extract and store
    //    JSON information -> A.
    // 4. Modify the CMakeLists.txt to add:
    //    - `find_package()` calls for each packages of the configuration provided;
    //    - each executable target should depend on one associated CMake target from the configuration.
    // 5. Invoke CMake again to create a different build directory, using options/variables
    //    from the configuration.
    // 6. Invoke CMake file-api on that new configuration and extract and store the
    //    JSON information -> B.
    // 7. Compare A and B, find what's in B that was not in B.
    // Return the result of that comparison.

    log() << "End cmake dependencies extraction";

    return {};
  }
}
