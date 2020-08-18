#pragma once
// This is just for test.

#if not defined(TEST_CMAKE_PROJECTSHARED_BUILD)
#define TEST_CMAKE_PROJECTSHARED
#endif

#if defined(TEST_CMAKE_PROJECTSTATIC)         // Using static.
#  define TEST_CMAKE_PROJECTSYMEXPORT
#elif defined(TEST_CMAKE_PROJECTSTATIC_BUILD) // Building static.
#  define TEST_CMAKE_PROJECTSYMEXPORT
#elif defined(TEST_CMAKE_PROJECTSHARED)       // Using shared.
#  ifdef _WIN32
#    define TEST_CMAKE_PROJECTSYMEXPORT __declspec(dllimport)
#  else
#    define TEST_CMAKE_PROJECTSYMEXPORT
#  endif
#elif defined(TEST_CMAKE_PROJECTSHARED_BUILD) // Building shared.
#  ifdef _WIN32
#    define TEST_CMAKE_PROJECTSYMEXPORT __declspec(dllexport)
#  else
#    define TEST_CMAKE_PROJECTSYMEXPORT
#  endif
#else
#  define TEST_CMAKE_PROJECTSYMEXPORT         // Using static or shared.
//#  error define TEST_CMAKE_PROJECTSTATIC or TEST_CMAKE_PROJECTSHARED preprocessor macro to signal libwyvern library type being linked
#endif


namespace test_cmake_project {
    TEST_CMAKE_PROJECTSYMEXPORT
    void function_test_project_yyy();
}


