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
	    virtual bool loadfile(vector<uint8_t> data) = 0; // Used for loading config files (for example, the config memory of the Mobile Adapter GB)
	    virtual string getsavefilename() = 0; // Used for fetching the name of the config file to save
	    virtual vector<uint8_t> getsavefiledata() = 0; // Used for fetching the data to save to the config file
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

	    bool loadfile(vector<uint8_t> data)
	    {
		return true;
	    }

	    string getsavefilename()
	    {
		return "";
	    }

	    vector<uint8_t> getsavefiledata()
	    {
		vector<uint8_t> empty;
		return empty;
	    }
    };

    // Emulates a disconnected Link Cable, but also prints debug logs (for debugging purposes)
    class LIBMBGB_API SerialDebug : public SerialDevice
    {
        public:
            SerialDebug();
            ~SerialDebug();
            
            linkfunc debuglink;

	    string getaddonname()
	    {
		return "SerialDebug";
	    }
            
            void setlinkcallback(linkfunc cb)
            {
                debuglink = cb;
            }

	    void setprintcallback(printfunc cb)
	    {
		return;
	    }
            
            void deviceready(uint8_t byte, bool ismode)
            {
		// Print debugging info before updating link cable status
		cout << "[SerialDebug] Byte: " << hex << (int)(byte) << endl;
		string mode_text = (ismode) ? "Internal" : "External";
		cout << "[SerialDebug] Clock mode: " << mode_text << " clock" << endl;
		cout << endl;
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
            	if (debuglink)
            	{
		    // Return 0xFF to emulate a disconnected link cable
            	    debuglink(0xFF);
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

	    bool loadfile(vector<uint8_t> data)
	    {
		return true;
	    }

	    string getsavefilename()
	    {
		return "";
	    }

	    vector<uint8_t> getsavefiledata()
	    {
		vector<uint8_t> empty;
		return empty;
	    }
    };

    // (Deprecated) netplay protocol client
    // TODO: Get rid of this, because a better implementation's coming soon...
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

	    bool loadfile(vector<uint8_t> data)
	    {
		return true;
	    }

	    string getsavefilename()
	    {
		return "";
	    }

	    vector<uint8_t> getsavefiledata()
	    {
		vector<uint8_t> empty;
		return empty;
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

	    // Previous command sent to the Game Boy Printer
	    uint8_t prev_cmd = 0x00;

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

	    // Variables for decoding compressed data
	    bool is_compressed_run = false;
	    int run_length = 0;

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

	    bool loadfile(vector<uint8_t> data)
	    {
		return true;
	    }

	    string getsavefilename()
	    {
		return "";
	    }

	    vector<uint8_t> getsavefiledata()
	    {
		vector<uint8_t> empty;
		return empty;
	    }
    };
    
    // Emulates the Barcode Boy (manufactured by Namco, used by Battle Space and several other Japan-exclusive titles)
    class LIBMBGB_API BarcodeBoy : public SerialDevice
    {
        public:
            BarcodeBoy();
            ~BarcodeBoy();
            
            linkfunc bcblink;

	    // Byte sent from link cable
	    uint8_t sentbyte = 0x00;

	    // Byte sent to link cable
	    uint8_t linkbyte = 0x00;

	    bool barcode_swiped = false;

	    // Possible values for the current add-on state
	    enum BCBState : int
	    {
		Init = 0,
		Active = 1,
		SendBarcode = 2,
		Finished = 3
	    };

	    BCBState bcbstate = BCBState::Init;

	    string getaddonname()
	    {
		return "Barcode Boy";
	    }
            
            void setlinkcallback(linkfunc cb)
            {
                bcblink = cb;
            }

	    void setprintcallback(printfunc cb)
	    {
		return;
	    }
            
	    // This add-on utilizes both internal and external clock transfers,
	    // so we need to get creative here
            void deviceready(uint8_t byte, bool ismode)
            {
		// Set current byte and current clock mode
		sentbyte = byte;
		isintclk = ismode;

		// Process current byte and update transfer accordingly
            	update();
            }
            
            void update();
	    void processbyte();

	    bool isintclk = false;
	    bool istransfer = false;

	    int bcb_counter = 0;

	    array<uint8_t, 13> testcode = {0x34, 0x39, 0x30, 0x32, 0x37, 0x37, 0x36, 0x38, 0x30, 0x39, 0x33, 0x36, 0x37};
            
            void transfer();
            
            void swipebarcode()
            {
		if (bcbstate == BCBState::Active)
		{
		    barcode_swiped = true;
		    bcbstate = BCBState::SendBarcode;   
		}
            }
            
            bool swipedcard()
            {
                return barcode_swiped;
            }
            
            int serialcycles()
            {
                return 0;
            }

	    bool loadfile(vector<uint8_t> data)
	    {
		return true;
	    }

	    string getsavefilename()
	    {
		return "";
	    }

	    vector<uint8_t> getsavefiledata()
	    {
		vector<uint8_t> empty;
		return empty;
	    }
    };

    // Emulates the Mobile Adapter GB (Nintendo's first serious attempt at online connectivity for the Game Boy Color and Game Boy Advance)
    // Note: Since this is a Game Boy / Game Boy Color emulator, and not a Game Boy Advance emulator,
    // only compatible Game Boy Color titles will be supported

    // Types of Japanese cell phones that were supported (or planned to be supported)
    // by the Mobile Adapter GB (formatted in an enum for simplicity)
    enum MobilePhoneType : int
    {
	PDC = 0, // PDC phones
	cdmaOne = 1, // cdmaOne phones
	PHS = 2, // PHS phones (unreleased)
	DDI = 3, // DDI phones
    };

    // Actual class definition starts here
    class LIBMBGB_API MobileAdapterGB : public SerialDevice
    {
	using htmlarr = array<string, 1>;

        public:
            MobileAdapterGB();
            ~MobileAdapterGB();
            
            linkfunc madaptlink;

	    uint8_t sent_byte = 0x00;
	    uint8_t link_byte = 0x00;

	    uint8_t adapter_cmd = 0x00;
	    uint8_t packet_length = 0x00;

	    vector<uint8_t> packet_data;
	    vector<uint8_t> response_data;

	    bool begun_session = false;

	    bool begun_pop_session = false;
	    bool begun_smtp_session = false;

	    bool line_busy = false;

	    uint16_t calc_checksum = 0;
	    uint16_t compare_checksum = 0;

	    uint8_t adapter_id = 0x88;

	    uint16_t port_num = 0;

	    array<uint8_t, 192> config_memory;

	    enum MobileAdapterState : int
	    {
		MagicBytes = 0,
		PacketHeader = 1,
		PacketData = 2,
		PacketChecksum = 3,
		PacketAcknowledge = 4,
		PacketReceive = 5,
	    };

	    MobileAdapterState madaptstate = MobileAdapterState::MagicBytes;

	    int madapt_counter = 0;

	    int transfer_state = 0;

	    void set_phone_type(MobilePhoneType type)
	    {
		adapter_id = (0x88 | type);
	    }

	    string getaddonname()
	    {
		return "Mobile Adapter GB";
	    }
            
            void setlinkcallback(linkfunc cb)
            {
                madaptlink = cb;
            }

	    void setprintcallback(printfunc cb)
	    {
		return;
	    }
            
            void deviceready(uint8_t byte, bool ismode)
            {
            	if (ismode)
            	{
		    sent_byte = byte;
            	    update();
            	}
            }

	    string http_data = "";
	    int http_data_index = 0;

	    htmlarr serverin = 
	    {
		"/01/CGB-B9AJ/index.html",
	    };

	    htmlarr html_text = 
	    {
		"<title>Hello from Karen Kujo, your FAVORITE waifu!</title>",
	    };
            
            void update();
	    void processbyte();
	    void processcmd();
	    void processhttp();
	    void processpop();
	    void processsmtp();
	    void prepareresponse(uint8_t cmd, vector<uint8_t> data);

	    template<size_t size>
	    void prepareresponse(uint8_t cmd, array<uint8_t, size> data)
	    {
		vector<uint8_t> arr_data;

		for (int i = 0; i < static_cast<int>(size); i++)
		{
		    arr_data.push_back(data[i]);
		}

		prepareresponse(cmd, arr_data);
	    }

	    void prepareresponse(uint8_t failed_cmd, uint8_t error_status)
	    {
		vector<uint8_t> error_data;
		error_data.push_back(failed_cmd);
		error_data.push_back(error_status);

		prepareresponse(0x6E, error_data);
	    }

	    void prepareresponse(uint8_t cmd)
	    {
		vector<uint8_t> empty;
		prepareresponse(cmd, empty);
	    }
            
            void transfer()
            {
            	if (madaptlink)
            	{
		    // Transfer response byte to Game Boy
            	    madaptlink(link_byte);
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

	    bool loadfile(vector<uint8_t> data)
	    {
		if (data.empty())
		{
		    cout << "Unable to load config memory file." << endl;
		    return false;
		}

		if (data.size() != 192)
		{
		    cout << "Invalid size of config memory file, should be 192 bytes" << endl;
		    return false;
		}

		for (int i = 0; i < static_cast<int>(data.size()); i++)
		{
		    config_memory[i] = data[i];
		}

		cout << "Config memory file successfully loaded." << endl;
		return true;
	    }

	    string getsavefilename()
	    {
		return "madapter.mbconf";
	    }

	    vector<uint8_t> getsavefiledata()
	    {
		vector<uint8_t> temp_data;
		
		for (int i = 0; i < 192; i++)
		{
		    temp_data.push_back(config_memory[i]);
		}

		return temp_data;
	    }
    };

    // Emulates the Turbo File GB (developed by ASCII Corporation, first used in RPG Tsukuru GB)
    class LIBMBGB_API TurboFileGB: public SerialDevice
    {
        public:
            TurboFileGB();
            ~TurboFileGB();
            
            linkfunc turbolink;

	    uint8_t sentbyte = 0;
	    uint8_t linkbyte = 0;

	    uint8_t packet_cmd = 0;

	    int turbo_file_counter = 0;

	    enum TurboFileGBState : int
	    {
		SyncSignal1 = 0,
		MagicByte = 1,
		Command = 2,
	    };

	    TurboFileGBState turbfstate = TurboFileGBState::SyncSignal1;

	    string getaddonname()
	    {
		return "TurboFileGB";
	    }
            
            void setlinkcallback(linkfunc cb)
            {
                turbolink = cb;
            }

	    void setprintcallback(printfunc cb)
	    {
		return;
	    }
            
            void deviceready(uint8_t byte, bool ismode)
            {
            	if (!ismode)
            	{
		    sentbyte = byte;
            	    update();
            	}
            }
            
            void update();
	    void processbyte();
            
            void transfer()
            {
            	if (turbolink)
            	{
            	    turbolink(linkbyte);
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

	    bool loadfile(vector<uint8_t> data)
	    {
		return true;
	    }

	    string getsavefilename()
	    {
		return "";
	    }

	    vector<uint8_t> getsavefiledata()
	    {
		vector<uint8_t> empty;
		return empty;
	    }
    };
    
};

#endif // LIBMBGB_ADDONS
