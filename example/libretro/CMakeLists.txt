project(mbgbretro)

find_package(SDL2 REQUIRED)

set(MBGBRETRO_HEADERS
	libretro.h)

set(MBGBRETRO_SOURCES 
	main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

include_directories(${LIBMBGB_INCLUDE_DIR})

add_library(mbgbretro SHARED ${MBGBRETRO_SOURCES} ${MBGBRETRO_HEADERS})
target_link_libraries(mbgbretro libmbGB)
