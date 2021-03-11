project(example)

option(BUILD_WEBCAM "Enables webcam support for the Game Boy Camera implementation (requires OpenCV)." OFF)
option(BUILD_OPENGL "Enables the hardware-accelerated OpenGL rendering backend." OFF)

set(EXAMPLE_SOURCES 
	toml.h
	main.h
	main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -DSDL_MAIN_HANDLED")

add_executable(example ${EXAMPLE_SOURCES})

include_directories(${LIBMBGB_INCLUDE_DIR})
target_link_libraries(example libmbGB)

if (BUILD_WEBCAM STREQUAL "ON")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLIBMBGB_CAMERA")
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
    target_link_libraries(example ${OpenCV_LIBS})
endif()

if (BUILD_OPENGL STREQUAL "ON")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLIBMBGB_OPENGL")
    find_package(OpenGL REQUIRED)
    find_package(GLUT REQUIRED)
    include_directories(${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})
    target_link_libraries(example ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

    if (WIN32)
	find_package(GLEW REQUIRED)
	include_directories(${GLUT_INCLUDE_DIRS})
	if (TARGET GLEW::GLEW)
	    target_link_libraries(example GLEW::GLEW)
	else()
	    target_link_libraries(example ${GLEW_LIBRARIES})
	endif()
    endif()
endif()

find_package(SDL2 REQUIRED)

if (TARGET SDL2::SDL2)
    target_link_libraries(example SDL2::SDL2)
else()
    target_link_libraries(example ${SDL2_LIBRARIES})
endif()

