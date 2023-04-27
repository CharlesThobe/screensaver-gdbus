cmake_minimum_required(VERSION 3.25)
project("Rails")

add_executable(test
screensaver-gdbus.cpp
)

install(TARGETS test DESTINATION bin)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
target_link_libraries(target PkgConfig::deps)