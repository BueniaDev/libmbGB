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

#ifndef LIBMBGB_SERIAL
#define LIBMBGB_SERIAL

#include "mmu.h"
#include "libmbgb_api.h"
#include "gpu.h" // This is for the custom RGB data type
#include <iostream>
#include <functional>
#include <bitset>
#include <cmath> // This is required for ceil, which is used to calculate the printout's height
using namespace gb;
using namespace std;

namespace gb
{
	
    using printfunc = function<void(vector<RGB> &)>;
    using serialfunc = function<void(uint8_t)>;
	
    class LIBMBGB_API SerialDevice
    {
	public:
	    SerialDevice();
	    ~SerialDevice();

	    virtual int devicebit(int bit) = 0;
    };

    class LIBMBGB_API Disconnected : public SerialDevice
    {
	public:
	    Disconnected();
	    ~Disconnected();

	    int devicebit(int bit);
    };

    class LIBMBGB_API GBPrinter : public SerialDevice
    {
	public:
	    GBPrinter();
	    ~GBPrinter();
	
	    uint8_t incomingbyte = 0;
	    uint8_t outgoingbyte = 0;
	    int clockbits = 0;
		
		array<uint8_t, 8192> printerram = { 0 };
		vector<RGB> printoutbuffer;
		
		printfunc printpixels;

	    int devicebit(int bit);

	    void processbyte();
		void initcommand();
		
		inline void setprintcallback(printfunc cb)
		{
			printpixels = cb;
		}
		
		void printpicture();
		
		uint16_t currentchecksum = 0;
		uint16_t comparechecksum = 0;
		uint8_t currentcommand = 0;
		uint16_t commandpacketlength = 0;
		
		int ramfillamount = 0;

	    enum State : int
	    {
		MagicByte = 0,
		Command = 1,
		Compression = 2,
		PacketLength = 3,
		PacketData = 4,
		Checksum = 5,
		AliveIndicator = 6,
		Status = 7
	    };

	    State currentstate = State::MagicByte;

	    bool firstmagicbyte = false;
		int statesteps = 0;
		bool printerrequest = false;
		uint8_t printpalette = 0;
		bool checksumpass = false;
    };

    class LIBMBGB_API Serial
    {
	public:
	    Serial(MMU& memory);
	    ~Serial();

	    void init();
	    void shutdown();

	    MMU& serialmem;

	    GBPrinter *print = new GBPrinter();
	    Disconnected *disconnect = new Disconnected();

	    SerialDevice* device = disconnect;

	    void setdevice(SerialDevice* connected)
	    {
		device = NULL;
		device = connected;
	    }
		
		inline void setprintcallback(printfunc cb)
		{
			cout << "True" << endl;
			print->setprintcallback(cb);
		}

	    int serialclock = 0;
	    int bitstoshift = 0;
	    bool previnc = false;

	    bool transfersignal = false;
	    bool prevtransfersignal = false;

	    void updateserial();

	    inline void initserialclock(uint8_t initval)
	    {
		serialclock = initval;
	    }

	    inline void shiftserialbit()
	    {
		int serialbit = TestBit(serialmem.sb, 7);
		cout << "Outgoing byte before: " << hex << (int)(serialmem.sb) << endl;
		// cout << "Outgoing byte before: " << bitset<8>(serialmem.sb) << endl;
		serialmem.sb <<= 1;
		serialmem.sb |= device->devicebit(serialbit);
		cout << "Outgoing byte after: " << hex << (int)(serialmem.sb) << endl;
		// cout << "Outgoing byte after: " << bitset<8>(serialmem.sb) << endl;
		cout << endl;

		if (--bitstoshift == 0)
		{
		    cout << "Master interrupt" << endl;
		    serialmem.sc &= 0x7F;
		    serialmem.requestinterrupt(3);
		}
	    }

	    inline int selectclockbit()
	    {
		int temp = 0;

		if (serialmem.isdmgconsole())
		{
		    temp = 7;
		}
		else if (serialmem.isgbcconsole())
		{
		    if (TestBit(serialmem.sc, 1))
		    {
			if (serialmem.isdoublespeed())
			{
			    temp = 13; // 64 KB/s
			}
			else
			{
			    temp = 12; // 32 KB/s
			}
		    }
		    else
		    {
			if (serialmem.isdoublespeed())
			{
			    temp = 8; // 2 KB/s
			}
			else
			{
			    temp = 7; // 1 KB/s
			}
		    }
		}

		return temp;
	    }

	    inline bool usinginternalclock()
	    {
		return (TestBit(serialmem.sc, 0));
	    }
    };
};

#endif // LIBMBGB_SERIAL
