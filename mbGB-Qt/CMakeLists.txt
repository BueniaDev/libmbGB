project(mbGB-Qt)

option(BUILD_QT6 "Build mbGB-Qt with Qt6." OFF)

if (BUILD_QT6 STREQUAL "ON")
    message(WARNING "Qt6 support is still a huge WIP, and may not even compile correctly, so be EXTREMELY careful here!")
    message(STATUS "Building mbGB-Qt with Qt6.")
    find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets REQUIRED)
    set(QT_LINK_LIBS Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)
else()
    message(STATUS "Building mbGB-Qt with Qt5.")
    find_package(Qt5 COMPONENTS Core Gui Widgets Multimedia MultimediaWidgets REQUIRED)
    set(QT_LINK_LIBS Qt5::Core Qt5::Gui Qt5::Widgets)
endif()


set(MBGB_QT_SOURCES
	qaudioprocessor.h
	coreupdater.h
	qgbcore.h
	displaywidget.h
	displaywidget.cpp
	mainwindow.ui
	mainwindow.h
	mainwindow.cpp
	main.cpp)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
else()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC")

include_directories(${LIBMBGB_INCLUDE_DIR})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${MBGB_QT_SOURCES})
target_link_libraries(${PROJECT_NAME} libmbGB ${QT_LINK_LIBS})

if (TARGET SDL2::SDL2)
    target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
else()
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
endif()