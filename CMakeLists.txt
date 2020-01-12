cmake_minimum_required(VERSION 3.1)

project(painless)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE "include")
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_11)

add_subdirectory(tests)
