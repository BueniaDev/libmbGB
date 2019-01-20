project(example)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

set(EXAMPLE_SOURCES 
	main.cpp)

add_executable(example ${EXAMPLE_SOURCES})
if (WIN32)
	target_link_libraries(example mbGB ${SDL2_LIBRARIES})
else()
	target_link_libraries(example mbGB-s ${SDL2_LIBRARIES})
endif()
