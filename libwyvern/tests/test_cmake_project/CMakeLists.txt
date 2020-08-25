cmake_minimum_required(VERSION 3.10)

project(test_cmake_project)

add_definitions( -DTEST_CMAKE_PROJECTSHARED_BUILD )

set(include_dir "${PROJECT_SOURCE_DIR}/include/")
message(STATUS "INCLUDE DIR = ${include_dir}")

add_library(xxx SHARED ${include_dir}/test_project_xxx.hpp xxx.cpp)
target_include_directories(xxx PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

add_library(yyy SHARED ${include_dir}/test_project_yyy.hpp yyy.cpp)
target_include_directories(yyy PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_compile_definitions(yyy
  INTERFACE
    -DYYY_THIS_DEFINITION_IS_INTERFACE
  PUBLIC
    -DYYY_THIS_DEFINITION_IS_PUBLIC
  PRIVATE
    -DYYY_THIS_DEFINITION_IS_PRIVATE
)

add_library(zzz STATIC ${include_dir}/test_project_zzz.hpp zzz.cpp)
target_include_directories(zzz PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_compile_definitions(zzz
  INTERFACE
    -DZZZ_THIS_DEFINITION_IS_INTERFACE
  PUBLIC
    -DZZZ_THIS_DEFINITION_IS_PUBLIC
  PRIVATE
    -DZZZ_THIS_DEFINITION_IS_PRIVATE
)


add_library(aaa SHARED ${include_dir}/test_project_aaa.hpp aaa.cpp)
target_link_libraries(aaa yyy)
target_include_directories(aaa PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)


install(TARGETS xxx yyy zzz aaa
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