cmake_minimum_required(VERSION 3.5.1)
project(libmbGB)

option(BUILD_EXAMPLE "Enables the example renderer (requires SDL2)." OFF)
option(BUILD_HEADLESS "Enables the headless renderer." OFF)
option(BUILD_LIBRETRO "Enables the Libretro core." OFF)
option(BUILD_GUI "Enables the ImGUI renderer (requires SDL2)." OFF)

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
	libmbGB/include/libmbGB/input.h
	libmbGB/include/libmbGB/libmbgb.h)

set(LIBMBGB
	libmbGB/src/core/utils.cpp
	libmbGB/src/core/addons.cpp
	libmbGB/src/core/mmu.cpp
	libmbGB/src/core/mbc.cpp
	libmbGB/src/core/cpu.cpp
	libmbGB/src/core/gpu.cpp
	libmbGB/src/core/apu.cpp
	libmbGB/src/core/timers.cpp
	libmbGB/src/core/serial.cpp
	libmbGB/src/core/input.cpp
	libmbGB/src/core/ops.cpp
	libmbGB/src/core/libmbgb.cpp)

if (BUILD_EXAMPLE STREQUAL "ON")
	add_subdirectory(example/sdl2)
endif()

if (BUILD_HEADLESS STREQUAL "ON")
	add_subdirectory(example/headless)
endif()

if (BUILD_LIBRETRO STREQUAL "ON")
	add_subdirectory(example/libretro)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

if (BUILD_GUI STREQUAL "ON")
	add_subdirectory(example/sdl2-imgui)
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

if (WIN32 AND BUILD_LIBRETRO STREQUAL "OFF")
	add_library(mbGB SHARED ${LIBMBGB} ${LIBMBGB_API_HEADERS})
else()
	add_library(mbGB STATIC ${LIBMBGB} ${LIBMBGB_API_HEADERS})
endif()

target_include_directories(mbGB PUBLIC ${LIBMBGB_INCLUDE_DIR})
add_library(libmbGB ALIAS mbGB)
