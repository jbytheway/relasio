cmake_minimum_required (VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(LIB_INSTALL_DIR lib CACHE PATH "Installation directory for libraries")
set(INCLUDE_INSTALL_DIR install CACHE PATH "Installation directory for headers")
find_package(Boost REQUIRED COMPONENTS system filesystem)

add_subdirectory(relasio)
add_subdirectory(test)
