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

#include "timers.h"
using namespace gb;
using namespace std;

namespace gb
{
    GBTimers::GBTimers()
    {

    }

    GBTimers::~GBTimers()
    {

    }

    void GBTimers::init()
    {
	divider = 8;
	timer_irq_cycles = 0;
	timer_reload_cycles = 0;
	is_timer_reloading = false;
    }

    void GBTimers::shutdown()
    {
	return;
    }

    uint8_t GBTimers::readIO(int addr)
    {
	uint8_t data = 0;
	switch (addr)
	{
	    case 0x4: data = (divider >> 8); break;
	    case 0x5: data = reg_tima; break;
	    case 0x6: data = reg_tma; break;
	    case 0x7: data = (reg_tac | 0xF8); break;
	    default: data = 0; break;
	}

	return data;
    }

    void GBTimers::writeIO(int addr, uint8_t data)
    {
	switch (addr)
	{
	    case 0x4:
	    {
		tickTIMA(divider, 0);
		divider = 0;
	    }
	    break;
	    case 0x5:
	    {
		if (timer_irq_cycles == 0)
		{
		    reg_tima = data;
		    is_timer_reloading = false;
		    timer_reload_cycles = 0;
		}
	    }
	    break;
	    case 0x6:
	    {
		reg_tma = data;

		if (timer_irq_cycles != 0)
		{
		    reg_tima = reg_tma;
		}
	    }
	    break;
	    case 0x7:
	    {
		if (testbit(reg_tac, 2) && !testbit(data, 2))
		{
		    tickTIMA(divider, 0);

		    if (is_timer_reloading && (timer_reload_cycles == 0))
		    {
			timer_reload_cycles = 0;
			is_timer_reloading = false;
			reg_tima = reg_tma;
			fireInterrupt();
		    }
		}

		checkTAC(data);
		reg_tac = (data & 0x7);
	    }
	    break;
	    default: break;
	}
    }

    void GBTimers::tickTimers()
    {
	if (timer_irq_cycles > 0)
	{
	    timer_irq_cycles += 1;

	    if (timer_irq_cycles == 4)
	    {
		timer_irq_cycles = 0;
	    }
	}

	if (is_timer_reloading)
	{
	    timer_reload_cycles += 1;

	    if (timer_reload_cycles == 4)
	    {
		timer_reload_cycles = 0;
		timer_irq_cycles = 0;
		is_timer_reloading = false;
		reg_tima = reg_tma;
		fireInterrupt();
	    }
	}

	uint16_t prev_div = divider;
	divider += 1;
	tickTIMA(prev_div, divider);
    }

    void GBTimers::checkTAC(uint8_t new_tac)
    {
	if (!testbit(new_tac, 2))
	{
	    return;
	}

	int old_timer_mode = (reg_tac & 0x3);
	int new_timer_mode = (new_tac & 0x3);

	int old_shift = shift_amounts.at(old_timer_mode);
	int new_shift = shift_amounts.at(new_timer_mode);

	if (!testbit(divider, old_shift) && testbit(divider, new_shift))
	{
	    reg_tima += 1;

	    if (reg_tima == 0)
	    {
		timer_reload_cycles = 0;
		is_timer_reloading = true;
	    }
	}
    }

    void GBTimers::tickTIMA(uint16_t prev_div, uint16_t new_div)
    {
	bool is_timer_enabled = testbit(reg_tac, 2);
	int timer_mode = (reg_tac & 0x3);

	if (is_timer_enabled)
	{
	    int shift_amount = shift_amounts.at(timer_mode);

	    if (testbit(prev_div, shift_amount) && !testbit(new_div, shift_amount))
	    {
		reg_tima += 1;

		if (reg_tima == 0)
		{
		    timer_reload_cycles = 0;
		    is_timer_reloading = true;
		}
	    }
	}
    }
};