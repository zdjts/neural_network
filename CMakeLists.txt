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
    ./app/main_window.cpp
    ./app/window.cpp
    ./app/drawingcanvas.cpp
)
add_executable(window ${WINDOW_FILES})

# 修正: 更改 executable 名称为 'training' (原为 'traning')
# 新增: 链接 'training' executable 到 'neural_network' 库
add_executable(training "./training/train.cpp")
target_link_libraries(training PRIVATE neural_network)

target_link_libraries(window PRIVATE neural_network Qt5::Widgets)

foreach(TEST_FILE ${TEST_SOURCES})
  get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

  add_executable(${TEST_NAME} ${TEST_FILE})

  target_link_libraries(${TEST_NAME} PRIVATE neural_network)
endforeach()
