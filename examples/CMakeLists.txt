cmake_minimum_required(VERSION 3.20)

add_executable(demo demo.c)
target_link_libraries(demo PRIVATE baba)

add_executable(calculator calculator.c)
target_link_libraries(calculator PRIVATE baba)

if(APPLE)
    set_target_properties(demo calculator PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/MacOSXBundleInfo.plist.in
    )
endif()