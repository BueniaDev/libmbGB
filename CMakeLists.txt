cmake_minimum_required(VERSION 3.0.2)
project(libmbGB)

option(BUILD_HEADLESS "Enables the headless renderer." OFF)

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
	libmbGB/include/libmbGB/enums.h
	libmbGB/include/libmbGB/mmu.h
	libmbGB/include/libmbGB/cpu.h)

set(LIBMBGB
	libmbGB/src/core/mmu.cpp
	libmbGB/src/core/cpu.cpp
	libmbGB/src/core/ops.cpp)

if (BUILD_HEADLESS STREQUAL "ON")
	add_subdirectory(example/headless)
endif()

if (WIN32)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -DLIBMBGB_EXPORTS")
		message(STATUS "Compiler is MinGW.")
	else()
		message(SEND_ERROR "Compiler not supported.")
		return()
	endif()
elseif (UNIX AND NOT APPLE)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security")
		message(STATUS "Compiler is GCC.")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security")
		message(STATUS "Compiler is Clang.")
	else()
		message(SEND_ERROR "Compiler not supported.")
		return()
	endif()
elseif (APPLE)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security")
		message(STATUS "Compiler is AppleClang.")
	else()
		message(SEND_ERROR "Compiler not supported.")
		return()
	endif()
endif()

if (WIN32)
	add_library(mbGB SHARED ${LIBMBGB} ${LIBMBGB_API_HEADERS})
else()
	add_library(mbGB ${LIBMBGB} ${LIBMBGB_API_HEADERS})
endif()
add_library(libmbGB ALIAS mbGB)
