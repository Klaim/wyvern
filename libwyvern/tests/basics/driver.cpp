#include <cassert>
#include <iostream>

#include <nocontracts/assert.hpp>

#include <libwyvern/version.hpp>
#include <libwyvern/wyvern.hpp>

using namespace wyvern;

namespace {

  const auto test_project_package_name = "test_cmake_project";
  const auto test_project_targets = std::vector<std::string>{ "xxx", "yyy" };
  const auto test_project_sources_dir = dir_path{ "./libwyvern/tests/test_cmake_project/" }.complete();
  const auto test_build_dir_name = "build-wyvern-test_cmake_project";

  const auto test_config = []{
      cmake::Configuration config;
      config.generator = "Visual Studio 16 2019 Win64";
      config.packages = { { test_project_package_name } };
      config.targets = test_project_targets;
      config.args = { "--debug", };
      return config;
  }();

  scoped_temp_dir build_test_cmake_project()
  {
    scoped_temp_dir project_dir;

    const auto source_dir = test_project_sources_dir;
    const auto build_dir = project_dir.path() / test_build_dir_name;

    const auto args = std::vector<std::string>{
      "-S", source_dir.string(),
      "-B", build_dir.string(),
    };

    // NOTE: we'll use the default generator, no need to precise it.
    cmake::invoke_cmake(args); // Configure
    cmake::invoke_cmake({ "--build", build_dir.string() }); // Build? Is that necessary?

    return project_dir;
  }

}

int main ()
{

  try
  {
    const auto test_cmake_project_dir = build_test_cmake_project();
    const auto test_build_dir = test_cmake_project_dir.path() / test_build_dir_name;

    auto config = test_config;
    config.options = {
      { "test_cmake_project_DIR", test_build_dir.string() }
    };

    const auto deps_info = extract_dependencies(config);
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
