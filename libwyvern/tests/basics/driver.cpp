#include <cassert>
#include <iostream>

#include <nocontracts/assert.hpp>

#include <libwyvern/version.hpp>
#include <libwyvern/wyvern.hpp>

static const auto test_config = []{
    wyvern::cmake::Configuration config;
    config.generator = "Visual Studio 16 2019 Win64";
    config.packages = { { "FMT" } }; // TODO: replace by test cmake project package name.
    config.targets = { "fmt::fmt" }; // TODO: replace by test cmake project targets names.
    config.args = { "--debug", };
    return config;
}();

wyvern::scoped_temp_dir build_test_cmake_project()
{
  wyvern::scoped_temp_dir project_dir;
  // TODO: invoke cmake with the sources of the test CMake project to build in a build directory here
  // Use the generator and args found in the test configuration.
  return project_dir;
}


int main ()
{
  using namespace std;
  using namespace wyvern;

  try
  {
    const auto test_cmake_project_dir = build_test_cmake_project();
    auto config = test_config;
    // TODO: add flags to find the build dir of the test cmake project
    const auto deps_info = extract_dependencies(test_config);
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
