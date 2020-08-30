project(example)

set(EXAMPLE_SOURCES 
	main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -DSDL_MAIN_HANDLED")

include_directories(${LIBMBGB_INCLUDE_DIR})

add_executable(example ${EXAMPLE_SOURCES})

find_package(SDL2 REQUIRED)

target_link_libraries(example PRIVATE libmbGB 

if (TARGET SDL2::SDL2)
    message(STATUS "SDL2 version is 2.0.12.")
    target_link_libraries(example PRIVATE SDL2::SDL2)
else()
    message(STATUS "SDL2 version is below 2.0.12.")
    target_link_libraries(example PRIVATE ${SDL2_LIBRARIES})
endif()

SDL2::SDL2)

