find_package(SDL2 REQUIRED)

set(IMGUI_SOURCES
	imgui/imgui.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_widgets.cpp
	imgui/imgui_demo.cpp
	imgui/examples/imgui_impl_sdl.cpp
	imgui_sdl/imgui_sdl.cpp)

set(TINY_FD_SOURCES
	tinyfiledialogs/tinyfiledialogs.c)

add_library(imgui STATIC ${IMGUI_SOURCES})
add_library(tinyfd STATIC ${TINY_FD_SOURCES})

target_include_directories(imgui PUBLIC
	imgui/
	imgui_sdl/
	${SDL2_INCLUDE_DIRS})

target_include_directories(tinyfd PUBLIC
	tinyfiledialogs/)

target_link_libraries(imgui PUBLIC ${SDL2_LIBRARIES})
