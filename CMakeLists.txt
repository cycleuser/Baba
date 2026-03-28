cmake_minimum_required(VERSION 3.20)
project(Baba VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BABA_BUILD_EXAMPLES "Build example applications" ON)
option(BABA_BUILD_TESTS "Build tests" OFF)
option(BABA_USE_X11 "Use X11 on Linux" ON)
option(BABA_USE_WAYLAND "Use Wayland on Linux" ON)

find_package(Vulkan QUIET)
if(NOT Vulkan_FOUND)
    if(DEFINED Vulkan_INCLUDE_DIRS)
        if(EXISTS "${Vulkan_INCLUDE_DIRS}/vulkan/vulkan.h")
            set(Vulkan_FOUND TRUE)
            message(STATUS "Using Vulkan headers from: ${Vulkan_INCLUDE_DIRS}")
        endif()
    elseif(WIN32 AND DEFINED ENV{VULKAN_SDK})
        set(Vulkan_INCLUDE_DIRS "$ENV{VULKAN_SDK}/Include")
        set(Vulkan_LIBRARIES "$ENV{VULKAN_SDK}/Lib/vulkan.lib")
        if(EXISTS "${Vulkan_INCLUDE_DIRS}/vulkan/vulkan.h")
            set(Vulkan_FOUND TRUE)
            message(STATUS "Found Vulkan SDK from environment: $ENV{VULKAN_SDK}")
        endif()
    endif()
endif()
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found. Install Vulkan SDK or set Vulkan_INCLUDE_DIRS.")
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
    
    if(BABA_USE_X11)
        pkg_check_modules(X11 x11)
        if(X11_FOUND)
            list(APPEND BABA_SOURCES
                src/platform/linux/x11_window.c
            )
            set(BABA_HAS_X11 TRUE)
        endif()
    endif()
    
    if(BABA_USE_WAYLAND)
        pkg_check_modules(WAYLAND wayland-client)
        if(WAYLAND_FOUND)
            list(APPEND BABA_SOURCES
                src/platform/linux/wayland_window.c
            )
            set(BABA_HAS_WAYLAND TRUE)
        endif()
    endif()
    
    if(NOT BABA_HAS_X11 AND NOT BABA_HAS_WAYLAND)
        message(WARNING "Neither X11 nor Wayland found")
    endif()
endif()

add_library(baba STATIC ${BABA_SOURCES})

target_include_directories(baba PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Vulkan_INCLUDE_DIRS}
)

target_compile_definitions(baba PRIVATE
    $<$<CONFIG:Debug>:BABA_DEBUG>
)

target_link_libraries(baba PRIVATE ${Vulkan_LIBRARIES})

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

if(BABA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BABA_BUILD_TESTS)
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
if(APPLE)
    message(STATUS "  Vulkan: ${Vulkan_LIBRARIES}")
elseif(UNIX AND NOT APPLE)
    message(STATUS "  X11: ${BABA_HAS_X11}")
    message(STATUS "  Wayland: ${BABA_HAS_WAYLAND}")
elseif(WIN32)
    message(STATUS "  Toolchain: ${CMAKE_C_COMPILER_ID}")
endif()
message(STATUS "")