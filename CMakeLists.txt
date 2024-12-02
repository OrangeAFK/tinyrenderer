cmake_minimum_required(VERSION 3.5.0)
project(tinyrenderer)

# Collect all the source files recursively in the src/ folder
file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_executable(tinyrenderer ${SOURCES})

