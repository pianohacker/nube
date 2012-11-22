cmake_minimum_required(VERSION 2.8)
project(nube)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GDK gdk-3.0)
pkg_check_modules(CLUTTER clutter-1.0 clutter-x11-1.0)
set(CMAKE_C_FLAGS "-std=gnu99 -ggdb3 -Wall")

add_executable(nube
	src/drawing.c
	src/panel.c
	src/util.c
	src/widgets.c
	src/main.c
)
include_directories(${GDK_INCLUDE_DIRS} ${CLUTTER_INCLUDE_DIRS})
target_link_libraries(nube m ${GDK_LDFLAGS} ${CLUTTER_LDFLAGS})
