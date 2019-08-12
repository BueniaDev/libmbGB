project(example)

find_package(SDL2 REQUIRED)

set(EXAMPLE_SOURCES 
	main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if (ENABLE_SIGNED16 STREQUAL "ON")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -DLIBMBGB_SIGNED16=1")
elseif (ENABLE_FLOAT32 STREQUAL "ON")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -DLIBMBGB_FLOAT32=1")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -DLIBMBGB_SIGNED16=1")
endif()

include_directories(${LIBMBGB_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS})

add_executable(example ${EXAMPLE_SOURCES})
target_link_libraries(example libmbGB ${SDL2_LIBRARIES})
