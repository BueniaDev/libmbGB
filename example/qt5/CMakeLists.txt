project(mbGB-Qt)

set(EXAMPLE_SOURCES
	mainwindow.h
	mainwindow.cpp
	mainwindow.ui 
	main.cpp)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC -DSDL_MAIN_HANDLED")

include_directories(${LIBMBGB_INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${EXAMPLE_SOURCES})

find_package(SDL2 REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

target_link_libraries(${PROJECT_NAME} libmbGB)

if (TARGET SDL2::SDL2)
    target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
else()
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
endif()

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets)