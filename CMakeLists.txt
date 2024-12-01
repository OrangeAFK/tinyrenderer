cmake_minimum_required(VERSION 3.5.0)
project(tinyrenderer VERSION 0.1.0 LANGUAGES C CXX)

file(GLOB_RECURSE SOURCES *.h *.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})