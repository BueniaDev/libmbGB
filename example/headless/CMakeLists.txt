project(headless)

set(EXAMPLE_SOURCES 
	main.cpp)

add_executable(headless ${EXAMPLE_SOURCES})
if (WIN32)
	target_link_libraries(headless mbGB)
else()
	target_link_libraries(headless mbGB-s)
endif()
