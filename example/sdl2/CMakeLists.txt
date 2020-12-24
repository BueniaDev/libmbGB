project(example)

option(BUILD_WEBCAM "Enables webcam support for the Game Boy Camera implementation." OFF)

set(EXAMPLE_SOURCES 
	toml.h
	main.h
	main.cpp)

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -DSDL_MAIN_HANDLED")

include_directories(${LIBMBGB_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

add_executable(example ${EXAMPLE_SOURCES})

if (BUILD_WEBCAM STREQUAL "ON")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLIBMBGB_CAMERA")
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
    target_link_libraries(example ${OpenCV_LIBS})
endif()

find_package(SDL2 REQUIRED)

target_link_libraries(example libmbGB ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

if (TARGET GLEW::GLEW)
    target_link_libraries(example GLEW::GLEW)
else()
    target_link_libraries(example ${GLEW_LIBRARIES})
endif()

if (TARGET SDL2::SDL2)
    target_link_libraries(example SDL2::SDL2)
else()
    target_link_libraries(example ${SDL2_LIBRARIES})
endif()

