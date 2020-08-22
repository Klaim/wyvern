#include <cassert>
#include <iostream>

#include <nocontracts/assert.hpp>

#include <libwyvern/version.hpp>
#include <libwyvern/wyvern.hpp>

using namespace wyvern;

namespace {

  const auto test_project_package_name = "test_cmake_project";
  const auto test_project_targets = std::vector<std::string>{ "test_project::xxx", "test_project::yyy" };
  const auto test_project_sources_dir = dir_path{ "libwyvern/tests/test_cmake_project/" };
  const auto test_build_dir_name = "build-wyvern-test_cmake_project";
  const auto test_install_dir_name = "install-wyvern-test_cmake_project";

  const auto test_config = []{
      cmake::Configuration config;
      // config.generator = "Visual Studio 16 2019"; // Use default generator
      config.packages = { { test_project_package_name } };
      config.targets = test_project_targets;
      config.args = { "--config release", };
      return config;
  }();

  scoped_temp_dir build_test_cmake_project()
  {
    scoped_temp_dir project_dir;

    const auto source_dir = test_project_sources_dir;
    const auto build_dir = (project_dir.path() / dir_path(test_build_dir_name)).normalize(true, true);
    const auto install_dir = (project_dir.path() / dir_path(test_install_dir_name)).normalize(true, true);

    const auto args = std::vector<std::string>{
      "-DCMAKE_INSTALL_PREFIX=" + install_dir.string(),
      // "-G", test_config.generator,
      "-S", source_dir.string(),
      "-B", build_dir.string(),
    };

    // NOTE: we'll use the default generator, no need to precise it.
    //       also we'll only install the release build, although the debug build would be available in some real projects
    cmake::invoke_cmake(args); // Configure
    cmake::invoke_cmake({ "--build", build_dir.string(), "--config", "Release" }); // Build
    cmake::invoke_cmake({ "--install", build_dir.string(), "--config", "Release" }); // Install

    return project_dir;
  }

}

int main ()
{

  try
  {
    const auto test_cmake_project_dir = build_test_cmake_project();
    const auto test_install_dir = (test_cmake_project_dir.path() / dir_path(test_install_dir_name)).normalize(true, true);

    auto config = test_config;
    config.options = {
      { "CMAKE_PREFIX_PATH", test_install_dir.string() }
    };

    const auto check_code = R"cpp(
#include <{target_name}.hpp>
void check() {{
  test_cmake_project::function_{target_name}();
}}
    )cpp";

    const auto deps_info = extract_dependencies(config, check_code);
    NC_ASSERT_TRUE( deps_info.empty() );
    return EXIT_SUCCESS;
  }
  catch(const std::exception& err){
    std::cerr << "ERROR: " << err.what() << std::endl;
  }
  catch(...){
    std::cerr << "ERROR: unknown exception" << std::endl;
  }
  return EXIT_FAILURE;
}
