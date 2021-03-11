project(mbGB-Qt)

option(BUILD_QT5 "Build mbGB-Qt with Qt5." ON)
if (UNIX AND NOT APPLE)
    option(BUILD_QT6 "Build mbGB-Qt with Qt6 (highly experimental)." OFF)
endif()

if (BUILD_QT5 STREQUAL "ON")
    message(STATUS "Building mbGB-Qt with Qt5.")
    find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
    set(QT_LINK_LIBS Qt5::Core Qt5::Gui Qt5::Widgets)
elseif(BUILD_QT6 STREQUAL "ON")
    message(STATUS "Building mbGB-Qt with Qt6.")
    message(WARNING "This configuration is highly experimental, and may not build correctly on all supported (Unix) platforms, so be EXTREMELY careful here!")
    find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets REQUIRED)
    set(QT_LINK_LIBS Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)
else()
    message(SEND_ERROR "Invalid Qt framework configuration. Please select at least one and only one Qt framework to link against mbGB-Qt.")
endif()


set(EXAMPLE_SOURCES
	displaywidget.h
	displaywidget.cpp
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

target_link_libraries(${PROJECT_NAME} libmbGB)

if (TARGET SDL2::SDL2)
    target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
else()
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
endif()

target_link_libraries(${PROJECT_NAME} ${QT_LINK_LIBS})