project(mbgbretro)

set(MBGBRETRO_SOURCES
	main.cpp)

set(LIBRETRO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/libretro-common/include")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC")

add_library(mbgbretro SHARED ${MBGBRETRO_SOURCES})
target_include_directories(mbgbretro PUBLIC ${LIBMBGB_INCLUDE_DIR} ${LIBRETRO_INCLUDE_DIR})

target_link_libraries(mbgbretro PRIVATE libmbGB)
