cmake_minimum_required(VERSION 3.1)
project(libmbGB)

option(BUILD_EXAMPLE "Enables the example renderer (requires SDL2)." OFF)
option(BUILD_HEADLESS "Enables the headless renderer." OFF)
option(BUILD_LIBRETRO "Enables the Libretro core." OFF)
option(BUILD_IMGUI "Enables the ImGUI renderer (requires SDL2, deprecated)." OFF)
option(BUILD_QT "Enables the (WIP) Qt frontend (requires SDL2 and Qt)." OFF)

if (WIN32)
	message(STATUS "Operating system is Windows.")
elseif(UNIX AND NOT APPLE)
	message(STATUS "Operating system is Linux.")
elseif(APPLE)
	message(STATUS "Operating system is Mac.")
else()
	message(SEND_ERROR "Operating system not supported.")
	return()
endif()

set(LIBMBGB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libmbGB/include")

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
	if (BUILD_EXAMPLE)
		message(WARNING "Building for Libretro, disabling SDL2 frontend")
		set(BUILD_EXAMPLE OFF)
	endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	add_subdirectory(example/libretro)
endif()

if (BUILD_EXAMPLE STREQUAL "ON")
	add_subdirectory(example/sdl2)
endif()

if (BUILD_HEADLESS STREQUAL "ON")
	add_subdirectory(example/headless)
endif()

if (BUILD_IMGUI STREQUAL "ON")
	message(WARNING "This frontend is deprecated and will be removed sometime in the future.")
	add_subdirectory(example/sdl2-imgui)
endif()

if (BUILD_QT STREQUAL "ON")
	message(WARNING "This frontend is still a huge WIP, so tread lightly here!")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	add_subdirectory(example/qt)
endif()

if (WIN32)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -Wno-format-security -DLIBMBGB_STATIC=1")
		message(STATUS "Compiler is MinGW.")
	else()
		message(SEND_ERROR "Compiler not supported.")
		return()
	endif()
elseif (UNIX AND NOT APPLE)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -Wno-format-security")
		message(STATUS "Compiler is GCC.")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -Wno-format-security")
		message(STATUS "Compiler is Clang.")
	else()
		message(SEND_ERROR "Compiler not supported.")
		return()
	endif()
elseif (APPLE)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -Wno-format-security")
		message(STATUS "Compiler is AppleClang.")
	else()
		message(SEND_ERROR "Compiler not supported.")
		return()
	endif()
endif()

add_library(mbGB ${LIBMBGB} ${LIBMBGB_API_HEADERS})

target_include_directories(mbGB PUBLIC ${LIBMBGB_INCLUDE_DIR})
add_library(libmbGB ALIAS mbGB)
