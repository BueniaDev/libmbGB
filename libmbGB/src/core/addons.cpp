// This file is part of libmbGB.
// Copyright (C) 2019 Buenia.
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

#include "../../include/libmbGB/addons.h"
using namespace gb;

namespace gb
{

    GBPrinter::GBPrinter()
    {

    }

    GBPrinter::~GBPrinter()
    {

    }

    void GBPrinter::update()
    {
	processbyte();
	transfer();
    }

    void GBPrinter::processbyte()
    {
	switch (currentstate)
	{
	    case State::MagicBytes:
	    {
		switch (printerbyte)
		{
		case 0x88:
		{
		    statesteps = 1;
		    linkbyte = 0;
		}
		break;
		case 0x33:
		{
		    if (statesteps == 1)
		    {
			statesteps = 0;
			currentstate = State::Command;
			linkbyte = 0;
		    }
		}
		break;
		}
	    }
	    break;
	    case State::Command:
	    {
		currentchecksum += printerbyte;
		currentcommand = printerbyte;

		linkbyte = 0;

		currentstate = State::CompressionFlag;
	    }
	    break;
	    case State::CompressionFlag:
	    {
		currentchecksum += printerbyte;
		linkbyte = 0;
		currentstate = State::DataLength;
	    }
	    break;
	    case State::DataLength:
	    {
		currentchecksum += printerbyte;

		if (statesteps == 0)
		{
		    commanddatalength = printerbyte;
		    statesteps = 1;
		    linkbyte = 0;
		}
		else
		{
		    commanddatalength |= (printerbyte << 8);

		    if (commanddatalength > 0)
		    {
			currentstate = State::CommandData;
		    }
		    else
		    {
			currentstate = State::Checksum;
		    }

		    statesteps = 0;
		    linkbyte = 0;

		    if (currentcommand == 1)
		    {
			printerram.fill(0);
			ramfillamount = 0;
			printerrequest = false;
		    }
		}
	    }
	    break;
	    case State::CommandData:
	    {
		currentchecksum += printerbyte;

		if (currentcommand == 2)
		{
		    if (statesteps == 2)
		    {
			printpalette = printerbyte;
		    }
		}
		else if (currentcommand == 4)
		{
		    printerram.at(ramfillamount) = printerbyte;
		    ramfillamount = ((ramfillamount + 1) & 0x1FFF);
		}

		statesteps += 1;

		if (statesteps == commanddatalength)
		{
		    currentstate = State::Checksum;
		    statesteps = 0;

		    if (currentcommand == 2)
		    {
			printerrequest = true;
			printpicture();
			ramfillamount = 0;
		    }
		}

		linkbyte = 0;
	    }
	    break;
	    case State::Checksum:
	    {
		if (statesteps == 0)
		{
		    comparechecksum = printerbyte;
		    statesteps = 1;
		    linkbyte = 0;
		}
		else if (statesteps == 1)
		{
		    comparechecksum |= (printerbyte << 8);
		    checksumpass = (comparechecksum == currentchecksum);
		    currentchecksum = 0;
		    currentstate = State::AliveIndicator;
		    statesteps = 0;
		    linkbyte = 0x81;
		}
	    }
	    break;	
	    case State::AliveIndicator:
	    {
		if (printerbyte == 0)
		{
		    currentstate = State::Status;
		}
		else
		{
		    currentstate = State::MagicBytes;
		}

		linkbyte = 0x81;
	    }
	    break;
	    case State::Status:
	    {
		if (currentcommand != 1)
		{
		    uint8_t tempbyte = 0x0;
		    tempbyte |= (!checksumpass) ? 1 : 0;
		    tempbyte |= ((printerrequest ? 1 : 0) << 2);
		    tempbyte |= (((ramfillamount >= 0x280) ? 1 : 0) << 3);
		    linkbyte = tempbyte;
		}
		else
		{
		    linkbyte = 0;
		}

		currentstate = State::MagicBytes;
	    }
	    break;
	}
    }

    void GBPrinter::printpicture()
    {
	int canvasheight = (int)(ceil(ceil(ramfillamount / 16.0) / 20.0) * 8);

	RGB white;
	white.red = 0xFF;
	white.green = 0xFF;
	white.blue = 0xFF;

	printoutbuffer.resize((160 * canvasheight), white);

	for (int i = 0; i < (int)(printoutbuffer.size()); i++)
	{
	    uint8_t tempbyte = 0;
	    int bufferx = (i % 160);
	    int buffery = (i / 160);
	    int tilenum = (bufferx / 8) + ((buffery / 8) * 20);
	    int tilestartloc = (tilenum * 16);
	    int tilex = (bufferx % 8);
	    int tiley = (buffery % 8);

	    uint8_t byte0 = printerram.at((tilestartloc + (tiley * 2)) & 0x1FFF);
	    uint8_t byte1 = printerram.at((tilestartloc + (tiley * 2) + 1) & 0x1FFF);
	    tempbyte = ((byte0 >> (7 - tilex)) & 0x1) | (((byte1 >> (7 - tilex)) & 0x1) << 1);
	    uint8_t palette = ((printpalette >> ((3 - tempbyte)) * 2) & 0x3);

	    int color = 0;

	    switch (palette)
	    {
		case 0: color = 0; break;
		case 1: color = 0x40; break;
		case 2: color = 0x80; break;
		case 3: color = 0xFF; break;
	    }

	    RGB temp;
	    temp.red = color;
	    temp.green = color;
	    temp.blue = color;

	    printoutbuffer.at(i) = temp;
	}

	printpixels(printoutbuffer);
    }

    LinkCable::LinkCable()
    {
		
    }
	
    LinkCable::~LinkCable()
    {
		
    }

    void LinkCable::update()
    {
	if (link1.ready && link2.ready)
	{
	    if (link1.mode != link2.mode)
	    {
		transfer();
	    }
	}
	else
	{
	    if (link1.ready)
	    {
		if (link1.mode)
		{
		    link2.byte = 0xFF;
		    transfer();
		}
	    }

	    if (link2.ready)
	    {
		if (link2.mode)
		{
		    link1.byte = 0xFF;
		    transfer();
		}
	    }
	}
    }
}
