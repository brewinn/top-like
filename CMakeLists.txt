cmake_minimum_required(VERSION 3.11)
project(top-like
  VERSION 1.0
  DESCRIPTION "Topish program"
  LANGUAGES CXX
)
add_executable(${PROJECT_NAME} src/main.cpp src/cpu.cpp)
target_include_directories(${PROJECT_NAME}
  PRIVATE
  src/)
