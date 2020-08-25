cmake_minimum_required(VERSION 3.10)

project(test_cmake_project_user)

message(STATUS "CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")

find_package(test_cmake_project REQUIRED)

add_definitions( -DTEST_CMAKE_PROJECTSHARED_BUILD )

set(include_dir "${PROJECT_SOURCE_DIR}/include/")
message(STATUS "INCLUDE DIR = ${include_dir}")

add_library(user_aaa SHARED ${include_dir}/test_project_user_user_aaa.hpp user_aaa.cpp)
target_link_libraries(user_aaa PRIVATE test_project::aaa)
target_include_directories(user_aaa PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)


 install(TARGETS user_aaa
   EXPORT test_cmake_project_user
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
 )

 install(DIRECTORY include
   DESTINATION .
 )

 install(EXPORT test_cmake_project_user
   DESTINATION lib/cmake/test_cmake_project_user
   FILE "test_cmake_project_user-config.cmake"
   NAMESPACE test_project_user::
 )