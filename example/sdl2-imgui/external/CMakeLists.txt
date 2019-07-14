find_package(SDL2 REQUIRED)

set(IMGUI_SOURCES
	imgui/imgui.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_widgets.cpp
	imgui/imgui_demo.cpp
	imgui_sdl/imgui_sdl.cpp)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC
	imgui/
	imgui_sdl/
	${SDL2_INCLUDE_DIRS})

target_link_libraries(imgui PUBLIC ${SDL2_LIBRARIES})
