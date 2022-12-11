/*
    This file is part of libmbGB.
    Copyright (C) 2022 BueniaDev.

    libmbGB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libmbGB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBMBGB_GPU_H
#define LIBMBGB_GPU_H

#include "utils.h"
using namespace gb;

namespace gb
{
    enum GBGPUState : int
    {
	HBlank = 0,
	VBlank = 1,
	OamSearch = 2,
	PixelTransfer = 3
    };

    enum GBFetcherState : int
    {
	FetchTileNumber,
	FetchTileDataLow,
	FetchTileDataHigh,
	PushToFIFO,
	FetchSpriteNumber,
	FetchSpriteDataLow,
	FetchSpriteDataHigh,
	PushToSpriteFIFO
    };

    struct GBFIFOPixel
    {
	int color = 0;
	int palette = 0;
	bool priority = false;
    };

    enum GBSpriteState : int
    {
	ReadingY,
	ReadingX
    };

    struct GBSprite
    {
	uint8_t xpos = 0;
	uint8_t ypos = 0;
	uint16_t addr = 0;
	bool is_rendered = false;
    };

    class LIBMBGB_API GBGPU
    {
	public:
	    GBGPU();
	    ~GBGPU();

	    void init();
	    void shutdown();

	    void tickGPU();

	    uint8_t readVRAM(uint16_t addr);
	    void writeVRAM(uint16_t addr, uint8_t data);

	    uint8_t readOAM(uint8_t addr);
	    void writeOAM(uint8_t addr, uint8_t data);

	    uint8_t readIO(uint16_t addr);
	    void writeIO(uint16_t addr, uint8_t data);

	    vector<GBRGB> getFramebuffer()
	    {
		return framebuffer;
	    }

	    int getDepth()
	    {
		return (160 * sizeof(GBRGB));
	    }

	    void setIRQCallback(voidintfunc cb)
	    {
		irq_func = cb;
	    }

	private:
	    array<uint8_t, 0x2000> vram;
	    array<uint8_t, 0xA0> oam;

	    vector<GBRGB> framebuffer;
	    array<GBRGB, 160> line_buffer;

	    uint8_t reg_lcdc = 0;
	    uint8_t reg_stat = 0;
	    uint8_t reg_scy = 0;
	    uint8_t reg_scx = 0;
	    uint8_t reg_bgp = 0;
	    uint8_t reg_obp0 = 0;
	    uint8_t reg_obp1 = 0;
	    uint8_t reg_winy = 0;
	    uint8_t reg_winx = 0;

	    uint8_t reg_ly = 0;
	    uint8_t reg_lyc = 0;

	    void checkStatIRQ();

	    voidintfunc irq_func;

	    void fireVBlankIRQ()
	    {
		if (irq_func)
		{
		    irq_func(0);
		}
	    }

	    void fireStatIRQ()
	    {
		if (irq_func)
		{
		    irq_func(1);
		}
	    }

	    bool lcd_just_on = false;
	    bool mode2_diff = false;

	    bool is_mode_latch = false;
	    int new_mode = 0;
	    int mode_cycles = 0;
	    int scanline = 0;
	    int dropped_pixels = 0;

	    void checkOAMSearch();

	    void tickFirstLine();

	    GBFetcherState fetcher_state;

	    int pixel_xpos = 0;

	    void startFetcher();
	    void startWindowFetcher();
	    void tickFetcher();

	    int fetcher_counter = 0;

	    bool last_push_success = false;

	    uint8_t tile_num = 0;
	    int tile_index = 0;
	    int tile_line = 0;
	    uint16_t map_addr = 0;
	    uint16_t tile_addr = 0;
	    array<uint8_t, 8> bg_data;

	    uint8_t sprite_num = 0;
	    uint8_t sprite_attribs = 0;
	    uint16_t sprite_addr = 0;
	    array<uint8_t, 8> obj_data;

	    GBFIFO<GBFIFOPixel, 16> bg_fifo;
	    GBFIFO<GBFIFOPixel, 16> obj_fifo;

	    int window_line_counter = 0;
	    bool is_window = false;

	    void setPixel(GBFIFOPixel bg_pixel, GBFIFOPixel obj_pixel);

	    void updateFramebuffer();

	    bool fetcher_begin_delay = false;

	    void pixelTransfer();

	    GBGPUState gpu_state;

	    void setStatMode(GBGPUState state)
	    {
		gpu_state = state;
		new_mode = int(state);
		is_mode_latch = true;
	    }

	    void startOAMSearch();
	    void tickOAMSearch();

	    int sprite_cycles = 0;
	    GBSpriteState sprite_state;

	    int tick_counter = 0;
	    int line_cycles = 0;
	    int frame_cycles = 0;
	    bool prev_stat_irq = false;

	    int sprite_ypos = 0;
	    int sprite_xpos = 0;

	    vector<GBSprite> sprites;
	    GBSprite current_sprite;
	    int sprite_index = 0;
	    int sprite_offs = 0;
	    bool is_sprite_fetch = false;
	    int sprite_line = 0;

	    void addSprite(int xpos, int index);

	    bool spriteInProgress()
	    {
		return ((fetcher_state >= FetchSpriteNumber) && (fetcher_state <= PushToSpriteFIFO));
	    }
    };
};

#endif // LIBMBGB_GPU_H