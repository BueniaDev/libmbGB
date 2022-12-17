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

    void GBGPU::init(GBModel model, bool is_bios_load)
    {
	is_cgb_mode = (model >= ModelCgbX);

	if (is_bios_load)
	{
	    reg_lcdc = 0x00;
	    reg_stat = 0x04;
	    reg_ly = 0x00;
	}
	else
	{
	    reg_lcdc = 0x91;
	    reg_stat = 0x05;
	    reg_ly = 0x00;
	    tick_counter = 454;
	    gpu_state = VBlank;
	}

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

	tick_counter += 1;

	if (lcd_just_on)
	{
	    tickFirstLine();
	    return;
	}

	if (is_mode_latch)
	{
	    mode_cycles += 1;

	    if (mode_cycles == 4)
	    {
		is_mode_latch = false;
		mode_cycles = 0;
		reg_stat = ((reg_stat & ~0x3) | new_mode);
	    }
	}

	switch (gpu_state)
	{
	    case HBlank: hblank(); break;
	    case VBlank: vblank(); break;
	    case OamSearch: oamSearch(); break;
	    case PixelTransfer: pixelTransfer(); break;
	}

	checkStatIRQ();
    }

    void GBGPU::checkStatIRQ()
    {
	int current_mode = (reg_stat & 0x3);
	bool is_lyc = (reg_ly == reg_lyc);
	reg_stat = changebit(reg_stat, 2, is_lyc);

	int tick_counter_val = is_cgb_mode ? 0 : 4;

	bool is_stat_irq = (testbit(reg_stat, 6) && is_lyc);
	is_stat_irq |= (testbit(reg_stat, 5) && (current_mode == 2));
	is_stat_irq |= (testbit(reg_stat, 5) && ((tick_counter == tick_counter_val) && (reg_ly == 144)));
	is_stat_irq |= (testbit(reg_stat, 4) && (current_mode == 1));
	is_stat_irq |= (testbit(reg_stat, 3) && (current_mode == 0));

	if (!prev_stat_irq && is_stat_irq)
	{
	    fireStatIRQ();
	}

	prev_stat_irq = is_stat_irq;
    }

    void GBGPU::tickFirstLine()
    {
	if (tick_counter == 80)
	{
	    startFetcher();
	    setStatMode(PixelTransfer);
	    lcd_just_on = false;
	}
    }

    void GBGPU::hblank()
    {
	if (tick_counter == 456)
	{
	    tick_counter = 0;
	    reg_ly += 1;

	    if (reg_ly == 144)
	    {
		setStatMode(VBlank);
	    }
	    else
	    {
		startOamSearch();
		setStatMode(OamSearch);
	    }
	}
    }

    void GBGPU::vblank()
    {
	if ((reg_ly == 144) && (tick_counter == 4))
	{
	    fireVBlankIRQ();
	}

	if ((reg_ly == 153) && (tick_counter == 4))
	{
	    reg_ly = 0;
	}

	if (tick_counter == 456)
	{
	    tick_counter = 0;
	    reg_ly += 1;

	    if (reg_ly == 1)
	    {
		reg_ly = 0;
		window_line_counter = 0;
		startOamSearch();
		setStatMode(OamSearch);
	    }
	}
    }

    void GBGPU::oamSearch()
    {
	tickOamSearch();
	if (sprite_cycles == 40)
	{
	    startFetcher();
	    setStatMode(PixelTransfer);
	}
    }

    void GBGPU::pixelTransfer()
    {
	tickFetcher();

	if (testbit(reg_lcdc, 1))
	{
	    if (sprite_in_progress)
	    {
		return;
	    }

	    bool is_sprite_added = false;

	    for (size_t index = 0; index < sprites.size(); index++)
	    {
		auto &sprite = sprites.at(index);

		if (sprite.is_rendered)
		{
		    continue;
		}

		if ((pixel_xpos == 0) && (sprite.xpos < 8))
		{
		    if (!is_sprite_added)
		    {
			addSprite((8 - sprite.xpos), index);
			is_sprite_added = true;
		    }

		    sprite.is_rendered = true;
		}
		else if ((sprite.xpos - 8) == pixel_xpos)
		{
		    if (!is_sprite_added)
		    {
			addSprite(0, index);
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

	if (bg_fifo.size() > 8)
	{
	    GBFIFOPixel bg_pixel = bg_fifo.pop();

	    if (dropped_pixels < (reg_scx & 0x7))
	    {
		dropped_pixels += 1;
		return;
	    }

	    if (!is_window && testbit(reg_lcdc, 5) && (reg_ly >= reg_winy) && (pixel_xpos >= (reg_winx - 7)))
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
	    updateFramebuffer();

	    if (is_window)
	    {
		window_line_counter += 1;
	    }

	    setStatMode(HBlank);
	    signalHDMA();
	}
    }

    void GBGPU::addSprite(int xpos, int index)
    {
	current_sprite = sprites.at(index);
	sprite_in_progress = true;
	fetcher_state = FetchSpriteNumber;
	sprite_line = (reg_ly + 16 - current_sprite.ypos);

	if (!testbit(reg_lcdc, 2))
	{
	    sprite_line &= 0x7;
	}

	sprite_offs = xpos;
	sprite_index = index;
    }

    void GBGPU::startOamSearch()
    {
	sprite_cycles = 0;
	sprites.clear();
	sprite_state = ReadingY;
    }

    void GBGPU::tickOamSearch()
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

		int sprite_ly = (reg_ly + 16);

		if ((sprites.size() < 10) && inRange(sprite_ly, sprite_ypos, (sprite_ypos + sprite_height)))
		{
		    GBSprite sprite;
		    sprite.xpos = sprite_xpos;
		    sprite.ypos = sprite_ypos;
		    sprite.addr = sprite_addr;
		    sprite.is_rendered = false;
		    sprite.index = sprite_cycles;
		    sprites.push_back(sprite);
		}

		sprite_cycles += 1;
		sprite_state = ReadingY;
	    }
	    break;
	}
    }

    void GBGPU::startFetcher()
    {
	is_window = false;
	sprite_in_progress = false;
	uint8_t ypos = ((reg_ly + reg_scy) & 0xFF);
	pixel_xpos = 0;
	dropped_pixels = 0;
	map_addr = 0x1800;

	if (testbit(reg_lcdc, 3))
	{
	    map_addr = 0x1C00;
	}

	map_addr += ((ypos >> 3) << 5);
	tile_index = (reg_scx >> 3);
	tile_line = (ypos & 0x7);
	fetcher_state = FetchTileNumber;
	fetcher_counter = 0;
	bg_fifo.clear();
	obj_fifo.clear();
    }

    void GBGPU::startWindowFetcher()
    {
	int xpos = (pixel_xpos - reg_winx + 7);
	uint8_t ypos = (window_line_counter & 0xFF);
	map_addr = 0x1800;

	if (testbit(reg_lcdc, 6))
	{
	    map_addr = 0x1C00;
	}

	map_addr += ((ypos >> 3) << 5);
	tile_index = (xpos >> 3);
	tile_line = (ypos & 0x7);
	fetcher_state = FetchTileNumber;
	fetcher_counter = 0;
	bg_fifo.clear();
    }

    void GBGPU::tickFetcher()
    {
	fetcher_counter += 1;

	if (fetcher_counter == 2)
	{
	    fetcher_counter = 0;

	    switch (fetcher_state)
	    {
		case FetchTileNumber:
		{
		    tile_num = vram.at(map_addr + tile_index);

		    if (is_cgb_mode)
		    {
			tile_attribs = vram.at(0x2000 + map_addr + tile_index);
		    }

		    fetcher_state = FetchTileDataLow;
		}
		break;
		case FetchTileDataLow:
		{
		    tile_addr = (is_cgb_mode && testbit(tile_attribs, 3)) ? 0x2000 : 0;

		    if (testbit(reg_lcdc, 4))
		    {
			tile_addr += (tile_num << 4);
		    }
		    else
		    {
			tile_addr += ((int8_t(tile_num) << 4) + 0x1000);
		    }

		    int tile_yline = tile_line;

		    bool is_yflip = testbit(tile_attribs, 6);

		    if (is_yflip)
		    {
			tile_yline = (7 - tile_yline);
		    }

		    tile_addr += (tile_yline << 1);

		    uint8_t tile_data = vram.at(tile_addr);

		    for (int bit = 0; bit < 8; bit++)
		    {
			bg_data.at(bit) = testbit(tile_data, bit);
		    }

		    fetcher_state = FetchTileDataHigh;
		}
		break;
		case FetchTileDataHigh:
		{
		    uint8_t tile_data = vram.at(tile_addr + 1);

		    for (int bit = 0; bit < 8; bit++)
		    {
			bg_data.at(bit) |= (testbit(tile_data, bit) << 1);
		    }

		    fetcher_state = PushToFIFO;
		}
		break;
		case PushToFIFO:
		{
		    int bg_pal = (tile_attribs & 0x7);
		    bool bg_prior = testbit(tile_attribs, 7);
		    bool is_xflip = testbit(tile_attribs, 5);


		    if (bg_fifo.size() <= 8)
		    {
			for (int i = 0; i < 8; i++)
			{
			    int index = (7 - i);

			    if (is_xflip)
			    {
				index = i;
			    }

			    GBFIFOPixel pixel;
			    pixel.color = bg_data.at(index);
			    pixel.palette = bg_pal;
			    pixel.priority = bg_prior;
			    bg_fifo.push(pixel);
			}

			tile_index = ((tile_index + 1) & 0x1F);

			fetcher_state = FetchTileNumber;
		    }
		}
		break;
		case FetchSpriteNumber:
		{
		    sprite_num = oam.at(current_sprite.addr + 2);
		    sprite_attribs = oam.at(current_sprite.addr + 3);
		    fetcher_state = FetchSpriteDataLow;
		}
		break;
		case FetchSpriteDataLow:
		{
		    int sprite_yline = sprite_line;

		    int sprite_mask = testbit(reg_lcdc, 2) ? 15 : 7;

		    bool is_yflip = testbit(sprite_attribs, 6);

		    if (is_yflip)
		    {
			sprite_yline = (sprite_mask - sprite_yline);
		    }

		    if (testbit(reg_lcdc, 2))
		    {
			sprite_num &= 0xFE;
		    }

		    sprite_addr = (is_cgb_mode && testbit(sprite_attribs, 3)) ? 0x2000 : 0;

		    sprite_addr += (sprite_num << 4);
		    sprite_addr += (sprite_yline << 1);

		    uint8_t sprite_data = vram.at(sprite_addr);

		    for (int bit = 0; bit < 8; bit++)
		    {
			obj_data.at(bit) = testbit(sprite_data, bit);
		    }

		    fetcher_state = FetchSpriteDataHigh;
		}
		break;
		case FetchSpriteDataHigh:
		{
		    uint8_t sprite_data = vram.at(sprite_addr + 1);

		    for (int bit = 0; bit < 8; bit++)
		    {
			obj_data.at(bit) |= (testbit(sprite_data, bit) << 1);
		    }

		    fetcher_state = PushToSpriteFIFO;
		}
		break;
		case PushToSpriteFIFO:
		{
		    // TODO: Implement proper sprite FIFO push

		    while (obj_fifo.size() < 8)
		    {
			GBFIFOPixel pixel;
			obj_fifo.push(pixel);
		    }

		    int palette = 0;

		    bool is_xflip = testbit(sprite_attribs, 5);

		    if (is_cgb_mode)
		    {
			palette = (sprite_attribs & 0x7);
		    }
		    else
		    {
			palette = testbit(sprite_attribs, 4) ? 1 : 0;
		    }

		    bool priority = testbit(sprite_attribs, 7);

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
			pixel.palette = palette;
			pixel.sprite_index = (is_cgb_mode) ? current_sprite.index : 0;
			pixel.priority = priority;

			auto &target = obj_fifo.at(index);

			if ((pixel.color != 0) && ((target.color == 0) || (target.sprite_index > pixel.sprite_index)))
			{
			    target = pixel;
			}
		    }

		    sprite_in_progress = false;
		    fetcher_state = FetchTileNumber;
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
    }

    void GBGPU::setPixel(GBFIFOPixel bg_pixel, GBFIFOPixel obj_pixel)
    {
	if (is_cgb_mode)
	{
	    setCGBPixel(bg_pixel, obj_pixel);
	}
	else
	{
	    setDMGPixel(bg_pixel, obj_pixel);
	}
    }

    void GBGPU::setDMGPixel(GBFIFOPixel bg_pixel, GBFIFOPixel obj_pixel)
    {
	int bg_pal = bg_pixel.color;
	int obj_pal = obj_pixel.color;

	uint8_t reg_obp = (obj_pixel.palette == 0) ? reg_obp0 : reg_obp1;

	int bg_color = ((reg_bgp >> (2 * bg_pal)) & 0x3);
	int obj_color = ((reg_obp >> (2 * obj_pal)) & 0x3);

	if (!testbit(reg_lcdc, 0))
	{
	    bg_color = 0;
	}

	int pal_color = bg_color;

	if ((obj_pal != 0) && ((bg_pal == 0) || !obj_pixel.priority))
	{
	    pal_color = obj_color;
	}

	array<uint8_t, 4> pal_colors = {0xFF, 0xAA, 0x55, 0x00};
	int palette_color = pal_colors.at(pal_color);
	line_buffer.at(pixel_xpos) = GBRGB::fromMonochrome(palette_color);
    }

    void GBGPU::setCGBPixel(GBFIFOPixel bg_pixel, GBFIFOPixel obj_pixel)
    {
	int bg_pal = bg_pixel.color;
	int obj_pal = obj_pixel.color;

	uint16_t bg_color = getBGColor(bg_pixel);
	uint16_t obj_color = getOBJColor(obj_pixel);

	bool is_obj_color = false;

	if (obj_pal == 0)
	{
	    is_obj_color = false;
	}
	else if (bg_pal == 0)
	{
	    is_obj_color = true;
	}
	else if (!testbit(reg_lcdc, 0))
	{
	    is_obj_color = true;
	}
	else if (bg_pixel.priority)
	{
	    is_obj_color = false;
	}
	else if (!obj_pixel.priority)
	{
	    is_obj_color = true;
	}
	else
	{
	    is_obj_color = false;
	}

	uint16_t pal_color = bg_color;

	if (is_obj_color)
	{
	    pal_color = obj_color;
	}

	line_buffer.at(pixel_xpos) = GBRGB::from16(pal_color);
    }

    void GBGPU::updateFramebuffer()
    {
	for (int xpos = 0; xpos < 160; xpos++)
	{
	    size_t fb_offs = (xpos + (reg_ly * 160));
	    framebuffer.at(fb_offs) = line_buffer.at(xpos);
	}

	line_buffer.fill(GBRGB());
    }

    uint8_t GBGPU::readVRAM(uint16_t addr)
    {
	addr &= 0x1FFF;
	uint32_t vram_addr = (addr + (vram_bank * 0x2000));
	return vram.at(vram_addr);
    }

    void GBGPU::writeVRAM(uint16_t addr, uint8_t data)
    {
	addr &= 0x1FFF;
	uint32_t vram_addr = (addr + (vram_bank * 0x2000));
	vram.at(vram_addr) = data;
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
	    case 0x47: data = reg_bgp; break;
	    case 0x48: data = reg_obp0; break;
	    case 0x49: data = reg_obp1; break;
	    case 0x4A: data = reg_winy; break;
	    case 0x4B: data = reg_winx; break;
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
		    gpu_state = OamSearch;
		}
		else if (prev_enable && !testbit(reg_lcdc, 7))
		{
		    reg_stat &= ~0x3;
		    tick_counter = 0;
		    reg_ly = 0;
		    pixel_xpos = 0;
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

    uint8_t GBGPU::readCGBIO(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0xFF;

	switch (addr)
	{
	    case 0x4F: data = (0xFE | vram_bank); break;
	    case 0x68: data = ((bg_pal_inc << 7) | bg_pal_addr); break;
	    case 0x69: data = bg_palettes.at(bg_pal_addr); break;
	    case 0x6A: data = ((obj_pal_inc << 7) | obj_pal_addr); break;
	    case 0x6B: data = obj_palettes.at(obj_pal_addr); break;
	    default: throw mbGBUnmappedMemory(addr, true); break;
	}

	return data;
    }

    void GBGPU::writeCGBIO(uint16_t addr, uint8_t data)
    {
	addr &= 0xFF;

	switch (addr)
	{
	    case 0x4F: vram_bank = testbit(data, 0); break;
	    case 0x68:
	    {
		bg_pal_inc = testbit(data, 7);
		bg_pal_addr = (data & 0x3F);
	    }
	    break;
	    case 0x69:
	    {
		bg_palettes.at(bg_pal_addr) = data;

		if (bg_pal_inc)
		{
		    bg_pal_addr = ((bg_pal_addr + 1) & 0x3F);
		}
	    }
	    break;
	    case 0x6A:
	    {
		obj_pal_inc = testbit(data, 7);
		obj_pal_addr = (data & 0x3F);
	    }
	    break;
	    case 0x6B:
	    {
		obj_palettes.at(obj_pal_addr) = data;

		if (obj_pal_inc)
		{
		    obj_pal_addr = ((obj_pal_addr + 1) & 0x3F);
		}
	    }
	    break;
	    default: throw mbGBUnmappedMemory(addr, data, true); break;
	}
    }
};