cmake_minimum_required(VERSION 3.25)
project("screensaver-gdbus")

add_executable(screensaver-gdbus
screensaver-gdbus.cpp
)

install(TARGETS screensaver-gdbus DESTINATION bin)

find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0 gio-2.0)
target_link_libraries(screensaver-gdbus PkgConfig::deps)

