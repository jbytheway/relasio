cmake_minimum_required (VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
find_package(Boost REQUIRED COMPONENTS system filesystem)

add_subdirectory(relasio)
add_subdirectory(test)
