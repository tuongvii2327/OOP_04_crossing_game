cmake_minimum_required(VERSION 3.15)
project(CrossyRoad)


if(MSVC)
    message(STATUS "IDE: Visual Studio.")
    set(SFML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/SFML-3.1.0-windows-vc17-64-bit/lib/cmake/SFML")
else()
    message(STATUS "IDE: VS Code / MinGW.")
    set(SFML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/SFML-3.1.0-windows-gcc-14.2.0-mingw-64-bit/lib/cmake/SFML")
endif()


find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Audio)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(CrossyRoad ${SOURCES})


if(MSVC)
    target_include_directories(CrossyRoad PRIVATE include)
    target_include_directories(CrossyRoad SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libs/SFML-3.1.0-windows-vc17-64-bit/include")
else()
    target_include_directories(CrossyRoad PRIVATE include)
    target_include_directories(CrossyRoad SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libs/SFML-3.1.0-windows-gcc-14.2.0-mingw-64-bit/include")
    
endif()

target_link_libraries(CrossyRoad PRIVATE SFML::Graphics SFML::Window SFML::System SFML::Audio)


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