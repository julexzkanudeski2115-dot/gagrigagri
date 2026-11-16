cmake_minimum_required(VERSION 3.20)
project(adrenaline_win CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.5
)
FetchContent_MakeAvailable(imgui)

add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

add_executable(adrenaline_win WIN32
    src/main.cpp
    src/memory.cpp
    src/game.cpp
    src/esp.cpp
    src/aimbot.cpp
    src/menu.cpp
    src/overlay.cpp
)

target_link_libraries(adrenaline_win PRIVATE
    imgui_lib
    d3d11
    dxgi
    dwmapi
)

if(MSVC)
    target_compile_options(adrenaline_win PRIVATE /W4 /permissive-)
else()
    target_compile_options(adrenaline_win PRIVATE -Wall -Wextra)
endif()
