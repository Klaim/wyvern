#include <cassert>
#include <iostream>
#include <algorithm>

#include <nocontracts/assert.hpp>

#include <libwyvern/version.hpp>
#include <libwyvern/wyvern.hpp>

using namespace wyvern;

namespace {

  const bool keep_generated_directories = false;
  const bool enable_loggging = true;

  const auto test_project_package_name = "test_cmake_project";
  const auto test_project_targets = std::vector<std::string>{ "test_project::aaa", "test_project::xxx", "test_project::yyy", "test_project::zzz" };
  const auto test_project_sources_dir = dir_path{ "libwyvern/tests/test_cmake_project/" };
  const auto test_project_build_dir_name = dir_path{"build-wyvern-test_cmake_project"};

  const auto test_project_user_package_name = "test_cmake_project_user";
  const auto test_project_user_targets = std::vector<std::string>{ "test_project_user::user_aaa" };
  const auto test_project_user_sources_dir = dir_path{ "libwyvern/tests/test_cmake_project_user/" };
  const auto test_project_user_build_dir_name = dir_path{"build-wyvern-test_cmake_project_user"};

  // All thest projects install in the same directory
  const auto test_install_dir_name = dir_path{"install-wyvern-test_cmake_project"};

  const auto test_config = []{
      cmake::Configuration config;
      // config.generator = "Visual Studio 16 2019"; // Use default generator for testing.
      config.packages = { { test_project_package_name }, { test_project_user_package_name } };
      config.targets = test_project_targets;
      config.targets.insert(config.targets.end(), test_project_user_targets.begin(), test_project_user_targets.end());
      config.args = { "--config release", };
      return config;
  }();

  scoped_temp_dir build_install(const dir_path& source_dir, dir_path build_dir_name, dir_path install_dir_name,
    std::vector<wyvern::cmake::Option> cmake_options = {},
    scoped_temp_dir project_dir = scoped_temp_dir{keep_generated_directories})
  {

    const auto build_dir = (project_dir.path() / build_dir_name).normalize(true, true);
    const auto install_dir = (project_dir.path() / install_dir_name).normalize(true, true);

    auto args = std::vector<std::string>{ "-DCMAKE_INSTALL_PREFIX=" + install_dir.string() };
    for(const auto& option : cmake_options)
    {
      const auto option_arg = std::string("-D") + option.first + "=" + option.second;
      args.push_back(option_arg);
    }
    args.insert(args.end(), {
//      "-G", test_config.generator, // Use the default generator for testing.
      "-DCMAKE_BUILD_TYPE=Release",
      "-S", source_dir.string(),
      "-B", build_dir.string(),
    });

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
    wyvern::enable_logging(enable_loggging);


    auto test_cmake_project_dir = build_install(test_project_sources_dir, test_project_build_dir_name, test_install_dir_name);
    const auto test_install_dir = (test_cmake_project_dir.path() / test_install_dir_name).normalize(true, true);

    auto config = test_config;
    config.options = { { "CMAKE_PREFIX_PATH", test_install_dir.string() } }; // The user project depends on the initial test project.

    test_cmake_project_dir = build_install(test_project_user_sources_dir,
      test_project_user_build_dir_name, test_install_dir_name, // Same install dir as the test project.
      config.options, std::move(test_cmake_project_dir)); // We want all the build happening in the same temporary directory.

    const auto check_code = R"cpp(
#include <{target_name}.hpp>
void check() {{
  test_cmake_project::function_{target_name}();
}}
    )cpp";

    Options options;
    options.enable_logging = enable_loggging;
    options.code_format_to_inject_in_client = check_code;
    options.keep_generated_projects = keep_generated_directories;

    const auto deps_info = extract_dependencies(config, options);
    NC_ASSERT_TRUE( !deps_info.empty() );

    // TODO: add checks here
    std::cout << "############# DEDUCED DEPENDENCIES ##############" << std::endl;
    std::cout << deps_info << std::endl;

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
