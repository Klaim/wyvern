#pragma once


#include <utility>
#include <vector>
#include <string>
#include <map>

#include <libbutl/path.mxx>

#include <libwyvern/export.hpp>

namespace wyvern::cmake {

  using Option = std::pair<std::string, std::string>; // Options/variables for CMake

  struct Package
  {
    std::string name; // Name of the package.
    std::string version; // Version of the package, or unspecified.
    std::vector<std::string> constraints; // Constraints like "VERSION 2.5" etc. see `find_package()` arguments in CMake documentation.
  };

  struct Configuration
  {
    std::string generator; // Name of the generator provided by CMake (that will create the proper build-system files).
    std::vector<Package> packages; // CMake packages to extract informations from.
    std::vector<std::string> targets; // Qualified names of CMake targets to extract information from.
    std::vector<Option> options; // CMake options and variables to pass to CMake on invokation.
    std::vector<std::string> args; // Additional arguments
  };

  LIBWYVERN_SYMEXPORT
  auto invoke_cmake(const std::vector<std::string>& args) -> void;

}


namespace wyvern
{
  struct Compilation
  {
    std::vector<std::string> include_directories;
    std::vector<std::string> compilation_flags;
    std::vector<std::string> defines;
    std::vector<std::string> source_files;
  };
  struct Target
  {
    std::string name;
    std::map<std::string, Compilation> language_compilation; //Compilation info per language
    std::vector<std::string> libraries_directories;
    std::vector<std::string> link_libraries;
    std::vector<std::string> link_flags;
  };

  struct Configuration
  {
    std::string name;
    std::map<std::string, Target> targets;
  };

  struct DependenciesInfo
  {
    std::map<std::string, Configuration> configurations; // There can be more than one configuration generated, for example with Visual Studio generators.
    bool empty() const { return configurations.empty(); }
  };

  LIBWYVERN_SYMEXPORT
  std::ostream& operator<<(std::ostream& out, const DependenciesInfo& deps);

  struct Options
  {
    bool keep_generated_projects = false;
    std::string code_format_to_inject_in_client;
    bool enable_logging = false;
  };

  LIBWYVERN_SYMEXPORT
  DependenciesInfo extract_dependencies(const cmake::Configuration& config, Options options = {});

  using path = butl::path; // File path
  using dir_path = butl::dir_path; // Directory path

  class LIBWYVERN_SYMEXPORT scoped_temp_dir
  {
    dir_path path_;
    bool keep_directory = false;

  public:

    // Move-only
    scoped_temp_dir(const scoped_temp_dir&) = delete;
    scoped_temp_dir& operator=(const scoped_temp_dir&) = delete;
    scoped_temp_dir(scoped_temp_dir&&);
    scoped_temp_dir& operator=(scoped_temp_dir&&);

    scoped_temp_dir(bool keep_directory = false);
    ~scoped_temp_dir();

    const dir_path& path() const { return this->path_; }
  };

  LIBWYVERN_SYMEXPORT
  bool enable_logging(bool is_enabled);

}
