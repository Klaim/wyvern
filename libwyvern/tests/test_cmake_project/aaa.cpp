
#include "test_project_aaa.hpp"
#include "test_project_yyy.hpp" // we use yyy

namespace test_cmake_project {
    void function_test_project_aaa()
    {
        function_test_project_yyy();
    }
}
