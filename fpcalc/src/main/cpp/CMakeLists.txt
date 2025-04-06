# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)
cmake_policy(SET CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_BUILD_TYPE "RELEASE")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)

project("fpcalc")

# chromaprint 配置
set(BUILD_TESTS OFF)
set(BUILD_TOOLS OFF)
set(FFT_LIB "kissfft")
set(AUDIO_PROCESSOR_LIB "swresample")

set(EXTERNAL_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/../../../libs/${ANDROID_ABI})

# FFMpeg 路径配置
set(FFMPEG_LIBS ${EXTERNAL_LIBRARY_DIR}/lib)
set(FFMPEG_INCLUDE ${EXTERNAL_LIBRARY_DIR}/include)
set(ENV{FFMPEG_DIR} ${EXTERNAL_LIBRARY_DIR})
set(ENV{KISSFFT_ROOT} ${CMAKE_SOURCE_DIR}/chromaprint/src/3rdparty/kissfft)

# 引入 FFMpeg
add_library(lib_swresample STATIC IMPORTED)
set_target_properties(lib_swresample PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIBS}/libswresample.a)

add_library(lib_avutil STATIC IMPORTED)
set_target_properties(lib_avutil PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIBS}/libavutil.a)

# 引入 chromaprint
add_subdirectory(chromaprint)

add_library(
        ${CMAKE_PROJECT_NAME} SHARED
        fpcalc.cpp
        fpcalc_jni.cpp
        fpcalc_reader.cpp
)

target_include_directories(
        ${CMAKE_PROJECT_NAME} PRIVATE
        ${FFMPEG_INCLUDE}
        ${CMAKE_CURRENT_SOURCE_DIR}
        "chromaprint/src"
)

target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        android
        mediandk
        log
        chromaprint
        lib_avutil
        lib_swresample
)
