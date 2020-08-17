#include <libwyvern/wyvern.hpp>
#include <nlohmann/json.hpp>
#include <libbutl/process.mxx>

namespace wyvern
{
  DependenciesInfo extract_dependencies(const cmake::Configuration& config)
  {
    // 1. Create a temporary cmake project with CMakeFiles.txt and an c++ source file.
    //    It should have no dependencies at all, just as many executable targets as
    //    the number of targets in the provided configuration.
    // 2. Invoke CMake for that project, creating a build directory (without the options
    //    specified in the provided configuration?).
    // 3. Invoke CMake file-api in the resulting build directory to extract and store
    //    JSON information -> A.
    // 4. Modify the CMakeLists.txt to add:
    //    - `find_package()` calls for each packages of the configuration provided;
    //    - each executable target should depend on one associated CMake target from the configuration.
    // 5. Invoke CMake again to create a different build directory, using options/variables
    //    from the configuration.
    // 6. Invoke CMake file-api on that new configuration and extract and store the
    //    JSON information -> B.
    // 7. Compare A and B, find what's in B that was not in B.
    // Return the result of that comparison.

    return {};
  }
}
