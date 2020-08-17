#pragma once

#include <utility>
#include <vector>
#include <string>

#include <libwyvern/export.hpp>

namespace wyvern::cmake {

  using Option = std::pair<std::string, std::string>; // Options/variables for CMake

  struct Package
  {
    std::string name; // Name of the package.
    std::vector<std::string> constraints; // Constraints like "VERSION 2.5" etc. see `find_package()` arguments in CMake documentation.
  };

  struct Configuration
  {
    std::vector<Package> packages; // CMake packages to extract informations from.
    std::vector<std::string> targets; // Qualified names of CMake targets to extract information from.
    std::vector<Option> options; // CMake options and variables to pass to CMake on invokation.
  };

}


namespace wyvern
{
  struct TargetInfo
  {
    std::string name;
    std::vector<std::string> include_directories;
    std::vector<std::string> lib_directories;
    std::vector<std::string> compilation_flags;
    std::vector<std::string> files;
  };

  using DependenciesInfo = std::vector<TargetInfo>;

  LIBWYVERN_SYMEXPORT
  DependenciesInfo extract_dependencies(const cmake::Configuration& config);

}
