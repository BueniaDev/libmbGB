project(headless)

set(EXAMPLE_SOURCES 
	main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${LIBMBGB_INCLUDE_DIR})

add_executable(headless ${EXAMPLE_SOURCES})
target_link_libraries(headless libmbGB)
