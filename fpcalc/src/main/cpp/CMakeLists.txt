# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)
cmake_policy(SET CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)

project("fpcalc")

# chromaprint 配置
set(BUILD_TESTS OFF)
set(BUILD_TOOLS ON)
set(FFT_LIB "avfft")
set(AUDIO_PROCESSOR_LIB "swresample")

set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/../../../libs/${ANDROID_ABI})

# FFMpeg 路径配置
set(FFMPEG_LIBS ${FFMPEG_DIR}/lib)
set(FFMPEG_INCLUDE ${FFMPEG_DIR}/include)
set(ENV{FFMPEG_DIR} ${FFMPEG_DIR})

# 引入 FFMpeg
add_library(lib_swresample STATIC IMPORTED)
set_target_properties(lib_swresample PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIBS}/libswresample.so)

add_library(lib_avformat STATIC IMPORTED)
set_target_properties(lib_avformat PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIBS}/libavformat.so)

add_library(lib_avcodec STATIC IMPORTED)
set_target_properties(lib_avcodec PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIBS}/libavcodec.so)

add_library(lib_avutil STATIC IMPORTED)
set_target_properties(lib_avutil PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIBS}/libavutil.so)

# 引入 chromaprint
add_subdirectory(chromaprint)

add_library(
        ${CMAKE_PROJECT_NAME} SHARED
        fpcalc.cpp
)

target_include_directories(
        ${CMAKE_PROJECT_NAME} PRIVATE
        ${FFMPEG_INCLUDE}
        "chromaprint/src/cmd"
)

target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        android
        log
        fpcalc_libs
        chromaprint
        lib_avformat
        lib_avcodec
        lib_avutil
        lib_swresample
)
