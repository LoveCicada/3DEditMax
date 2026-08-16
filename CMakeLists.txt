cmake_minimum_required(VERSION 3.16)
project(3DEditMax LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

if(DEFINED ENV{QT_DIR})
  string(STRIP "$ENV{QT_DIR}" _qt_dir)
  string(REGEX REPLACE "[\\\\/]+$" "" _qt_dir "${_qt_dir}")
  get_filename_component(_qt_name "${_qt_dir}" NAME)
  if(_qt_name STREQUAL "bin")
    get_filename_component(_qt_dir "${_qt_dir}" DIRECTORY)
  endif()
  list(PREPEND CMAKE_PREFIX_PATH "${_qt_dir}")
  message(STATUS "Qt prefix from QT_DIR: ${_qt_dir}")
endif()

find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets)

add_executable(3DEditMax
  src/main.cpp
  src/app/MainWindow.h
  src/app/MainWindow.cpp
)

target_include_directories(3DEditMax PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(3DEditMax PRIVATE Qt5::Widgets)

enable_testing()
add_executable(3deditmax_tests
  tests/test_main.cpp
)
target_include_directories(3deditmax_tests PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/tests"
)
add_test(NAME 3deditmax_tests COMMAND 3deditmax_tests)
