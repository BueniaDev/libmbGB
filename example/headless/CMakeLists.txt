project(headless)

set(EXAMPLE_SOURCES 
	main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(headless ${EXAMPLE_SOURCES})
target_link_libraries(headless libmbGB)
