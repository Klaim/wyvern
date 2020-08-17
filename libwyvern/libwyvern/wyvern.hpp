#pragma once

#include <iosfwd>
#include <string>

#include <libwyvern/export.hpp>

namespace wyvern
{
  // Print a greeting for the specified name into the specified
  // stream. Throw std::invalid_argument if the name is empty.
  //
  LIBWYVERN_SYMEXPORT void
  say_hello (std::ostream&, const std::string& name);
}
