# FindFFmpeg.cmake — locates FFmpeg headers/libs on Windows and Unix.
#
# Usage:
#   set(FFMPEG_ROOT "C:/ffmpeg")  # optional hint
#   find_package(FFmpeg COMPONENTS avformat avcodec avutil REQUIRED)
#
# Provides:
#   FFmpeg_FOUND
#   FFMPEG_INCLUDE_DIRS
#   FFMPEG_LIBRARIES
#   FFMPEG_<COMPONENT>_FOUND / FFMPEG_<COMPONENT>_LIBRARY

cmake_policy(PUSH)
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW) # use <PACKAGENAME>_ROOT variables
endif()

set(_FFMPEG_ROOT_HINTS "")
if(DEFINED FFMPEG_ROOT AND NOT "${FFMPEG_ROOT}" STREQUAL "")
    list(APPEND _FFMPEG_ROOT_HINTS "${FFMPEG_ROOT}")
endif()
if(DEFINED ENV{FFMPEG_ROOT} AND NOT "$ENV{FFMPEG_ROOT}" STREQUAL "")
    list(APPEND _FFMPEG_ROOT_HINTS "$ENV{FFMPEG_ROOT}")
endif()
list(APPEND _FFMPEG_ROOT_HINTS "${CMAKE_SOURCE_DIR}/third_party/ffmpeg")

find_path(FFMPEG_INCLUDE_DIR
    NAMES libavformat/avformat.h
    HINTS ${_FFMPEG_ROOT_HINTS}
    PATH_SUFFIXES include
)

set(_ffmpeg_components ${FFmpeg_FIND_COMPONENTS})
if(NOT _ffmpeg_components)
    set(_ffmpeg_components avformat avcodec avutil)
endif()

set(FFMPEG_LIBRARIES "")

foreach(_comp IN LISTS _ffmpeg_components)
    string(TOUPPER "${_comp}" _COMP_UPPER)
    find_library(FFMPEG_${_COMP_UPPER}_LIBRARY
        NAMES ${_comp} lib${_comp}
        HINTS ${_FFMPEG_ROOT_HINTS}
        PATHS ${_FFMPEG_ROOT_HINTS}
        PATH_SUFFIXES lib lib/x64 bin
        NO_DEFAULT_PATH
    )
    # Fallback to default search if hints miss.
    if(NOT FFMPEG_${_COMP_UPPER}_LIBRARY)
        find_library(FFMPEG_${_COMP_UPPER}_LIBRARY
            NAMES ${_comp} lib${_comp}
            HINTS ${_FFMPEG_ROOT_HINTS}
            PATH_SUFFIXES lib lib/x64 bin
        )
    endif()

    if(FFMPEG_${_COMP_UPPER}_LIBRARY)
        set(FFMPEG_${_COMP_UPPER}_FOUND TRUE)
        # Required by find_package_handle_standard_args(HANDLE_COMPONENTS)
        set(FFmpeg_${_comp}_FOUND TRUE)
        list(APPEND FFMPEG_LIBRARIES "${FFMPEG_${_COMP_UPPER}_LIBRARY}")
    else()
        set(FFMPEG_${_COMP_UPPER}_FOUND FALSE)
        set(FFmpeg_${_comp}_FOUND FALSE)
        if(FFmpeg_FIND_REQUIRED_${_comp} OR FFmpeg_FIND_REQUIRED)
            message(STATUS "FFmpeg component not found: ${_comp}")
        endif()
    endif()
    mark_as_advanced(FFMPEG_${_COMP_UPPER}_LIBRARY)
endforeach()

set(FFMPEG_INCLUDE_DIRS "${FFMPEG_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFmpeg
    REQUIRED_VARS FFMPEG_INCLUDE_DIR FFMPEG_LIBRARIES
    HANDLE_COMPONENTS
)

mark_as_advanced(FFMPEG_INCLUDE_DIR)
cmake_policy(POP)
