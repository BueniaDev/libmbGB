// This file is part of libmbGB.
// Copyright (C) 2021 Buenia.
//
// libmbGB is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// libmbGB is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.

#include "../../include/libmbGB/infrared.h"
using namespace gb;
using namespace std::placeholders;

namespace gb
{
    Infrared::Infrared(MMU& memory) : mem(memory)
    {
	mem.addmemoryreadhandler(0xFF56, bind(&Infrared::readinfrared, this, _1));
	mem.addmemorywritehandler(0xFF56, bind(&Infrared::writeinfrared, this, _1, _2));
    }

    Infrared::~Infrared()
    {

    }

    void Infrared::init()
    {
	if (mem.iscgbconsole())
	{
	    infrared_reg = 0x3E;
	}
	else if (mem.isagbconsole())
	{
	    infrared_reg = 0x00;
	}
	else
	{
	    infrared_reg = 0xFF;
	}
	cout << "Infrared::Initialized" << endl;
    }

    void Infrared::shutdown()
    {

    }

    uint8_t Infrared::readinfrared(uint16_t addr)
    {
	if (!mem.iscgbconsole())
	{
	    return (mem.isdmgconsole()) ? 0x00 : 0xFF;
	}

	uint8_t temp = 0x00;

	if (addr == 0xFF56)
	{
	    if ((infrared_reg >> 6) != 3)
	    {
		temp = BitSet(infrared_reg, 1);
	    }
	    else
	    {
		if (dev != NULL)
		{
		    temp = BitChange(infrared_reg, 1, !dev->getirstatus());
		}
		else
		{
		    temp = BitSet(infrared_reg, 1);
		}
	    }
	}

	return temp;
    }

    void Infrared::writeinfrared(uint16_t addr, uint8_t val)
    {
	if (!mem.iscgbconsole())
	{
	    return;
	}

	if (addr == 0xFF56)
	{
	    bool old_ir_signal = TestBit(infrared_reg, 1);
	    bool old_ir_stat = TestBit(infrared_reg, 0);

	    uint8_t temp_val = val;
	    temp_val &= 0xC1;
	    temp_val |= (old_ir_signal << 1);
	    temp_val |= 0x3C;
	    infrared_reg = temp_val;

	    ir_signal = TestBit(val, 0);

	    if (ir_signal != old_ir_stat)
	    {
		ir_send = ir_signal;
	    }
	}
    }

    void Infrared::updateinfrared()
    {
	if (dev != NULL)
	{
	    dev->updateir(ir_send);
	}

	return;
    }
}