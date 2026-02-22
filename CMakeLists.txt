cmake_minimum_required(VERSION 3.14)
project(JsonFormat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC include/)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall -Wextra>
)
if (JSON_FORMAT_STRICT_ERRORS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DJSON_FORMAT_STRICT_ERRORS)
endif()

if (JSON_FORMAT_BUILD_TESTS)
    enable_testing()

    add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/googletest ${CMAKE_BINARY_DIR}/_deps/googletest-build SYSTEM)

    add_executable(${PROJECT_NAME}Tests tests/main.cpp)
    set_target_properties(${PROJECT_NAME}Tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    target_link_libraries(${PROJECT_NAME}Tests PRIVATE ${PROJECT_NAME} GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}Tests)
endif()
