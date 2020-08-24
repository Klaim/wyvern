#include <iostream>

#include <libwyvern/wyvern.hpp>


int main (int argc, char* argv[])
{
  if(argc < 3)
  {
    std::cerr << "FAIL!" << std::endl;
    return EXIT_FAILURE;
  }

  const auto cmake_install_dir = wyvern::dir_path(argv[1]).realize();

  wyvern::cmake::Configuration config;
  config.args = { "--config release" };
  config.options = {
    { "CMAKE_PREFIX_PATH", cmake_install_dir.string() }
  };

  config.packages = { { argv[2], /*"1.73.0", { "COMPONENTS filesystem" }*/ } };

  for(int target_name_idx = 3; target_name_idx < argc; ++target_name_idx)
  {
    config.targets.push_back(argv[target_name_idx]);
  }

  wyvern::Options options;
  options.enable_logging = true;
  options.keep_generated_projects = true;

  const auto deps_info = extract_dependencies(config, options);
  std::cout << "############# WYVERN: DEDUCED DEPENDENCIES ##############" << std::endl;
  std::cout << deps_info << std::endl;
}
