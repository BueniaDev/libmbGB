cmake_minimum_required(VERSION 2.8.11)
project(libmbGB)

if (WIN32)
	message(STATUS "Operating system is Windows.")
elseif(UNIX)
	message(STATUS "Operating system is Linux.")
else()
	message(SEND_ERROR "Operating system not supported.")
endif()

set(LIBMBGB_API_HEADERS
	include/libmbgb/libmbgb_api.h
	include/libmbgb/utils.h
	include/libmbgb/mmu.h)

set(LIBMBGB
	src/core/mmu.cpp)

if (WIN32)
	if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -D LIBMBGB_EXPORTS")
		message(STATUS "Compiler is MinGW.")
	else()
		message(SEND_ERROR "Compiler not supported.")
	endif()
elseif (UNIX)
	if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -D LIBMBGB_EXPORTS")
		message(STATUS "Compiler is GCC.")
	else()
		message(SEND_ERROR "Compiler not supported.")
	endif()
endif()

add_library(mbGB SHARED ${LIBMBGB} ${LIBMBGB_API_HEADERS})

add_library(mbGB-s STATIC ${LIBMBGB} ${LIBMBGB_API_HEADERS})