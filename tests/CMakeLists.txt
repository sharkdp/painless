add_subdirectory(Catch2)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Catch2/contrib)

include(CTest)
include(Catch)

set(WARNING_FLAGS "-Wall" "-pedantic")

add_executable(${PROJECT_NAME}-tests main.cpp parameter.cpp)
target_compile_options(${PROJECT_NAME}-tests PRIVATE ${WARNING_FLAGS})
target_link_libraries(${PROJECT_NAME}-tests ${PROJECT_NAME} Catch2::Catch2 pthread)

catch_discover_tests(${PROJECT_NAME}-tests)

add_executable(${PROJECT_NAME}-interactive interactive.cpp)
target_compile_options(${PROJECT_NAME}-interactive PRIVATE ${WARNING_FLAGS})
target_link_libraries(${PROJECT_NAME}-interactive ${PROJECT_NAME} pthread)
