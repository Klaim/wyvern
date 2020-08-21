#include <cassert>
#include <iostream>

#include <nocontracts/assert.hpp>

#include <libwyvern/version.hpp>
#include <libwyvern/wyvern.hpp>

int main ()
{
  using namespace std;
  using namespace wyvern;

  try
  {
    cmake::Configuration config;
    config.generator = "Visual Studio 16 2019 Win64";
    config.packages = { { "FMT" } };
    config.targets = { "fmt::fmt" };

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
