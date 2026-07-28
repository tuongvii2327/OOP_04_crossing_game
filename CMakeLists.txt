set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.15)
project(CrossyRoad)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    message(STATUS "IDE: Visual Studio.")
    set(SFML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/SFML-3.1.0-windows-vc17-64-bit/lib/cmake/SFML")
else()
    message(STATUS "IDE: VS Code / MinGW.")
    set(SFML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/SFML-3.1.0-windows-gcc-14.2.0-mingw-64-bit/lib/cmake/SFML")
endif()

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Audio)

# TỰ ĐỘNG QUÉT TOÀN BỘ FILE TRONG DỰ ÁN
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")

# CHỈ TRUYỀN 2 BIẾN QUÉT TỰ ĐỘNG VÀO ĐÂY, KHÔNG ĐIỀN TÊN FILE THỦ CÔNG
add_executable(CrossyRoad ${SOURCES} ${HEADERS}
    src/states/game_over_state.cpp
    include/states/shop_state.h "src/world/lane_river.cpp" "include/world/lane_river.h" "include/entities/wood.h" "src/entities/wood.cpp")

if(MSVC)
    target_include_directories(CrossyRoad PRIVATE include)
    target_include_directories(CrossyRoad SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libs/SFML-3.1.0-windows-vc17-64-bit/include")
else()
    target_include_directories(CrossyRoad PRIVATE include)
    target_include_directories(CrossyRoad SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libs/SFML-3.1.0-windows-gcc-14.2.0-mingw-64-bit/include")
endif()

target_link_libraries(CrossyRoad PRIVATE SFML::Graphics SFML::Window SFML::System SFML::Audio)

# Copy assets folder to build directory
add_custom_command(TARGET CrossyRoad POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:CrossyRoad>/assets"
)

# Copy data folder to build directory
add_custom_command(TARGET CrossyRoad POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/data"
    "$<TARGET_FILE_DIR:CrossyRoad>/data"
)

if(MSVC)
    add_custom_command(TARGET CrossyRoad POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/SFML-3.1.0-windows-vc17-64-bit/bin"
        "$<TARGET_FILE_DIR:CrossyRoad>"
    )
else()
    add_custom_command(TARGET CrossyRoad POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/SFML-3.1.0-windows-gcc-14.2.0-mingw-64-bit/bin"
        "$<TARGET_FILE_DIR:CrossyRoad>"
    )
endif()