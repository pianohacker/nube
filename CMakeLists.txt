cmake_minimum_required(VERSION 2.8)
project(nube)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PKG clutter-1.0 clutter-x11-1.0 gsdl-1.0)
set(CMAKE_C_FLAGS "-std=gnu99 -ggdb3 -Wall")

add_executable(nube
	src/config.c
	src/drawing.c
	src/linux-info.c
	src/panel.c
	src/sources.c
	src/util.c
	src/widgets.c
	src/main.c
)
include_directories(${PKG_INCLUDE_DIRS})
target_link_libraries(nube m ${PKG_LDFLAGS})
