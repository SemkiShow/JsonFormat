cmake_minimum_required(VERSION 3.14)
project(JsonFormat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# JsonFormat
add_subdirectory(${CMAKE_SOURCE_DIR}/.. ${CMAKE_SOURCE_DIR}/../build)

function(new_example name)
    add_executable(${name} src/${name}.cpp)
    target_compile_options(${name} PRIVATE -Wall -Wextra)
    set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    target_link_libraries(${name} PRIVATE JsonFormat)

    # Checks if OSX and links appropriate frameworks (Only required on MacOS)
    if (APPLE)
        target_link_libraries(${name} PRIVATE "-framework IOKit")
        target_link_libraries(${name} PRIVATE "-framework Cocoa")
        target_link_libraries(${name} PRIVATE "-framework OpenGL")
    endif()
endfunction()

new_example(Simple)
