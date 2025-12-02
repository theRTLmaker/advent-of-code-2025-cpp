add_library(aoc_compile_options INTERFACE)

if (MSVC)
    target_compile_options(aoc_compile_options INTERFACE
        /W4 /permissive-
    )
else()
    target_compile_options(aoc_compile_options INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
    )
endif()