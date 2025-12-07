cmake_minimum_required(VERSION 3.16)
project(aoc2025 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(cmake/CompileOptions.cmake)

# Header-only common helpers
add_library(aoc_common INTERFACE)

target_include_directories(aoc_common
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(aoc_common
    INTERFACE aoc_compile_options
)

function(add_day day)
    string(LENGTH "${day}" len)
    if (NOT len EQUAL 2)
        message(FATAL_ERROR "Day should be two digits, e.g. 01, 02, ...")
    endif()

    set(src_file "${CMAKE_CURRENT_SOURCE_DIR}/src/days/day${day}.cpp")
    if (NOT EXISTS "${src_file}")
        message(WARNING "Source file ${src_file} does not exist yet")
    endif()

    add_executable(day${day} ${src_file})

    target_link_libraries(day${day}
        PRIVATE aoc_common      # brings in include/ and flags
    )

    target_compile_options(day${day} PRIVATE -O3)

    # For IDEs (Xcode/MSVC); harmless elsewhere
    set_target_properties(day${day} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endfunction()

add_day(07)