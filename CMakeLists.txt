cmake_minimum_required(VERSION 3.10)

find_package(GTest REQUIRED)
find_package(Boost REQUIRED)

add_library(ulplib INTERFACE)
target_include_directories(ulplib INTERFACE include)
target_include_directories(ulplib INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(ulplib INTERFACE cxx_std_17)

add_subdirectory(test)
