project(example)

find_package(SDL2 REQUIRED)

set(EXAMPLE_SOURCES 
	main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${LIBMBGB_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS})

add_executable(example ${EXAMPLE_SOURCES})
target_link_libraries(example libmbGB ${SDL2_LIBRARIES})
