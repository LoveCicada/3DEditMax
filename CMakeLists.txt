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
  src/ui/Dx11ViewportWidget.h
  src/ui/Dx11ViewportWidget.cpp
  src/ui/DebugLogPanel.h
  src/ui/DebugLogPanel.cpp
  src/ui/TransformPanel.h
  src/ui/TransformPanel.cpp
  src/ui/ObjectPanel.h
  src/ui/ObjectPanel.cpp
  src/ui/TrackerPanel.h
  src/ui/TrackerPanel.cpp
  src/ui/MatrixBoardPanel.h
  src/ui/MatrixBoardPanel.cpp
  src/ui/TutorialPanel.h
  src/ui/TutorialPanel.cpp
  src/ui/LabPanel.h
  src/ui/LabPanel.cpp
  src/core/SnapshotBuffer.cpp
  src/core/MatrixFormat.cpp
  src/render/CommandQueue.cpp
  src/render/FeedbackQueue.cpp
  src/render/RenderThread.cpp
  src/render/D3D11Renderer.cpp
  src/render/DebugDraw.cpp
  src/render/MeshGpu.cpp
  src/render/ShaderSet.cpp
  src/teach/Transforms.cpp
  src/teach/MeshBuild.cpp
  src/teach/TutorialScript.cpp
  src/teach/DemoPlayer.cpp
  src/teach/JsonIo.cpp
)

target_include_directories(3DEditMax PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(3DEditMax PRIVATE Qt5::Widgets d3d11 dxgi d3dcompiler)
if(MSVC)
  target_compile_options(3DEditMax PRIVATE /utf-8)
endif()

add_custom_command(TARGET 3DEditMax POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets/shaders"
    "$<TARGET_FILE_DIR:3DEditMax>/shaders")

enable_testing()
add_executable(3deditmax_tests
  tests/test_main.cpp
  tests/test_queues.cpp
  tests/test_transforms.cpp
  tests/test_tutorial.cpp
  tests/test_json.cpp
  tests/test_lab.cpp
  src/render/CommandQueue.cpp
  src/render/FeedbackQueue.cpp
  src/render/RenderThread.cpp
  src/core/SnapshotBuffer.cpp
  src/core/MatrixFormat.cpp
  src/teach/Transforms.cpp
  src/teach/MeshBuild.cpp
  src/teach/TutorialScript.cpp
  src/teach/DemoPlayer.cpp
  src/teach/JsonIo.cpp
)
target_include_directories(3deditmax_tests PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/tests"
)
target_compile_definitions(3deditmax_tests PRIVATE D3DEDITMAX_NO_D3D)
if(MSVC)
  target_compile_options(3deditmax_tests PRIVATE /utf-8)
endif()
add_test(NAME 3deditmax_tests COMMAND 3deditmax_tests)
