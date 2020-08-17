#include <cassert>
#include <sstream>
#include <stdexcept>

#include <libwyvern/version.hpp>
#include <libwyvern/wyvern.hpp>

int main ()
{
  using namespace std;
  using namespace wyvern;

  // Basics.
  //
  {
    ostringstream o;
    say_hello (o, "World");
    assert (o.str () == "Hello, World!\n");
  }

  // Empty name.
  //
  try
  {
    ostringstream o;
    say_hello (o, "");
    assert (false);
  }
  catch (const invalid_argument& e)
  {
    assert (e.what () == string ("empty name"));
  }
}
