cmake_minimum_required(VERSION 3.15)
project(libmbGB)

# Require C++17 (and position independent code)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BUILD_EXAMPLES "Enables the headless renderer." OFF)
option(BUILD_LIBRETRO "Enables the Libretro core." OFF)
option(BUILD_SDL2 "Enables the example renderer (requires SDL2)." OFF)
option(BUILD_QT "Enables the (WIP) Qt frontend (requires SDL2 and Qt)." OFF)

set(LIBMBGB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libmbGB/include")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libmbGB/include/libmbGB")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(LIBMBGB_API_HEADERS
	libmbGB/include/libmbGB/libmbgb_api.h
	libmbGB/include/libmbGB/utils.h
	libmbGB/include/libmbGB/enums.h
	libmbGB/include/libmbGB/mmu.h
	libmbGB/include/libmbGB/cpu.h
	libmbGB/include/libmbGB/gpu.h
	libmbGB/include/libmbGB/apu.h
	libmbGB/include/libmbGB/timers.h
	libmbGB/include/libmbGB/addons.h
	libmbGB/include/libmbGB/serial.h
	libmbGB/include/libmbGB/irdevices.h
	libmbGB/include/libmbGB/infrared.h
	libmbGB/include/libmbGB/input.h
	libmbGB/include/libmbGB/libmbgb.h)

set(LIBMBGB
	libmbGB/src/core/utils.cpp
	libmbGB/src/core/addons.cpp
	libmbGB/src/core/irdevices.cpp
	libmbGB/src/core/mmu.cpp
	libmbGB/src/core/mbc.cpp
	libmbGB/src/core/cpu.cpp
	libmbGB/src/core/gpu.cpp
	libmbGB/src/core/apu.cpp
	libmbGB/src/core/timers.cpp
	libmbGB/src/core/serial.cpp
	libmbGB/src/core/infrared.cpp
	libmbGB/src/core/input.cpp
	libmbGB/src/core/ops.cpp
	libmbGB/src/core/libmbgb.cpp)

if (BUILD_LIBRETRO STREQUAL "ON")
	message(STATUS "Building mbGB-retro...")
	add_subdirectory(mbGB-retro)
endif()

if (BUILD_SDL2 STREQUAL "ON")
	message(STATUS "Building mbGB-SDL2...")
	add_subdirectory(mbGB-SDL2)
endif()

if (BUILD_EXAMPLES STREQUAL "ON")
	add_subdirectory(examples)
endif()

if (BUILD_QT STREQUAL "ON")
	message(STATUS "Building mbGB-Qt...")
	message(WARNING "mbGB-Qt is still a huge WIP, so tread lightly here!")
	add_subdirectory(mbGB-Qt)
endif()

add_library(mbGB ${LIBMBGB} ${LIBMBGB_API_HEADERS})
target_include_directories(mbGB PUBLIC ${LIBMBGB_INCLUDE_DIR})
target_compile_definitions(mbGB PRIVATE LIBMBGB_STATIC=1 _CRT_SECURE_NO_WARNINGS=1)
add_library(libmbGB ALIAS mbGB)

if (WIN32)
    message(STATUS "Operating system is Windows.")
    if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	target_compile_options(mbGB PRIVATE -Wall)
	message(STATUS "Compiler is MinGW.")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
	message(WARNING "MSVC support is HIGHLY experimental, and may not even compile correctly, so be EXTREMELY careful here!")
	target_compile_options(mbGB PRIVATE /W4)
	message(STATUS "Compiler is MSVC.")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
	message(WARNING "Clang support on Windows is HIGHLY experimental, and may not even compile correctly, so be EXTREMELY careful here!")
	target_compile_options(mbGB PRIVATE -Wall)
	message(STATUS "Compiler is Clang.")
    else()
	message(SEND_ERROR "Compiler not supported.")
	return()
    endif()
elseif(UNIX AND NOT APPLE)
    message(STATUS "Operating system is Linux.")
    if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	target_compile_options(mbGB PRIVATE -Wall)
	message(STATUS "Compiler is GCC.")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
	target_compile_options(mbGB PRIVATE -Wall)
	message(STATUS "Compiler is Clang.")
    else()
	message(SEND_ERROR "Compiler not supported.")
	return()
    endif()
elseif(APPLE)
    message(STATUS "Operating system is Mac.")
    if (CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
	target_compile_options(mbGB PRIVATE -Wall)
	message(STATUS "Compiler is AppleClang.")
    else()
	message(SEND_ERROR "Compiler not supported.")
	return()
    endif()
else()
    message(SEND_ERROR "Operating system not supported.")
    return()
endif()
