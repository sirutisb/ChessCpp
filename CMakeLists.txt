cmake_minimum_required(VERSION 3.5...4.0.0)
project(ChessEngine)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

# FetchContent_Declare(ImGui
#     GIT_REPOSITORY https://github.com/ocornut/imgui.git
#     GIT_TAG v1.91.1
#     GIT_SHALLOW ON
#     EXCLUDE_FROM_ALL
#     SYSTEM)
# FetchContent_MakeAvailable(ImGui)
# FetchContent_GetProperties(ImGui SOURCE_DIR IMGUI_DIR)

# set(IMGUI_SFML_FIND_SFML OFF)
# FetchContent_Declare(ImGui-SFML
#     GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
#     GIT_TAG v3.0
#     GIT_SHALLOW ON
#     EXCLUDE_FROM_ALL
#     SYSTEM)
# FetchContent_MakeAvailable(ImGui-SFML)


include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(main src/main.cpp src/board.cpp)

# target_compile_features(main PRIVATE cxx_std_17)
target_include_directories(main PRIVATE include)
target_link_libraries(main PRIVATE SFML::Graphics) # target_link_libraries(main PRIVATE SFML::Graphics ImGui-SFML::ImGui-SFML)