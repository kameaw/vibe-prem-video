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

find_package(Qt6 REQUIRED COMPONENTS Widgets)

option(VIBE_ENABLE_FFMPEG "Link FFmpeg and enable media I/O" ON)

set(VIBE_SOURCES
    src/app/main.cpp
    src/app/MainWindow.cpp
    src/media/MediaInfo.cpp
    src/media/MediaProbe.cpp
    src/media/FrameCache.cpp
    src/media/VideoDecoder.cpp
    src/media/AudioPeaks.cpp
    src/media/ThumbnailStrip.cpp
    src/media/PcmAudio.cpp
    src/media/WinMmPlayer.cpp
    src/ui/PlaceholderDock.cpp
    src/ui/WaveformWidget.cpp
    src/ui/SourceMonitorWidget.cpp
)

set(VIBE_HEADERS
    src/core/Rational.h
    src/app/MainWindow.h
    src/media/MediaInfo.h
    src/media/MediaProbe.h
    src/media/FrameCache.h
    src/media/VideoDecoder.h
    src/media/AudioPeaks.h
    src/media/ThumbnailStrip.h
    src/media/PcmAudio.h
    src/media/WinMmPlayer.h
    src/ui/PlaceholderDock.h
    src/ui/WaveformWidget.h
    src/ui/SourceMonitorWidget.h
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

if(WIN32)
    target_link_libraries(vibe-prem-video PRIVATE winmm)
endif()

set(VIBE_HAS_FFMPEG FALSE)

if(VIBE_ENABLE_FFMPEG)
    # Prefer our module over Qt's FindFFmpeg.cmake (which can return bare lib names).
    list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    find_package(FFmpeg MODULE COMPONENTS avformat avcodec avutil swscale swresample)

    if(FFmpeg_FOUND)
        set(VIBE_HAS_FFMPEG TRUE)
        target_compile_definitions(vibe-prem-video PRIVATE VIBE_HAS_FFMPEG=1)
        target_include_directories(vibe-prem-video PRIVATE ${FFMPEG_INCLUDE_DIRS})
        target_link_libraries(vibe-prem-video PRIVATE ${FFMPEG_LIBRARIES})
        message(STATUS "FFmpeg include: ${FFMPEG_INCLUDE_DIRS}")
        message(STATUS "FFmpeg libs: ${FFMPEG_LIBRARIES}")
        message(STATUS "FFmpeg found — media I/O enabled")

        if(WIN32)
            get_filename_component(_ffmpeg_root "${FFMPEG_INCLUDE_DIRS}" DIRECTORY)
            set(_ffmpeg_bin "${_ffmpeg_root}/bin")
            if(EXISTS "${_ffmpeg_bin}")
                file(GLOB _ffmpeg_dlls "${_ffmpeg_bin}/*.dll")
                foreach(_dll IN LISTS _ffmpeg_dlls)
                    add_custom_command(TARGET vibe-prem-video POST_BUILD
                        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                            "${_dll}"
                            "$<TARGET_FILE_DIR:vibe-prem-video>"
                        COMMENT "Copy FFmpeg DLL"
                    )
                endforeach()
            endif()
        endif()
    else()
        target_compile_definitions(vibe-prem-video PRIVATE VIBE_HAS_FFMPEG=0)
        message(WARNING "FFmpeg not found — UI builds, media I/O disabled. Set FFMPEG_ROOT.")
    endif()
else()
    target_compile_definitions(vibe-prem-video PRIVATE VIBE_HAS_FFMPEG=0)
endif()

if(MSVC)
    target_compile_options(vibe-prem-video PRIVATE /W4 /permissive- /Zc:__cplusplus)
else()
    target_compile_options(vibe-prem-video PRIVATE -Wall -Wextra -Wpedantic)
endif()

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
