cmake_minimum_required(VERSION 3.10)

project(test_cmake_project)

set(include_dir "${PROJECT_SOURCE_DIR}/include/")
set(header "${include_dir}/test_project.hpp")

add_library(xxx ${header} source.cpp)
target_include_directories(xxx PUBLIC ${include_dir} )

add_library(yyy ${header} source.cpp)
target_include_directories(yyy PUBLIC ${include_dir} )

export(
  TARGETS xxx yyy
  NAMESPACE test_cmake::
  FILE test_cmake_project-config.cmake
)

install( TARGETS xxx yyy DESTINATION lib )
install(FILES "${PROJECT_SOURCE_DIR}/include/test_project.hpp"
  DESTINATION include
)



