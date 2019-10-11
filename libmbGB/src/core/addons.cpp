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

#include "../../include/libmbGB/serial.h"
using namespace gb;

namespace gb
{

    GBPrinter::GBPrinter()
    {

    }

    GBPrinter::~GBPrinter()
    {

    }

    int GBPrinter::devicebit(int bit)
    {
	int outgoingbit = 0;
	outgoingbit = TestBit(outgoingbyte, 7);
	outgoingbyte <<= 1;
	incomingbyte <<= 1;
	incomingbyte |= bit;

	clockbits += 1;

	if (clockbits == 8)
	{
	    clockbits = 0;
	    processbyte();
	}

	return outgoingbit;
    }

    void GBPrinter::processbyte()
    {
	switch (currentstate)
	{
	    case State::MagicByte:
	    {
		switch (incomingbyte)
		{
		    case 0x88:
		    {
			statesteps = 1;
			outgoingbyte = 0;
		    }
		    break;
			case 0x33:
			{
				if (statesteps == 1)
				{
					statesteps = 0;
					currentstate = State::Command;
					outgoingbyte = 0;
				}
			}
			break;
			default: cout << "Unrecognized byte of " << hex << (int)(incomingbyte) << endl; exit(1); break;
		}
	    }
	    break;
		case State::Command:
		{
			currentchecksum += incomingbyte;
			currentcommand = incomingbyte;
			outgoingbyte = 0;
			currentstate = State::Compression;
		}
		break;
		case State::Compression:
		{
			currentchecksum += incomingbyte;
			outgoingbyte = 0;
			currentstate = State::PacketLength;
		}
		break;
		case State::PacketLength:
		{
			currentchecksum += incomingbyte;
			
			if (statesteps == 0)
			{
				commandpacketlength = incomingbyte;
				statesteps = 1;
				outgoingbyte = 0;
			}
			else
			{
				commandpacketlength |= (incomingbyte << 8);
				
				if (commandpacketlength > 0)
				{
					currentstate = State::PacketData;
				}
				else
				{
					currentstate = State::Checksum;
				}
				
				statesteps = 0;
				outgoingbyte = 0;
				
				if (currentcommand == 1)
				{
					initcommand();
					ramfillamount = 0;
					printerrequest = false;
				}
			}
		}
		break;
		case State::PacketData:
		{
			currentchecksum += incomingbyte;
			
			if (currentcommand == 2)
			{
				if (statesteps == 2)
				{
					printpalette = incomingbyte;
				}
			}
			else if (currentcommand == 4)
			{
				printerram.at(ramfillamount) = incomingbyte;
				ramfillamount = ((ramfillamount + 1) & 0x1FFF);
			}
			
			statesteps++;
			
			if (statesteps >= commandpacketlength)
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
			
			outgoingbyte = 0;
		}
		break;
		case State::Checksum:
		{
			if (statesteps == 0)
			{
				comparechecksum = incomingbyte;
				statesteps = 1;
				outgoingbyte = 0;
			}
			else if (statesteps == 1)
			{
				comparechecksum |= (incomingbyte << 8);
				checksumpass = (comparechecksum == currentchecksum);
				currentchecksum = 0;
				currentstate = State::AliveIndicator;
				statesteps = 0;
				outgoingbyte = 0x81;
			}
		}
		break;
		case State::AliveIndicator:
		{
			if (currentcommand != 1)
			{
				uint8_t tempbyte = 0;
				tempbyte |= (!checksumpass) ? 1 : 0;
				tempbyte |= ((printerrequest ? 1 : 0) << 2);
				tempbyte |= (((ramfillamount >= 0x280) ? 1 : 0) << 3);
				outgoingbyte = tempbyte;
			}
			else
			{
				outgoingbyte = 0;
			}
			
			currentstate = State::Status;
		}
		break;
		case State::Status:
		{
			currentstate = State::MagicByte;
			outgoingbyte = 0;
		}
		break;
	}
    }
	
	void GBPrinter::initcommand()
	{
		for (int i = 0; i < 8192; i++)
		{
			printerram.at(i) = 0;
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
			uint8_t palette = (printpalette >> ((3 - tempbyte)) * 2) & 0x3;
			
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

    Disconnected::Disconnected()
    {

    }

    Disconnected::~Disconnected()
    {

    }

    int Disconnected::devicebit(int bit)
    {
	return 1;
    }
}
