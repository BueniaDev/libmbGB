project(embed)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DLIBMBGB_STATIC")

set(LIBMBGB_EMBED_HEADERS
	embed/include/libmbGB/utils.h
	embed/include/libmbGB/mmu.h
	embed/include/libmbGB/gpu.h
	embed/include/libmbGB/cpu.h
	embed/include/libmbGB/apu.h
	embed/include/libmbGB/input.h
	embed/include/libmbGB/timers.h
	embed/include/libmbGB/libmbgb.h)

set(LIBMBGB_EMBED
	embed/src/core/mmu.cpp	
	embed/src/core/mbc.cpp
	embed/src/core/utils.cpp
	embed/src/core/cpu.cpp
	embed/src/core/ops.cpp
	embed/src/core/gpu.cpp
	embed/src/core/apu.cpp
	embed/src/core/input.cpp
	embed/src/core/timers.cpp
	embed/src/core/libmbgb.cpp)

set(EXAMPLE_SOURCES 
	main.cpp)

add_executable(embed ${LIBMBGB_EMBED_HEADERS} ${LIBMBGB_EMBED} ${EXAMPLE_SOURCES})
target_link_libraries(embed ${SDL2_LIBRARIES})
