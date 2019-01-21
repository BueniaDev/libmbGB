cmake_minimum_required(VERSION 2.8.11)
project(libmbGB)

option(BUILD_EXAMPLE "Enables the example renderer (requires SDL2)." OFF)
option(BUILD_HEADLESS "Enables headless example project." OFF)

if (WIN32)
	message(STATUS "Operating system is Windows.")
elseif(UNIX)
	message(STATUS "Operating system is Linux.")
elseif(APPLE)
	message(STATUS "Operating system is Mac.")
else()
	message(SEND_ERROR "Operating system not supported.")
endif()

set(LIBMBGB_API_HEADERS
	include/libmbGB/libmbgb_api.h
	include/libmbGB/utils.h
	include/libmbGB/mmu.h
	include/libmbGB/gpu.h
	include/libmbGB/cpu.h
	include/libmbGB/apu.h
	include/libmbGB/input.h
	include/libmbGB/timers.h
	include/libmbGB/libmbgb.h)

set(LIBMBGB
	src/core/mmu.cpp	
	src/core/mbc.cpp
	src/core/utils.cpp
	src/core/cpu.cpp
	src/core/ops.cpp
	src/core/gpu.cpp
	src/core/apu.cpp
	src/core/input.cpp
	src/core/timers.cpp
	src/core/libmbgb.cpp)

if (BUILD_EXAMPLE STREQUAL "ON")
	add_subdirectory(example/sdl2)
endif()

if (BUILD_HEADLESS STREQUAL "ON")
	add_subdirectory(example/headless)
endif()

if (WIN32)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -D LIBMBGB_EXPORTS")
		message(STATUS "Compiler is MinGW.")
	else()
		message(SEND_ERROR "Compiler not supported.")
	endif()
elseif (UNIX)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -D LIBMBGB_EXPORTS")
		message(STATUS "Compiler is GCC.")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -D LIBMBGB_EXPORTS")
		message(STATUS "Compiler is Clang.")
	else()
		message(SEND_ERROR "Compiler not supported.")
	endif()
elseif (APPLE)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -D LIBMBGB_EXPORTS")
		message(STATUS "Compiler is GCC.")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -D LIBMBGB_EXPORTS")
		message(STATUS "Compiler is Clang.")
	else()
		message(SEND_ERROR "Compiler not supported.")
	endif()
endif()

add_library(mbGB ${LIBMBGB} ${LIBMBGB_API_HEADERS})
add_library(libmbGB ALIAS mbGB)
