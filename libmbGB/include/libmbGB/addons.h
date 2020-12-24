// This file is part of libmbGB.
// Copyright (C) 2020 Buenia.
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
#include "utils.h"
#include <iostream>
#include <functional>
#include <vector>
#include <queue>
#include <bitset>
#include <cmath> // This is required for ceil, which is used to calculate the printout's height
#include <ctime>
using namespace gb;
using namespace std;

namespace gb
{
    using linkfunc = function<void(uint8_t)>;
    using printfunc = function<void(vector<gbRGB>&, bool)>;
    
    // Interface for addons that connect via the Link Cable
    class LIBMBGB_API SerialDevice
    {
        public:
            SerialDevice();
            ~SerialDevice();
            
	    virtual string getaddonname() = 0; // Fetches the name of the connected addon
            virtual void setlinkcallback(linkfunc cb) = 0; // The callback function for Serial::receive
	    virtual void setprintcallback(printfunc cb) = 0; // The callback function for GBPrinter::printpixels
            virtual void deviceready(uint8_t byte, bool ismode) = 0; // Called by Serial class when it's ready for data transfer
            virtual void update() = 0; // Updates the status of the addon
            virtual void transfer() = 0; // Fires the callback function
            virtual void swipebarcode() = 0; // This function is hooked up to a hotkey in the frontend
            virtual bool swipedcard() = 0; // This is required for proper emulation of the BTB and the Barcode Boy
            virtual int serialcycles() = 0; // Possible returned values: 0=Use normal link cable timings, all other values=use returned value for custom link cable timings (i.e. for DMG-07)
    };
    
    // Emulates a disconnected Link Cable
    class LIBMBGB_API Disconnected : public SerialDevice
    {
        public:
            Disconnected();
            ~Disconnected();
            
            linkfunc dislink;

	    string getaddonname()
	    {
		return "Disconnected";
	    }
            
            void setlinkcallback(linkfunc cb)
            {
                dislink = cb;
            }

	    void setprintcallback(printfunc cb)
	    {
		return;
	    }
            
            void deviceready(uint8_t byte, bool ismode)
            {
            	if (ismode)
            	{
            	    update();
            	}
            }
            
            void update()
            {
            	transfer();
            }
            
            void transfer()
            {
            	if (dislink)
            	{
		    // Return 0xFF to emulate a disconnected link cable
            	    dislink(0xFF);
            	}
            }
            
            void swipebarcode()
            {
            	return;
            }
            
            bool swipedcard()
            {
                return false;
            }
            
            int serialcycles()
            {
                return 0;
            }
    };

    // (WIP) netplay protocol client (tabled for future versions)
    class LIBMBGB_API KujoGBClient : public SerialDevice
    {
        public:
            KujoGBClient();
            ~KujoGBClient();
            
            linkfunc dislink;
	    bool linkiswaiting = false;

	    uint8_t linkbyte = 0xFF;
	    uint8_t sentbyte = 0;

	    bool sentmode = false;
	    bool linkmode = false;

	    string getaddonname()
	    {
		return "KujoGBClient";
	    }
            
            void setlinkcallback(linkfunc cb)
            {
                dislink = cb;
            }
            
	    void setprintcallback(printfunc cb)
	    {
		return;
	    }

            void deviceready(uint8_t byte, bool ismode)
            {
            	sentbyte = byte;
		sentmode = ismode;
		update();
            }
            
            void update()
            {
		if (sentmode)
		{
		    transfer();
		}
            }
            
            void transfer()
            {
            	if (dislink)
            	{
            	    dislink(0xFF);
            	}
            }
            
            void swipebarcode()
            {
            	return;
            }
            
            bool swipedcard()
            {
                return false;
            }
            
            int serialcycles()
            {
                return 0;
            }
    };

    // Emulates the Game Boy Printer (may add seperate addon and/or code for the unreleased GB Printer Color)
    class LIBMBGB_API GBPrinter : public SerialDevice
    {
        public:
            GBPrinter();
            ~GBPrinter();
            
	    // Callback function for serial transfers
            linkfunc printlink;

	    // Callback function for printing pictures
	    printfunc printpic;

	    // Byte sent from link cable
	    uint8_t sentbyte = 0x00;

	    // Byte sent to link cable
	    uint8_t linkbyte = 0x00;

	    // Status byte for Game Boy Printer
	    uint8_t status_byte = 0x00;

	    // Used to determine whether the first "magic byte" has already been received
	    bool first_magic_byte_rec = false;

	    // Current command sent to the Game Boy Printer
	    uint8_t print_cmd = 0x00;

	    // Compression flag for Game Boy Printer
	    bool compress_flag = false;

	    // Used to determine whether the LSB of the checksum or data length has already been received
	    bool lsb_rec = false;

	    // Length of data received by Game Boy Printer
	    uint16_t data_length = 0;

	    // This variable keeps track of how much data should be appended to
	    // the Game Boy Printer's internal buffer
	    uint16_t printer_length = 0;

	    // Vector for data received by Game Boy Printer
	    vector<uint8_t> printer_data;

	    // Vector for Game Boy Printer's image data
	    vector<uint8_t> image_data;
	    
	    // Index counter for above data array
	    uint16_t printer_data_index = 0;

	    // Internal checksum for Game Boy Printer
	    uint16_t calc_checksum = 0;
	    
	    // Checksum received by Game Boy Printer
	    uint16_t compare_checksum = 0;

	    // Variable for previous printer margins
	    uint8_t lastmargins = -1;

	    // Variable for printer margins
	    uint8_t printermargins = -1;

	    // Printer palette
	    uint8_t printerpalette = 0;

	    // Printer exposure
	    uint8_t printerexposure = 0;

	    // Possible values for the current printer state
	    enum PrinterState : int
	    {
		MagicBytes = 0,
		Command = 1,
		CompressFlag = 2,
		DataLength = 3,
		PacketData = 4,
		Checksum = 5,
		AliveIndicator = 6,
		Status = 7
	    };

	    // Printer state variable
	    PrinterState state = PrinterState::MagicBytes;

	    string getaddonname()
	    {
		return "Game Boy Printer";
	    }
            
            void setlinkcallback(linkfunc cb)
            {
                printlink = cb;
            }

	    void setprintcallback(printfunc cb)
	    {
		printpic = cb;
	    }

	    void printpixels(vector<gbRGB> &temp, bool appending)
	    {
		if (printpic)
		{
		    printpic(temp, appending);
		}
	    }
            
            void deviceready(uint8_t byte, bool ismode)
            {
            	if (ismode)
            	{
		    sentbyte = byte;
            	    update();
            	}
            }

	    int datacountdown = 0;
	    int transfercountdown = 0;
	    int busycountdown = 0;
            
            void update();
	    void processbyte();
	    void processcmdchecksum();
	    void processcmd();
	    void sendvarlendata(uint8_t val);
	    void printpicture();
            
            void transfer()
            {
            	if (printlink)
            	{
            	    printlink(linkbyte);
            	}
            }
            
            void swipebarcode()
            {
            	return;
            }
            
            bool swipedcard()
            {
                return false;
            }
            
            int serialcycles()
            {
                return 0;
            }
    };
    
    // Emulates the Barcode Boy (manufactured by Namco, used by Battle Space and several other Japan-exclusive titles)
    // TODO: refactor this to get rid of compiler warnings (should be accomplished in Phase 2 of Project Genie)
    class LIBMBGB_API BarcodeBoy : public SerialDevice
    {
    	public:
    	    BarcodeBoy();
    	    ~BarcodeBoy();
    	    
    	    uint8_t recbyte = 0;
    	    uint8_t linkbyte = 0;
    	    
    	    linkfunc powerlink;
    	    
    	    bool barcodeswiped = false;

	    string getaddonname()
	    {
		return "BarcodeBoy";
	    }
    	    
    	    void setlinkcallback(linkfunc cb)
    	    {
    	    	powerlink = cb;
    	    }

	    void setprintcallback(printfunc cb)
	    {
		return;
	    }
    	    
    	    void swipebarcode()
    	    {
    	    	if (state == BCBState::Active)
    	    	{
    	    	    state = BCBState::SendBarcode;
    	    	    barcodeswiped = true;
    	    	}
    	    }
    	    
    	    void update();
    	    void processbyte();
    	    
    	    enum BCBState : int
    	    {
    	        Inactive = 0,
    	        Active = 1,
    	        SendBarcode = 2,
    	        Finished = 3,
    	    };
    	    
    	    BCBState state = BCBState::Inactive;
    	    
    	    array<uint8_t, 13> testcode = {0x34, 0x39, 0x30, 0x32, 0x37, 0x37, 0x36, 0x38, 0x30, 0x39, 0x33, 0x36, 0x37};
    	    
    	    int bcbcounter = 0;
    	    
    	    void transfer()
    	    {
    	        if (powerlink)
    	        {
    	            powerlink(linkbyte);
    	        }
    	    }
    	    
    	    void deviceready(uint8_t byte, bool ismode)
    	    {
    	        if (ismode)
    	        {
    	            recbyte = byte;
    	            update();
    	        }
    	        else if (barcodeswiped)
    	        {
    	            update();
    	        }
    	    }
    	    
    	    bool swipedcard()
    	    {
    	    	return barcodeswiped;
    	    }
    	    
    	    int serialcycles()
    	    {
    	        return 0;
    	    }
    };
};

#endif // LIBMBGB_ADDONS
