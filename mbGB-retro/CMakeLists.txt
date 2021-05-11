project(mbGB-retro)

set(RETRO_SOURCES
	main.cpp)

set(LIBRETRO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/libretro-common/include")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC")

add_library(${PROJECT_NAME} SHARED ${RETRO_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${LIBMBGB_INCLUDE_DIR} ${LIBRETRO_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE libmbGB)
