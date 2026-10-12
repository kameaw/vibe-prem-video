cmake_minimum_required(VERSION 3.21)

if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()

project(vibe-prem-video
    VERSION 0.1.0
    DESCRIPTION "Windows NLE (Premiere-like) for self-education"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt 6 — set CMAKE_PREFIX_PATH to your Qt kit, e.g.:
#   -DCMAKE_PREFIX_PATH=C:/Qt/6.7.2/msvc2022_64
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# FFmpeg — optional at configure time so the UI shell can build first.
# Provide FFMPEG_ROOT pointing at a Windows shared/dev build, e.g.:
#   -DFFMPEG_ROOT=C:/ffmpeg
option(VIBE_ENABLE_FFMPEG "Link FFmpeg and enable media probe" ON)

set(VIBE_SOURCES
    src/app/main.cpp
    src/app/MainWindow.cpp
    src/ui/PlaceholderDock.cpp
    src/media/MediaProbe.cpp
)

set(VIBE_HEADERS
    src/app/MainWindow.h
    src/ui/PlaceholderDock.h
    src/media/MediaProbe.h
)

add_executable(vibe-prem-video WIN32
    ${VIBE_SOURCES}
    ${VIBE_HEADERS}
)

target_include_directories(vibe-prem-video PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(vibe-prem-video PRIVATE
    Qt6::Widgets
)

set(VIBE_HAS_FFMPEG FALSE)

if(VIBE_ENABLE_FFMPEG)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    find_package(FFmpeg COMPONENTS avformat avcodec avutil)

    if(FFmpeg_FOUND)
        set(VIBE_HAS_FFMPEG TRUE)
        target_compile_definitions(vibe-prem-video PRIVATE VIBE_HAS_FFMPEG=1)
        target_include_directories(vibe-prem-video PRIVATE ${FFMPEG_INCLUDE_DIRS})
        target_link_libraries(vibe-prem-video PRIVATE ${FFMPEG_LIBRARIES})
        message(STATUS "FFmpeg found — media probe enabled")

        if(WIN32)
            # Prefer bin/ next to include/ (Gyan shared layout).
            get_filename_component(_ffmpeg_root "${FFMPEG_INCLUDE_DIRS}" DIRECTORY)
            set(_ffmpeg_bin "${_ffmpeg_root}/bin")
            if(EXISTS "${_ffmpeg_bin}")
                file(GLOB _ffmpeg_dlls "${_ffmpeg_bin}/*.dll")
                foreach(_dll IN LISTS _ffmpeg_dlls)
                    add_custom_command(TARGET vibe-prem-video POST_BUILD
                        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                            "${_dll}"
                            "$<TARGET_FILE_DIR:vibe-prem-video>"
                        COMMENT "Copy FFmpeg DLL $(FILENAME)"
                    )
                endforeach()
            endif()
        endif()
    else()
        target_compile_definitions(vibe-prem-video PRIVATE VIBE_HAS_FFMPEG=0)
        message(WARNING "FFmpeg not found — UI builds, probe logs a stub. Set FFMPEG_ROOT.")
    endif()
else()
    target_compile_definitions(vibe-prem-video PRIVATE VIBE_HAS_FFMPEG=0)
endif()

if(MSVC)
    target_compile_options(vibe-prem-video PRIVATE /W4 /permissive- /Zc:__cplusplus)
else()
    target_compile_options(vibe-prem-video PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy Qt runtime on Windows when using shared Qt (dev convenience).
if(WIN32)
    get_target_property(_qt_qmake Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qt_qmake}" DIRECTORY)
    set(VIBE_WINDEPLOYQT "${_qt_bin_dir}/windeployqt.exe")
    if(EXISTS "${VIBE_WINDEPLOYQT}")
        add_custom_command(TARGET vibe-prem-video POST_BUILD
            COMMAND "${VIBE_WINDEPLOYQT}"
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                "$<TARGET_FILE:vibe-prem-video>"
            COMMENT "Running windeployqt"
        )
    endif()
endif()
