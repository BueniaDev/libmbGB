project(mbGB-SDL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXAMPLE_SOURCES 
	main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSDL_MAIN_HANDLED")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message(STATUS ${CMAKE_MODULE_PATH})

add_executable(${PROJECT_NAME} ${EXAMPLE_SOURCES})

include_directories(${LIBMBGB_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} libmbGB)

find_package(SDL2 REQUIRED)
find_package(SDL2_net REQUIRED)

if (TARGET SDL2::SDL2)
    target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
else()
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
endif()

target_link_libraries(${PROJECT_NAME} SDL2_net::SDL2_net)

