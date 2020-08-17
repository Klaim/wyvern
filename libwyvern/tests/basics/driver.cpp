#include <cassert>
#include <nocontracts/assert.hpp>

#include <libwyvern/version.hpp>
#include <libwyvern/wyvern.hpp>

int main ()
{
  using namespace std;
  using namespace wyvern;

  const auto deps_info = extract_dependencies({});
  NC_ASSERT_TRUE( deps_info.empty() );

}
