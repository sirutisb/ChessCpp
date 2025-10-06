cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(main src/main.cpp)
# add_executable(benchmark src/benchmark.cpp)

# Add subdirectory for tests
# add_subdirectory(tests)
