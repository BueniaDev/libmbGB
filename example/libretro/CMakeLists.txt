project(mbgbretro)

set(MBGBRETRO_HEADERS
	libretro.h)

set(MBGBRETRO_SOURCES 
	main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC -DLIBMBGB_STATIC=1")

add_library(mbgbretro SHARED ${MBGBRETRO_SOURCES} ${MBGBRETRO_HEADERS})
target_include_directories(mbgbretro PRIVATE ${LIBMBGB_INCLUDE_DIR})
target_link_libraries(mbgbretro libmbGB)
