cmake_minimum_required(VERSION 3.30)
project(tclcurl C)

set(CMAKE_C_STANDARD 11)

add_definitions(-DPACKAGE_NAME="tclcurl" -DPACKAGE_VERSION="0.1" -DUSE_TCL_STUBS)

find_package(CURL REQUIRED)


add_library(tclcurl SHARED tclcurl.c)

target_link_libraries (tclcurl /opt/tcl9/lib/libtclstub.a ${CURL_LIBRARIES})
include_directories (/opt/tcl9/include)