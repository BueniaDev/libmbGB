project(example)

set(EXAMPLE_SOURCES 
	main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -DSDL_MAIN_HANDLED")

include_directories(${LIBMBGB_INCLUDE_DIR})

add_executable(example ${EXAMPLE_SOURCES})

find_package(SDL2 REQUIRED)
target_link_libraries(example PRIVATE libmbGB SDL2::SDL2)

