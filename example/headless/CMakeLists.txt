project(headless)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DLIBMBGB_STATIC")

set(LIBMBGB_EMBED_HEADERS
	libmbGB/include/libmbGB/utils.h
	libmbGB/include/libmbGB/mmu.h
	libmbGB/include/libmbGB/gpu.h
	libmbGB/include/libmbGB/cpu.h
	libmbGB/include/libmbGB/apu.h
	libmbGB/include/libmbGB/input.h
	libmbGB/include/libmbGB/timers.h
	libmbGB/include/libmbGB/libmbgb.h)

set(LIBMBGB_EMBED
	libmbGB/src/core/mmu.cpp	
	libmbGB/src/core/mbc.cpp
	libmbGB/src/core/utils.cpp
	libmbGB/src/core/cpu.cpp
	libmbGB/src/core/ops.cpp
	libmbGB/src/core/gpu.cpp
	libmbGB/src/core/apu.cpp
	libmbGB/src/core/input.cpp
	libmbGB/src/core/timers.cpp
	libmbGB/src/core/libmbgb.cpp)

set(EXAMPLE_SOURCES 
	main.cpp)

add_executable(headless ${LIBMBGB_EMBED_HEADERS} ${LIBMBGB_EMBED} ${EXAMPLE_SOURCES})
