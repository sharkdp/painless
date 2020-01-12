cmake_minimum_required(VERSION 3.1)

project(painless)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE "include")

add_subdirectory(tests)
