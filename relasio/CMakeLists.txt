add_library(relasio readline.cpp)

target_include_directories(relasio PUBLIC .. ${Boost_INCLUDE_DIRS})
target_link_libraries(relasio PUBLIC
  ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} readline)

file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

install(TARGETS relasio ARCHIVE DESTINATION ${LIB_INSTALL_DIR})
install(FILES ${HEADERS}
  DESTINATION "${INCLUDE_INSTALL_DIR}/relasio" )
