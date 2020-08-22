cmake_minimum_required(VERSION 3.10)

project(test_cmake_project)

add_definitions( -DTEST_CMAKE_PROJECTSHARED_BUILD )

set(include_dir "${PROJECT_SOURCE_DIR}/include/")
set(header "${include_dir}/test_project.hpp")
message(STATUS "INCLUDE DIR = ${include_dir}")

add_library(xxx SHARED ${header} xxx.cpp)
target_include_directories(xxx PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

add_library(yyy SHARED ${header} yyy.cpp)
target_include_directories(yyy PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

install(TARGETS xxx yyy
  EXPORT test_cmake_project
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(DIRECTORY include
  DESTINATION .
)

install(EXPORT test_cmake_project
  DESTINATION lib/cmake/test_cmake_project
  FILE "test_cmake_project-config.cmake"
  NAMESPACE test_project::
)