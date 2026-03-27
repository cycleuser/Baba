cmake_minimum_required(VERSION 3.20)
project(Baba VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BABA_BUILD_EXAMPLES "Build example applications" ON)
option(BABA_BUILD_TESTS "Build tests" OFF)
option(BABA_CROSS_COMPILE "Cross-compile mode (stub platform code)" OFF)
option(BABA_TARGET_PLATFORM "Target platform: macos, linux, windows" "")

if(BABA_CROSS_COMPILE AND BABA_TARGET_PLATFORM)
    message(STATUS "Cross-compiling for: ${BABA_TARGET_PLATFORM}")
endif()

if(NOT BABA_CROSS_COMPILE)
    find_package(Vulkan REQUIRED)
endif()

set(BABA_SOURCES
    src/core/app.c
    src/core/window.c
    src/platform/platform.c
    src/render/vulkan_renderer.c
    src/render/painter.c
    src/widgets/widget.c
    src/widgets/button.c
    src/widgets/label.c
    src/widgets/textbox.c
    src/widgets/container.c
    src/layout/layout.c
    src/theme/theme.c
)

if(BABA_CROSS_COMPILE)
    list(APPEND BABA_SOURCES
        src/platform/stub.c
    )
else()
    if(APPLE)
        list(APPEND BABA_SOURCES
            src/platform/macos/window.m
            src/platform/macos/surface.m
        )
        enable_language(OBJC)
        set_source_files_properties(src/platform/macos/*.m PROPERTIES LANGUAGE OBJC)
        
    elseif(WIN32)
        list(APPEND BABA_SOURCES
            src/platform/windows/window.c
        )
        
    elseif(UNIX AND NOT APPLE)
        find_package(PkgConfig REQUIRED)
        
        if(BABA_USE_X11 OR NOT DEFINED BABA_USE_X11)
            pkg_check_modules(X11 x11)
            if(X11_FOUND)
                list(APPEND BABA_SOURCES
                    src/platform/linux/x11_window.c
                )
                set(BABA_HAS_X11 TRUE)
            endif()
        endif()
        
        if(BABA_USE_WAYLAND OR NOT DEFINED BABA_USE_WAYLAND)
            pkg_check_modules(WAYLAND wayland-client)
            if(WAYLAND_FOUND)
                list(APPEND BABA_SOURCES
                    src/platform/linux/wayland_window.c
                )
                set(BABA_HAS_WAYLAND TRUE)
            endif()
        endif()
        
        if(NOT BABA_HAS_X11 AND NOT BABA_HAS_WAYLAND)
            message(WARNING "Neither X11 nor Wayland found, using stub platform")
            list(APPEND BABA_SOURCES src/platform/stub.c)
        endif()
    endif()
endif()

add_library(baba STATIC ${BABA_SOURCES})

target_include_directories(baba PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(NOT BABA_CROSS_COMPILE)
    target_include_directories(baba PUBLIC ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(baba PRIVATE ${Vulkan_LIBRARIES})
endif()

target_compile_definitions(baba PRIVATE
    $<$<CONFIG:Debug>:BABA_DEBUG>
)

if(BABA_CROSS_COMPILE)
    if(BABA_TARGET_PLATFORM STREQUAL "macos")
        target_compile_definitions(baba PRIVATE BABA_PLATFORM_MACOS)
    elseif(BABA_TARGET_PLATFORM STREQUAL "linux")
        target_compile_definitions(baba PRIVATE BABA_PLATFORM_LINUX)
    elseif(BABA_TARGET_PLATFORM STREQUAL "windows")
        target_compile_definitions(baba PRIVATE BABA_PLATFORM_WINDOWS WIN32_LEAN_AND_MEAN NOMINMAX)
    endif()
else()
    if(APPLE)
        target_link_libraries(baba PRIVATE
            "-framework Cocoa"
            "-framework QuartzCore"
            "-framework Metal"
        )
        target_compile_definitions(baba PRIVATE BABA_PLATFORM_MACOS)
        
    elseif(WIN32)
        target_link_libraries(baba PRIVATE user32 gdi32)
        target_compile_definitions(baba PRIVATE 
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            BABA_PLATFORM_WINDOWS
        )
        
        if(MINGW)
            target_compile_options(baba PRIVATE -Wall -Wextra)
        endif()
        
    elseif(UNIX AND NOT APPLE)
        if(BABA_HAS_X11)
            target_include_directories(baba PRIVATE ${X11_INCLUDE_DIRS})
            target_link_libraries(baba PRIVATE ${X11_LIBRARIES})
            target_compile_definitions(baba PRIVATE BABA_HAS_X11)
        endif()
        
        if(BABA_HAS_WAYLAND)
            target_include_directories(baba PRIVATE ${WAYLAND_INCLUDE_DIRS})
            target_link_libraries(baba PRIVATE ${WAYLAND_LIBRARIES})
            target_compile_definitions(baba PRIVATE BABA_HAS_WAYLAND)
        endif()
        
        target_compile_definitions(baba PRIVATE BABA_PLATFORM_LINUX)
    endif()
endif()

if(BABA_BUILD_EXAMPLES AND NOT BABA_CROSS_COMPILE)
    add_subdirectory(examples)
endif()

if(BABA_BUILD_TESTS AND NOT BABA_CROSS_COMPILE)
    enable_testing()
endif()

install(TARGETS baba
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(DIRECTORY src/ DESTINATION include/baba
    FILES_MATCHING PATTERN "*.h"
)

message(STATUS "")
message(STATUS "Baba / 把拔 GUI Framework Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID}")
if(BABA_CROSS_COMPILE)
    message(STATUS "  Cross-compile target: ${BABA_TARGET_PLATFORM}")
else()
    if(APPLE)
        message(STATUS "  Vulkan: ${Vulkan_LIBRARIES}")
    elseif(UNIX AND NOT APPLE)
        message(STATUS "  X11: ${BABA_HAS_X11}")
        message(STATUS "  Wayland: ${BABA_HAS_WAYLAND}")
    elseif(WIN32)
        message(STATUS "  Toolchain: ${CMAKE_C_COMPILER_ID}")
    endif()
endif()
message(STATUS "")