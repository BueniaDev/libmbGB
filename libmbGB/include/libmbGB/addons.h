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

#ifndef LIBMBGB_ADDONS
#define LIBMBGB_ADDONS

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
    using linkfunc = function<void(uint8_t)>;
    using printfunc = function<void(vector<RGB> &)>;

    class LIBMBGB_API GBPrinter
    {
	public:
	    GBPrinter();
	    ~GBPrinter();

	    uint8_t linkbyte = 0;
	    uint8_t printerbyte = 0;

	    int statesteps = 0;

	    linkfunc printerrec;

	    

	    void setprintreccallback(linkfunc cb)
	    {
		printerrec = cb;
	    }

	    void printerready(uint8_t byte, bool ismode)
	    {
		if (ismode)
		{
		    printerbyte = byte;
		    update();
		}
		else
		{
		    if (byte != 0xDD)
		    {
		        cout << "External transfer" << endl;
			exit(1);
		    }
		}
	    }

	    enum State : int
	    {
		MagicBytes = 0,
		Command = 1,
		CompressionFlag = 2,
		DataLength = 3,
		CommandData = 4,
		Checksum = 5,
		AliveIndicator = 6,
		Status = 7,
	    };

	    array<uint8_t, 8192> printerram;
	    int ramfillamount = 0;
	    bool printerrequest = false;
	    bool isaliveindicator = false;

	    printfunc printpixels;

	    inline void setprintcallback(printfunc cb)
	    {
		printpixels = cb;
	    }

	    int clockbits = 0;

	    uint8_t printpalette = 0;

	    uint16_t currentchecksum = 0;
	    uint8_t currentcommand = 0;
	    uint16_t commanddatalength = 0;
	    uint16_t comparechecksum = 0;
	    bool checksumpass = false;

	    State currentstate = State::MagicBytes;

	    void update();
	    void processbyte();
	    void printpicture();

	    vector<RGB> printoutbuffer;

	    void transfer()
	    {
		// cout << hex << (int)(linkbyte) << endl;
		if (printerrec)
		{
		    printerrec(linkbyte);
		}
		else
		{
		    cout << "Bad function call" << endl;
		    exit(1);
		}
	    }
    };

    class LIBMBGB_API LinkCable
    {
	public:
	    LinkCable();
	    ~LinkCable();

	    linkfunc recievelink1;
	    linkfunc recievelink2;

	    void setlinkreccallbacks(linkfunc p1cb, linkfunc p2cb)
	    {
		recievelink1 = p1cb;
		recievelink2 = p2cb;
	    }
			
	    struct LinkData
	    {
		uint8_t byte = 0;
		bool mode = false;
		bool ready = false;
	    };

	    LinkData link1;
	    LinkData link2;
	
	    bool islink2 = false;
			
	    void setlink2(bool val)
	    {
		islink2 = val;
	    }

	    void transfer()
	    {
		if (recievelink1)
		{
		     recievelink1(link2.byte);
		}

		if (recievelink2)
		{
		    recievelink2(link1.byte);
		}

		link1.ready = false;
		link2.ready = false;
	    }
			
	    void link1ready(uint8_t val, bool ismode)
	    {
		link1.byte = val;
		link1.mode = ismode;
		link1.ready = true;
		update();
	    }

	    void link2ready(uint8_t val, bool ismode)
	    {
		link2.byte = val;
		link2.mode = ismode;
		link2.ready = true;
		update();
	    }

	    void update();
    };
};

#endif // LIBMBGB_ADDONS
