cmake_minimum_required(VERSION 3.0.2)
project(libmbGB)

option(BUILD_EXAMPLE "Enables the example renderer (requires SDL2)." OFF)
option(BUILD_HEADLESS "Enables headless example project." OFF)
option(ENABLE_AUDIO "Enables audio." OFF)
option(SDL2_AUDIO "Enables SDL2 audio backend (requires SDL2)." OFF)

if ((NOT ENABLE_AUDIO STREQUAL "ON") AND (SDL2_AUDIO STREQUAL "ON"))
	message(SEND_ERROR "Please enable audio in order to build the SDL2 audio backend.")
	return()
endif()

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

set(LIBMBGB_API_HEADERS
	include/libmbGB/libmbgb_api.h
	include/libmbGB/utils.h
	include/libmbGB/mmu.h
	include/libmbGB/gpu.h
	include/libmbGB/cpu.h
	include/libmbGB/apu.h
	include/libmbGB/input.h
	include/libmbGB/timers.h
	include/libmbGB/libmbgb.h
	audio-backends/null/nullbackend.h
	audio-backends/sdl2/sdl2backend.h)

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
	src/core/libmbgb.cpp
	audio-backends/null/nullbackend.cpp
	audio-backends/sdl2/sdl2backend.cpp)

if (BUILD_EXAMPLE STREQUAL "ON")
	add_subdirectory(example/sdl2)
endif()

if (BUILD_HEADLESS STREQUAL "ON")
	add_subdirectory(example/headless)
endif()

if (WIN32)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		if ((ENABLE_AUDIO STREQUAL "ON") AND (SDL2_AUDIO STREQUAL "ON"))	
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -DLIBMBGB_EXPORTS -DSDL2_AUDIO")			
			message(STATUS "SDL2 audio backend selected.")
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -DNULL_AUDIO")
			message(STATUS "NULL audio backend selected.")
		endif()
		message(STATUS "Compiler is MinGW.")
	else()
		message(SEND_ERROR "Compiler not supported.")
		return()
	endif()
elseif (UNIX AND NOT APPLE)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		if ((ENABLE_AUDIO STREQUAL "ON") AND (SDL2_AUDIO STREQUAL "ON"))		
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -DSDL2_AUDIO")			
			message(STATUS "SDL2 audio backend selected.")
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -DNULL_AUDIO")
			message(STATUS "NULL audio backend selected.")
		endif()
		message(STATUS "Compiler is GCC.")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		if ((ENABLE_AUDIO STREQUAL "ON") AND (SDL2_AUDIO STREQUAL "ON"))		
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -DSDL2_AUDIO")
			message(STATUS "SDL2 audio backend selected.")
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -DNULL_AUDIO")
			message(STATUS "NULL audio backend selected.")
		endif()
		message(STATUS "Compiler is Clang.")
	else()
		message(SEND_ERROR "Compiler not supported.")
		return()
	endif()
elseif (APPLE)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
		if ((ENABLE_AUDIO STREQUAL "ON") AND (SDL2_AUDIO STREQUAL "ON"))		
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -DSDL2_AUDIO")
			message(STATUS "SDL2 audio backend selected.")
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -DNULL_AUDIO")
			message(STATUS "NULL audio backend selected.")
		endif()
		message(STATUS "Compiler is AppleClang.")
	else()
		message(SEND_ERROR "Compiler not supported.")
		return()
	endif()
endif()


if ((ENABLE_AUDIO STREQUAL "ON") AND (SDL2_AUDIO STREQUAL "ON"))
	find_package(SDL2 REQUIRED)
	include_directories(${SDL2_INCLUDE_DIRS})
endif()

if (WIN32)
	add_library(mbGB SHARED ${LIBMBGB} ${LIBMBGB_API_HEADERS})
else()
	add_library(mbGB ${LIBMBGB} ${LIBMBGB_API_HEADERS})
endif()
add_library(libmbGB ALIAS mbGB)

if ((ENABLE_AUDIO STREQUAL "ON") AND (SDL2_AUDIO STREQUAL "ON"))
	target_link_libraries(mbGB ${SDL2_LIBRARIES})
endif()
