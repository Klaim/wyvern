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
    const auto deps_info = extract_dependencies();
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
