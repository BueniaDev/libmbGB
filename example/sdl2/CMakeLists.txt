project(example)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(EXAMPLE_SOURCES 
	main.cpp)

add_executable(example ${EXAMPLE_SOURCES})
target_link_libraries(example mbGB ${SDL2_LIBRARIES})
