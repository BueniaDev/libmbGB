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
#include <ctime>
using namespace gb;
using namespace std;

namespace gb
{
    using linkfunc = function<void(uint8_t)>;
    using printfunc = function<void(vector<RGB> &)>;
    using htmlarr = array<string, 1>;
	
    class LIBMBGB_API MobileAdapterGB
    {
	public:
	    MobileAdapterGB();
	    ~MobileAdapterGB();
			
	    uint8_t linkbyte = 0;
	    uint8_t adapterbyte = 0;
			
	    uint8_t commandid = 0;
	    uint8_t packetdatalength = 0;
	    uint16_t calculatedchecksum = 0;
	    uint16_t comparechecksum = 0;
	    vector<uint8_t> packetdata;
			
	    linkfunc adaptrec;
			
	    void setadaptreccallback(linkfunc cb)
	    {
		adaptrec = cb;
	    }
			
	    void transfer()
	    {
		if (adaptrec)
		{
		    adaptrec(linkbyte);
		}
	    }
			
	    void mobileadapterready(uint8_t byte, bool ismode)
	    {
		if (ismode)
		{
		    adapterbyte = byte;
		    update();
		}
		else
		{
		    return;
		}
	    }
			
	    enum State : int
	    {
		AwaitingPacket = 0,
		PacketHeader = 1,
		PacketData = 2,
		PacketChecksum = 3,
		AcknowledgingPacket = 4,
		EchoPacket = 5,
	    };
			
	    int statesteps = 0;
	    int packetsize = 0;
			
	    State adapterstate;

	    array<uint8_t, 192> adapterdata;

	    bool linebusy = false;

	    uint32_t getipaddr()
	    {
		uint32_t temp = 0;

		if (packetdatalength >= 6)
		{
		    for (int i = 0; i < 4; i++)
		    {
			// Hack to grab value in big-endian format
			temp |= (packetdata[6 + i] << (8 * (~i & 3)));
		    }
		}
		else
		{
		    cout << "Error - Mobile adapter tried opening a tcp connection without a server address" << endl;
		    temp = 0;
		}

		return temp;
	    }

	    uint16_t getport()
	    {
		uint16_t temp = 0;

		if (packetdatalength >= 6)
		{
		    for (int i = 0; i < 2; i++)
		    {
			// Hack to grab value in big-endian format
			temp |= (packetdata[10 + i] << (8 * (~i & 1)));
		    }
		}
		else
		{
		    cout << "Error - Mobile adapter tried opening a tcp connection without a port" << endl;
		    temp = 0;
		}

		return temp;
	    }

	    uint32_t ipaddr = 0;
	    uint16_t port = 0;
			
	    void update();
	    void processbyte();
	    void processcommand();
	    void processpop();	
	    void processhttp();

	    bool popsessionstarted = false;
	    int poptransferstate = 0;

	    int httptransferstate = 0;

	    string httpdata;

	    bool loadadapterdata()
	    {
		bool success = false;

		fstream file("mobiledata.mbmob", ios::in | ios::binary);

		if (!file.is_open())
		{
		    cout << "Mobile adapter data could not be read." << endl;
		    success = false;
		}
		else
		{
		    file.read((char*)&adapterdata[0], 192);
		    cout << "Mobile adapter data succesfully loaded." << endl;
		    success = true;
		}

		return success;
	    }

	    bool saveadapterdata()
	    {
		bool success = false;

		fstream file("mobiledata.mbmob", ios::out | ios::binary);

		if (!file.is_open())
		{
		    cout << "Mobile adapter data could not be written." << endl;
		    success = false;
		}
		else
		{
		    file.write((char*)&adapterdata[0], 192);
		    cout << "Mobile adapter data succesfully stored." << endl;
		    success = true;
		}

		return success;
	    }

	    void strtodata(uint8_t* data, string input)
	    {
		for (int x = 0; x < (int)(input.size()); x++)
		{
		    char ascii = input[x];
		    *data = ascii;
		    data += 1;
		}
	    }

	    string datatostr(uint8_t* data, uint32_t length)
	    {
		string temp = "";

		for (int x = 0; x < (int)(length); x++)
		{
		    char ascii = *data;
		    temp += ascii;
		    data += 1;
		}

		return temp;
	    }

	    inline bool isxmas()
	    {
		time_t t = time(NULL);
		tm* timeptr = localtime(&t);

		return (timeptr->tm_mon == 11);
	    }

	    array<string, 1> serverin = 
	    {
		"/01/CGB-B9AJ/index.html",
	    };

	    htmlarr htmldata = 
	    {
		"<title>Hello world!</title>",
	    };

	    htmlarr htmlxmas = 
	    {
		"<title>Happy holidays!</title>",
	    };

	    array<htmlarr, 2> htmltext =
	    {
		htmldata,
		htmlxmas,
	    };

	    int dataindex = 0;
    };

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
		    return;
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
