cmake_minimum_required(VERSION 3.10)

project(neural_network_project VERSION 0.1
  LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 20) # c++ 20
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
  .
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ops
)

file(GLOB_RECURSE NEURAL_NETWORK_LIB_SOURCES
    "./src/core/*.cpp"
    "./src/ops/*.cpp"
    "./src/stb/*.cpp"
)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
add_library(neural_network STATIC ${NEURAL_NETWORK_LIB_SOURCES})

file(GLOB_RECURSE TEST_SOURCES "./test/*.cpp")

set(WINDOW_FILES
    ./window/main_window.cpp
    ./window/window.cpp
    # 如果有 DrawingCanvas 相关实现，也要加进来
    ./window/drawingcanvas.cpp
)
add_executable(window ${WINDOW_FILES})

target_link_libraries(window PRIVATE neural_network Qt5::Widgets)
foreach(TEST_FILE ${TEST_SOURCES})
  get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

  add_executable(${TEST_NAME} ${TEST_FILE})

  target_link_libraries(${TEST_NAME} PRIVATE neural_network)
endforeach()

