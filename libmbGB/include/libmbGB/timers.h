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

#ifndef LIBMBGB_TIMERS_H
#define LIBMBGB_TIMERS_H

#include "utils.h"
using namespace gb;

namespace gb
{
    class LIBMBGB_API GBTimers
    {
	public:
	    GBTimers();
	    ~GBTimers();

	    void init();
	    void shutdown();

	    uint8_t readIO(int addr);
	    void writeIO(int addr, uint8_t data);

	    void doSavestate(mbGBSavestate &file);

	    void tickTimers();

	    void setIRQCallback(voidintfunc cb)
	    {
		irq_func = cb;
	    }

	private:
	    uint16_t divider = 0;
	    uint8_t reg_tima = 0;
	    uint8_t reg_tma = 0;
	    uint8_t reg_tac = 0;

	    int timer_irq_cycles = 0;
 	    int timer_reload_cycles = 0;

	    voidintfunc irq_func;

	    void checkTAC(uint8_t new_tac);

	    void fireInterrupt()
	    {
		if (irq_func)
		{
		    irq_func(2);
		}
	    }

	    void tickTIMA(uint16_t prev_div, uint16_t new_div);

	    array<int, 4> shift_amounts = {9, 3, 5, 7};
	    bool is_timer_reloading = false;
    };
};

#endif // LIBMBGB_TIMERS_H