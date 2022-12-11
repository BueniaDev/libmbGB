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

#include <gpu.h>
using namespace gb;
using namespace std;

namespace gb
{
    GBGPU::GBGPU()
    {

    }

    GBGPU::~GBGPU()
    {

    }

    void GBGPU::init()
    {
	reg_lcdc = 0x00;
	reg_stat = 0x04;
	reg_ly = 0x00;
	vram.fill(0);
	framebuffer.resize((160 * 144));
    }

    void GBGPU::shutdown()
    {
	framebuffer.clear();
    }

    void GBGPU::tickGPU()
    {
	if (!testbit(reg_lcdc, 7))
	{
	    return;
	}

	if ((scanline == 0) && (tick_counter == 0))
	{
	    if (!lcd_just_on)
	    {
		startOAMSearch();
		setStatMode(OamSearch);
	    }
	}
	else if (inRangeEx(scanline, 1, 143) && (tick_counter == 0))
	{
	    startOAMSearch();
	    setStatMode(OamSearch);
	}
	else if (inRangeEx(scanline, 0, 143) && (tick_counter == 80))
	{
	    startFetcher();
	    setStatMode(PixelTransfer);
	}
	else if ((scanline == 144) && (tick_counter == 0))
	{
	    setStatMode(VBlank);
	}
	else if ((scanline == 144) && (tick_counter == 4))
	{
	    fireVBlankIRQ();
	}

	if (is_mode_latch)
	{
	    mode_cycles += 1;

	    if (mode_cycles == 4)
	    {
		is_mode_latch = false;
		reg_stat = ((reg_stat & ~0x3) | new_mode);
	    }
	}
	else
	{
	    mode_cycles = 0;
	}


	// FIXME: Verify proper STAT IRQ timing
	bool is_stat_irq = false;

	switch (gpu_state)
	{
	    case HBlank:
	    {
		is_stat_irq |= testbit(reg_stat, 3);
	    }
	    break;
	    case VBlank:
	    {
		if ((tick_counter == 4) && (scanline == 144))
		{
		    is_stat_irq |= testbit(reg_stat, 4);
		    is_stat_irq |= testbit(reg_stat, 5);
		}
		else
		{
		    is_stat_irq |= testbit(reg_stat, 4);
		}
	    }
	    break;
	    case OamSearch:
	    {
		tickOAMSearch();
		if (tick_counter == 4)
		{
		    is_stat_irq |= testbit(reg_stat, 5);

		    if (scanline == 0)
		    {
			is_stat_irq |= testbit(reg_stat, 5);
		    }
		}
	    }
	    break;
	    case PixelTransfer: pixelTransfer(); break;
	}

	bool is_lyc = (reg_ly == reg_lyc);
	reg_stat = changebit(reg_stat, 2, is_lyc);

	is_stat_irq |= (testbit(reg_stat, 6) && is_lyc);

	if (!prev_stat_irq && is_stat_irq)
	{
	    fireStatIRQ();
	}

	prev_stat_irq = is_stat_irq;

	if ((scanline == 153) && (tick_counter == 4))
	{
	    reg_ly = 0;
	}

	tick_counter += 1;

	if (tick_counter == 456)
	{
	    tick_counter = 0;

	    scanline += 1;

	    if (scanline == 154)
	    {
		scanline = 0;
		window_line_counter = 0;
	    }

	    reg_ly = scanline;
	}

	lcd_just_on = false;
    }

    void GBGPU::startOAMSearch()
    {
	sprites.clear();
	sprite_cycles = 0;
	sprite_state = ReadingY;
    }

    void GBGPU::tickOAMSearch()
    {
	uint16_t sprite_addr = (sprite_cycles * 4);

	switch (sprite_state)
	{
	    case ReadingY:
	    {
		sprite_ypos = oam.at(sprite_addr);
		sprite_state = ReadingX;
	    }
	    break;
	    case ReadingX:
	    {
		sprite_xpos = oam.at(sprite_addr + 1);

		int sprite_height = testbit(reg_lcdc, 2) ? 16 : 8;

		int sprite_ly = (scanline + 16);

		if ((sprites.size() < 10) && inRange(sprite_ly, sprite_ypos, (sprite_ypos + sprite_height)))
		{
		    GBSprite sprite;
		    sprite.xpos = sprite_xpos;
		    sprite.ypos = sprite_ypos;
		    sprite.addr = sprite_addr;
		    sprite.is_rendered = false;
		    sprites.push_back(sprite);
		}

		sprite_cycles += 1;
		sprite_state = ReadingY;
	    }
	    break;
	}
    }

    void GBGPU::pixelTransfer()
    {
	tickFetcher();

	if (testbit(reg_lcdc, 1))
	{
	    if (spriteInProgress())
	    {
		return;
	    }

	    bool is_sprite_added = false;

	    for (size_t i = 0; i < sprites.size(); i++)
	    {
		auto &sprite = sprites.at(i);

		if (sprite.is_rendered)
		{
		    continue;
		}

		if ((pixel_xpos == 0) && (sprite.xpos < 8))
		{
		    if (!is_sprite_added)
		    {
			addSprite((8 - sprite.xpos), i);
			is_sprite_added = true;
		    }

		    sprite.is_rendered = true;
		}
		else if ((sprite.xpos - 8) == pixel_xpos)
		{
		    if (!is_sprite_added)
		    {
			addSprite(0, i);
			is_sprite_added = true;
		    }

		    sprite.is_rendered = true;
		}

		if (is_sprite_added)
		{
		    return;
		}
	    }
	}

	if (!bg_fifo.empty())
	{
	    GBFIFOPixel bg_pixel = bg_fifo.pop();

	    if (dropped_pixels < (reg_scx & 0x7))
	    {
		dropped_pixels += 1;
		return;
	    }

	    if (!is_window && testbit(reg_lcdc, 5) && (scanline >= reg_winy) && (pixel_xpos >= (reg_winx - 7)))
	    {
		is_window = true;
		startWindowFetcher();
		return;
	    }

	    GBFIFOPixel obj_pixel;

	    if (!obj_fifo.empty())
	    {
		obj_pixel = obj_fifo.pop();
	    }

	    setPixel(bg_pixel, obj_pixel);
	    pixel_xpos += 1;
	}

	if (pixel_xpos == 160)
	{
	    if (is_window)
	    {
		window_line_counter += 1;
	    }

	    updateFramebuffer();
	    setStatMode(HBlank);
	}
    }

    void GBGPU::addSprite(int xpos, int index)
    {
	current_sprite = sprites.at(index);
	sprite_offs = xpos;
	sprite_index = index;
	sprite_line = (scanline + 16 - current_sprite.ypos);

	if (!testbit(reg_lcdc, 2))
	{
	    sprite_line &= 0x7;
	}

	fetcher_state = FetchSpriteNumber;
    }

    void GBGPU::startFetcher()
    {
	uint8_t ypos = ((scanline + reg_scy) & 0xFF);
	map_addr = 0x1800;

	if (testbit(reg_lcdc, 3))
	{
	    map_addr = 0x1C00;
	}

	map_addr += ((ypos >> 3) << 5);
	tile_line = (ypos & 0x7);
	fetcher_state = FetchTileNumber;
	fetcher_begin_delay = false;
	fetcher_counter = 0;
	tile_index = (reg_scx >> 3);
	pixel_xpos = 0;
	dropped_pixels = 0;
	is_window = false;
	bg_fifo.clear();
	obj_fifo.clear();
    }

    void GBGPU::startWindowFetcher()
    {
	uint8_t ypos = (window_line_counter & 0xFF);

	map_addr = 0x1800;

	if (testbit(reg_lcdc, 6))
	{
	    map_addr = 0x1C00;
	}

	map_addr += ((ypos >> 3) << 5);
	tile_line = (ypos & 0x7);
	fetcher_state = FetchTileNumber;
	tile_index = ((pixel_xpos - reg_winx + 7) >> 3);
	bg_fifo.clear();
    }

    void GBGPU::tickFetcher()
    {
	fetcher_counter += 1;
	switch (fetcher_state)
	{
	    case FetchTileNumber:
	    {
		if (fetcher_counter == 2)
		{
		    fetcher_counter = 0;
		    tile_num = vram.at(map_addr + tile_index);
		    fetcher_state = FetchTileDataLow;
		}
	    }
	    break;
	    case FetchTileDataLow:
	    {
		if (fetcher_counter == 2)
		{
		    if (testbit(reg_lcdc, 4))
		    {
			tile_addr = (tile_num << 4);
		    }
		    else
		    {
			tile_addr = (0x1000 + (int8_t(tile_num) << 4));
		    }

		    tile_addr += (tile_line << 1);

		    uint8_t tile_data = vram.at(tile_addr);

		    for (int bit = 0; bit < 8; bit++)
		    {
			bg_data.at(bit) = testbit(tile_data, bit);
		    }

		    fetcher_counter = 0;
		    fetcher_state = FetchTileDataHigh;
		}
	    }
	    break;
	    case FetchTileDataHigh:
	    {
		if (fetcher_counter == 2)
		{
		    uint8_t tile_data = vram.at(tile_addr + 1);

		    for (int bit = 0; bit < 8; bit++)
		    {
			bg_data.at(bit) |= (testbit(tile_data, bit) << 1);
		    }

		    fetcher_counter = 0;
		    fetcher_state = PushToFIFO;

		    if (!fetcher_begin_delay)
		    {
			fetcher_begin_delay = true;
			fetcher_state = FetchTileNumber;
		    }
		}
	    }
	    break;
	    case PushToFIFO:
	    {
		if (fetcher_counter == 1)
		{
		    last_push_success = false;
		}

		if (bg_fifo.empty())
		{
		    last_push_success = true;

		    for (int i = 7; i >= 0; i--)
		    {
			GBFIFOPixel pixel;
			pixel.color = bg_data.at(i);
			bg_fifo.push(pixel);
		    }

		    tile_index = ((tile_index + 1) & 0x1F);
		}

		if (fetcher_counter > 1)
		{
		    if (last_push_success)
		    {
			fetcher_counter = 0;
			fetcher_state = FetchTileNumber;
		    }
		}
	    }
	    break;
	    case FetchSpriteNumber:
	    {
		if (fetcher_counter == 2)
		{
		    fetcher_counter = 0;
		    sprite_num = oam.at(current_sprite.addr + 2);
		    sprite_attribs = oam.at(current_sprite.addr + 3);
		    fetcher_state = FetchSpriteDataLow;
		}
	    }
	    break;
	    case FetchSpriteDataLow:
	    {
		if (fetcher_counter == 2)
		{
		    int sprite_mask = testbit(reg_lcdc, 2) ? 15 : 7;

		    int sprite_yline = sprite_line;

		    bool is_yflip = testbit(sprite_attribs, 6);

		    if (is_yflip)
		    {
			sprite_yline = (sprite_mask - sprite_yline);
		    }

		    if (testbit(reg_lcdc, 2))
		    {
			sprite_num &= 0xFE;
		    }

		    fetcher_counter = 0;

		    sprite_addr = (sprite_num << 4);
		    sprite_addr += (sprite_yline << 1);

		    uint8_t sprite_data = vram.at(sprite_addr);

		    for (int bit = 0; bit < 8; bit++)
		    {
			obj_data.at(bit) = testbit(sprite_data, bit);
		    }

		    fetcher_state = FetchSpriteDataHigh;
		}
	    }
	    break;
	    case FetchSpriteDataHigh:
	    {
		if (fetcher_counter == 2)
		{
		    fetcher_counter = 0;

		    uint8_t sprite_data = vram.at(sprite_addr + 1);

		    for (int bit = 0; bit < 8; bit++)
		    {
			obj_data.at(bit) |= (testbit(sprite_data, bit) << 1);
		    }

		    fetcher_state = PushToSpriteFIFO;
		}
	    }
	    break;
	    case PushToSpriteFIFO:
	    {
		if (fetcher_counter == 2)
		{
		    fetcher_counter = 0;

		    while (obj_fifo.size() < 8)
		    {
			GBFIFOPixel pixel;
			obj_fifo.push(pixel);
		    }

		    bool is_xflip = testbit(sprite_attribs, 5);

		    for (int i = sprite_offs; i < 8; i++)
		    {
			int shift_offs = (7 - i);

			if (is_xflip)
			{
			    shift_offs = i;
			}

			int index = (i - sprite_offs);

			GBFIFOPixel pixel;
			pixel.color = obj_data.at(shift_offs);
			pixel.palette = testbit(sprite_attribs, 4);
			pixel.priority = testbit(sprite_attribs, 7);

			if (obj_fifo.at(index).color == 0)
			{
			    obj_fifo.at(index) = pixel;
			}
		    }

		    fetcher_state = FetchTileNumber;
		}
	    }
	    break;
	    default:
	    {
		cout << "Unrecognized fetcher state of " << dec << int(fetcher_state) << endl;
		exit(0);
	    }
	    break;
	}
    }

    void GBGPU::setPixel(GBFIFOPixel bg_pixel, GBFIFOPixel obj_pixel)
    {
	int bg_pal = bg_pixel.color;
	int obj_pal = obj_pixel.color;

	int bg_color = ((reg_bgp >> (2 * bg_pal)) & 0x3);

	uint8_t reg_obp = (obj_pixel.palette == 0) ? reg_obp0 : reg_obp1;

	if (!testbit(reg_lcdc, 0))
	{
	    bg_color = 0;
	}

	int obj_color = ((reg_obp >> (2 * obj_pal)) & 0x3);

	int pal_color = bg_color;

	if ((obj_pal != 0) && ((bg_pal == 0) || !obj_pixel.priority))
	{
	    pal_color = obj_color;
	}

	array<uint8_t, 4> pixel_colors = {0xFF, 0xAA, 0x55, 0x00};

	uint8_t pixel_color = pixel_colors.at(pal_color);
	line_buffer.at(pixel_xpos) = GBRGB::fromMonochrome(pixel_color);
    }

    void GBGPU::updateFramebuffer()
    {
	for (int xpos = 0; xpos < 160; xpos++)
	{
	    size_t fb_offs = (xpos + (scanline * 160));
	    framebuffer.at(fb_offs) = line_buffer.at(xpos);
	}

	line_buffer.fill(GBRGB());
    }

    uint8_t GBGPU::readVRAM(uint16_t addr)
    {
	addr &= 0x1FFF;
	return vram.at(addr);
    }

    void GBGPU::writeVRAM(uint16_t addr, uint8_t data)
    {
	addr &= 0x1FFF;
	vram.at(addr) = data;
    }

    uint8_t GBGPU::readOAM(uint8_t addr)
    {
	if (addr >= 0xA0)
	{
	    return 0x00;
	}

	return oam.at(addr);
    }

    void GBGPU::writeOAM(uint8_t addr, uint8_t data)
    {
	if (addr >= 0xA0)
	{
	    return;
	}

	oam.at(addr) = data;
    }

    uint8_t GBGPU::readIO(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0xFF;

	switch (addr)
	{
	    case 0x40: data = reg_lcdc; break;
	    case 0x41: data = (reg_stat | 0x80); break;
	    case 0x42: data = reg_scy; break;
	    case 0x43: data = reg_scx; break;
	    case 0x44: data = reg_ly; break;
	    case 0x45: data = reg_lyc; break;
	    default: throw mbGBUnmappedMemory(addr, true); break;
	}

	return data;
    }

    void GBGPU::writeIO(uint16_t addr, uint8_t data)
    {
	addr &= 0xFF;

	switch (addr)
	{
	    case 0x40:
	    {
		bool prev_enable = testbit(reg_lcdc, 7);
		reg_lcdc = data;

		if (!prev_enable && testbit(reg_lcdc, 7))
		{
		    reg_stat &= ~0x3;
		    tick_counter = 4;
		    reg_ly = 0;
		    lcd_just_on = true;
		    mode2_diff = true;
		}
		else if (prev_enable && !testbit(reg_lcdc, 7))
		{
		    reg_stat &= ~0x3;
		    tick_counter = 0;
		    reg_ly = 0;
		}
	    }
	    break;
	    case 0x41:
	    {
		reg_stat = ((reg_stat & 0x7) | (data & 0xF8));
	    }
	    break;
	    case 0x42: reg_scy = data; break;
	    case 0x43: reg_scx = data; break;
	    case 0x44: break; // LY should not be written to
	    case 0x45: reg_lyc = data; break;
	    case 0x47: reg_bgp = data; break;
	    case 0x48: reg_obp0 = data; break;
	    case 0x49: reg_obp1 = data; break;
	    case 0x4A: reg_winy = data; break;
	    case 0x4B: reg_winx = data; break;
	    default: throw mbGBUnmappedMemory(addr, data, true); break;
	}
    }
};