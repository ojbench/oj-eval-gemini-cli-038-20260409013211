cmake_minimum_required(VERSION 3.10)
project(vector)

set(CMAKE_CXX_STANDARD 20)

file(GLOB SOURCES "*.cpp")

if(SOURCES)
    add_executable(code ${SOURCES})
else()
    file(WRITE dummy.cpp "int main() { return 0; }")
    add_executable(code dummy.cpp)
endif()
