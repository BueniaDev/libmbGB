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

#include "../../include/libmbGB/addons.h"
using namespace gb;

namespace gb
{
    // Definitions of SerialDevice constructor/deconstructor
    SerialDevice::SerialDevice()
    {
    
    }
    
    SerialDevice::~SerialDevice()
    {
    
    }
    
    // Constructor/deconstructor definitions of Disconnected "serial device"
    Disconnected::Disconnected()
    {
    
    }
    
    Disconnected::~Disconnected()
    {
    
    }

    // Constructor/deconstructor definitions of KujoGBClient "serial device"
    // TODO: Get rid of this, because a better implementation's coming soon...
    KujoGBClient::KujoGBClient()
    {
    
    }
    
    KujoGBClient::~KujoGBClient()
    {
    
    }

    // Function definitions for Game Boy Printer "serial device"
    /* TODO: Emulation is VERY preliminary and basic at this point, and it assumes that the ROM correctly uses
	the printer's interface.

	Incorrect usage and any "undefined behavior" are not correctly emulated, mainly for two key reasons:

	1. Said usage and "undefined behavior" are completely undocumented, and:
	2. I currently don't have a Game Boy Printer of my own to figure out that behavior myself.

	This emulation also does not currently emulate communication timeout, which means that a bug
	might prevent proper printer operation until the emulated GameBoy is restarted.

	Most of the printers released into the market come with a Toshiba TMP87CM40AF 8-bit MCU that contains
	32 KB of internal ROM and 1024 bytes of internal ram extended by an additional 8192 byte ram chip.

	In terms of any incorrect usage and "undefined behavior" listed below, only a dissassembly of the MCU's
	32KB ROM can confirm all this. However, to the best of all our collective knowledge, that has never been
	done or even attempted before.

	If you have any information about the internal ROM that the Game Boy Printer's MCU uses,
	drop me a line at buenia.mbemu@gmail.com.

    */
    GBPrinter::GBPrinter()
    {

    }

    GBPrinter::~GBPrinter()
    {

    }

    // Determine necessary byte
    void GBPrinter::update()
    {
	processbyte();
	transfer();
    }

    // Proccess byte sent over Link Cable
    void GBPrinter::processbyte()
    {
	switch (state)
	{
	    // "Magic bytes"
	    case PrinterState::MagicBytes:
	    {
		// TODO: What happens if other bytes are sent instead of the "magic bytes"? (may need to confirm this via hardware tests)
		// 0x88 is the first "magic byte"
		if (!first_magic_byte_rec && sentbyte == 0x88)
		{
		    // Reset printer data index here
		    printer_data_index = 0;

		    // Move on to the next magic byte
		    first_magic_byte_rec = true;

		    // Reply byte is 0x00
		    linkbyte = 0x00;
		}
		else if (first_magic_byte_rec)
		{
		    // 0x33 is the second "magic byte"
		    if (sentbyte == 0x33)
		    {
			// Move on to the next stage
			state = PrinterState::Command;
		    }
		    else
		    {
			// Set "packet error" bit (bit 4) if the second "magic byte" is incorrect
			status_byte = BitSet(status_byte, 4);
		    }

		    // This is to ensure that the magic bytes are properly received on subsequent packet transfers
		    first_magic_byte_rec = false;

		    // Reply byte is 0x00
		    linkbyte = 0x00;
		}
	    }
	    break;
	    // Command
	    case PrinterState::Command:
	    {
		// Check that the command sent to the Game Boy Printer is valid
		// Valid commands are 0x01, 0x02, 0x04, 0x08 and 0x0F

		switch (sentbyte)
		{
		    case 0x01: // INIT
		    case 0x02: // PRINT
		    case 0x04: // DATA
		    case 0x08: // BREAK
		    case 0x0F: // STATUS
		    {
			// Set print command and add command to internal checksum
			print_cmd = sentbyte;
			calc_checksum = sentbyte;
			state = PrinterState::CompressFlag;
		    }
		    break;
		    default:
		    {
			// If the command is invalid, raise a "packet error"
			// TODO: Confirm this behavior via hardware tests
			cout << "Printer error: Unrecognized printer command of " << hex << (int)(sentbyte) << endl;
			status_byte = BitSet(status_byte, 4);
			state = PrinterState::MagicBytes;
		    }
		    break;
		}

		// Reply byte is 0x00
		linkbyte = 0x00;
	    }
	    break;
	    // Compression flag
	    case PrinterState::CompressFlag:
	    {
		// If the current transfered byte is 0x01, compression is enabled
		if (sentbyte == 0x01)
		{
		    // cout << "Compression is enabled" << endl;
		    compress_flag = true;
		}
		// Otherwise, if it's 0x00, compression is disabled
		// TODO: Is this command exclusive to the DATA command (0x04), and do other values affect the internal compression flag? (both may need to be confirmed via hardware tests)
		else if (sentbyte == 0x00)
		{
		    compress_flag = false;
		}

		// Add transfered byte to internal checksum
		calc_checksum += sentbyte;

		// Move on to next stage
		state = PrinterState::DataLength;

		// Reply byte is 0x00
		linkbyte = 0x00;
	    }
	    break;
	    // Data length
	    case PrinterState::DataLength:
	    {
		// Add transfered byte to internal checksum
		calc_checksum += sentbyte;

		if (!lsb_rec)
		{
		    // LSB of data length
		    data_length = sentbyte;
		    lsb_rec = true;

		    // Reply byte is 0x00
		    linkbyte = 0x00;
		}
		else
		{
		    // MSB of data length
		    data_length |= (sentbyte << 8);

		    // Reset lsb_rec variable to false
		    lsb_rec = false;
		    
		    // Each segment of an image printed by the Game Boy Printer is 640 bytes (160x144 image in 2BPP).
		    //
		    // Here's an equation to check that the math behind the above number is correct:
		    // 
		    // buffer_len = Length of buffer (in bytes)
		    // width = Width of final image (in pixels)
		    // height = Height of final image (in pixels)
		    // bits_per_pixel = Number of bits per pixel in each pixel of undecoded image
		    //
		    // Equation is:
		    // buffer_len = (width * height) / (8 / bits_per_pixel)

		    // A given buffer data length is valid if it's between 0 and 640
		    if ((data_length >= 0) && (data_length <= 640))
		    {
			// Valid data lengths allow the printer to continue normally
			printer_length = data_length;
			// cout << "Valid data length of " << dec << (int)(data_length) << endl;

			// If a "PRINT" command (or a "DATA" command with a non-zero data length) is sent,
			// start fetching printer data
			// TODO: What happens if a "PRINT" command is sent with a data length of less than 4?
			// (may need to confirm this via hardware tests)
			if ((print_cmd == 0x02) || ((print_cmd == 0x04) && (data_length > 0)))
			{
			    state = PrinterState::PacketData;
			}
			// Otherwise, begin calculation of checksums
			else
			{
			    state = PrinterState::Checksum;
			}
		    }
		    // Invalid data lengths raise a packet error
		    // TODO: Confirm this behavior via hardware tests
		    else
		    {
			cout << "Printer error: Invalid data length of " << dec << (int)(data_length) << ", should be between 0 and 640" << endl;
			status_byte = BitSet(status_byte, 4);
			state = PrinterState::MagicBytes;
		    }

		    // Reply byte is 0x00
		    linkbyte = 0x00;
		}
	    }
	    break;
	    case PrinterState::PacketData:
	    {
		// Add transfered byte to internal checksum
		calc_checksum += sentbyte;

		// Push transfered byte into printer's data array
		printer_data.push_back(sentbyte);

		// Decrement the current data length during each iteration
		printer_length -= 1;

		// If the current data length is 0, then all the appropriate packet data has been transfered,
		// so begin calculation of checksums
		if (printer_length == 0)
		{
		    state = PrinterState::Checksum;
		}

		// Reply byte is 0x00
		linkbyte = 0x00;
	    }
	    break;
	    case PrinterState::Checksum:
	    {
		if (!lsb_rec)
		{
		    // LSB of checksum
		    compare_checksum = sentbyte;
		    lsb_rec = true;

		    // Reply byte is 0x00
		    linkbyte = 0x00;
		}
		else
		{
		    // MSB of checksum
		    compare_checksum |= (sentbyte << 8);
		    lsb_rec = false;

		    // If the internal checksum and the checksum sent to the printer
		    // don't match, raise a "checksum error" (i.e. set bit 0 of the status byte)

		    if (calc_checksum != compare_checksum)
		    {
			cout << "Printer error: Checksums don't match!" << endl;
			status_byte = BitSet(status_byte, 0);
		    }
		    else
		    {
			status_byte = BitReset(status_byte, 0);
		    }

		    state = PrinterState::AliveIndicator;

		    // Reply byte is 0x00
		    linkbyte = 0x00;
		}
	    }
	    break;
	    case PrinterState::AliveIndicator:
	    {
		// Reply byte here is 0x81
		linkbyte = 0x81;
		state = PrinterState::Status;
	    }
	    break;
	    case PrinterState::Status:
	    {
		// cout << "Current status byte is " << hex << (int)(status_byte) << endl;
		// Reply byte here is the status byte
		linkbyte = status_byte;

		// Process printer command
		processcmd();

		// Switch back to the "magic bytes" stage for the next transfer
		state = PrinterState::MagicBytes;		
	    }
	    break;
	}
    }

    // Process printer command
    void GBPrinter::processcmd()
    {
	switch (print_cmd)
	{
	    // INIT
	    case 0x01:
	    {
		// cout << "Initializing printer..." << endl;
		printer_data.clear();
		image_data.clear();
		printer_data_index = 0;
		status_byte = 0;
	    }
	    break;
	    // PRINT
	    case 0x02:
	    {
		// cout << "Printing..." << endl;
		// This (hopefully) shouldn't happen with commercial titles
		if (printer_data.size() != 4)
		{
		    cout << "Printer error: Invalid PRINT command length of " << dec << (int)(printer_data.size()) << ", should be 4" << endl;
		    printer_data.clear();
		    image_data.clear();
		    status_byte = BitSet(status_byte, 4);
		    state = PrinterState::MagicBytes;
		    return;
		}

		// TODO: Figure out how byte 0 (number of sheets to print) actually affects the printer
		// (hardware tests likely needed)

		// Update status byte
		status_byte = BitReset(status_byte, 3);
		status_byte = BitSet(status_byte, 2);
		status_byte = BitSet(status_byte, 1);

		// Set up the margins
		lastmargins = printermargins;
		printermargins = printer_data[1];

		// Set up the printer palette
		printerpalette = printer_data[2];

		// Set the printer exposure
		// TODO: Implement this (somehow)
		printerexposure = printer_data[3];

		// Print picture
		printpicture();

		// Clear printer data buffer
		printer_data.clear();
	    }
	    break;
	    // DATA
	    case 0x04:
	    {
		// It is expected that we always receive complete bands (aka. 20 tiles' worth) of image data.
		int data_len = ((int)printer_data.size() % 40);

		if (data_len == 0)
		{
		    // Update status byte
		    status_byte = BitSet(status_byte, 3);

		    // Append printer data to vector containing image data

		    int prev_size = ((int)image_data.size());

		    image_data.resize((image_data.size() + printer_data.size()), 0);

		    for (int i = 0; i < (int)printer_data.size(); i++)
		    {
			image_data[(prev_size + i)] = printer_data[i];
		    }

		    printer_data.clear();
		}
		else
		{
		    cout << "Printer error: Misaligned image data" << endl;
		    printer_data.clear();
		    image_data.clear();
		    status_byte = BitSet(status_byte, 4);
		    state = PrinterState::MagicBytes;
		}
	    }
	    break;
	    // BREAK
	    case 0x08:
	    {
		cout << "Halting printer..." << endl;
		printer_data.clear();
		image_data.clear();
		printer_data_index = 0;
		status_byte = 0;
	    }
	    break;
	    // STATUS
	    case 0x0F:
	    {
		cout << "Fetching status byte..." << endl;
		printer_data.clear();

		// Printing is done instantly, but we leave the "busy" bit set for one iteration,
		// just for the sake of compatibiltiy

		if (TestBit(status_byte, 2) && TestBit(status_byte, 1) && !TestBit(status_byte, 4))
		{
		    status_byte = BitReset(status_byte, 1);
		}
	    }
	    break;
	}
    }

    void GBPrinter::printpicture()
    {
	// If the boolean of "appending" is true, the current image will be appended onto the previous one.
	// Some games (i.e. Pokemon Yellow) have a tendency to print an image in multiple runs.
	bool appending = false;

	if ((lastmargins != -1) && ((lastmargins & 0xF) == 0) && ((printermargins >> 4) == 0))
	{
	    appending = true;
	}

	int canvasheight = (int)(ceil(ceil(image_data.size() / 16.0) / 20.0) * 8);

	gbRGB white;
	white.red = white.green = white.blue = 0xFF;

	vector<gbRGB> printoutbuffer((160 * canvasheight), white);

	for (int i = 0; i < (int)(printoutbuffer.size()); i++)
	{
	    uint8_t tempbyte = 0;
	    int bufferx = (i % 160);
	    int buffery = (i / 160);

	    int tilenum = (bufferx / 8) + ((buffery / 8) * 20);

	    int tilestartloc = (tilenum << 4);

	    int tilex = (bufferx % 8);
	    int tiley = (buffery % 8);

	    uint8_t byte0 = image_data.at((tilestartloc + (tiley * 2)) % image_data.size());
	    uint8_t byte1 = image_data.at((tilestartloc + (tiley * 2) + 1) % image_data.size());
	    tempbyte = ((byte0 >> (7 - tilex)) & 0x1) | (((byte1 >> (7 - tilex)) & 0x1) << 1);
	    uint8_t palette = ((printerpalette >> ((3 - tempbyte)) * 2) & 0x3);

	    uint8_t color = 0;

	    switch (palette)
	    {
		case 0: color = 0; break;
		case 1: color = 0x40; break;
		case 2: color = 0x80; break;
		case 3: color = 0xFF; break;
	    }

	    gbRGB rgbcolor;
	    rgbcolor.red = rgbcolor.green = rgbcolor.blue = color;

	    printoutbuffer[i] = rgbcolor;
	}

	printpixels(printoutbuffer, appending);
    }

    BarcodeBoy::BarcodeBoy()
    {
    
    }
    
    BarcodeBoy::~BarcodeBoy()
    {
    
    }
    
    void BarcodeBoy::update()
    {
    	processbyte();
    	transfer();
    }
    
    void BarcodeBoy::processbyte()
    {
        if ((state != BCBState::Inactive) && ((recbyte == 0x10) || (recbyte == 0x07)))
        {
            if (state == BCBState::Active)
            {
                linkbyte = 0xFF;
                return;
            }
            else if (state == BCBState::Finished)
            {
                state = BCBState::Inactive;
                bcbcounter = 0;
            }
        }
    
    	switch (state)
    	{
    	    case BCBState::Inactive:
    	    {
    	    	if (bcbcounter < 2)
    	    	{
    	    	    linkbyte = 0xFF;
    	    	    bcbcounter += 1;
    	    	}
    	    	else if (bcbcounter < 4)
    	    	{
    	    	    if ((recbyte == 0x10) && (bcbcounter == 2))
    	    	    {
    	    	        linkbyte = 0x10;
    	    	        bcbcounter += 1;
    	    	    }
    	    	    else if ((recbyte == 0x7) && (bcbcounter == 3))
    	    	    {
    	    	        linkbyte = 0x07;
    	    	        bcbcounter += 1;
    	    	    }
    	    	}
    	    	
    	    	if (bcbcounter == 4)
    	    	{
    	    	    bcbcounter = 0;
    	    	    state = BCBState::Active;
    	    	}
    	    }
    	    break;
    	    case BCBState::SendBarcode:
    	    {
    	    	if (bcbcounter == 0)
    	    	{
    	    	    linkbyte = 0x2;
    	    	    bcbcounter += 1;
    	    	}
    	    	else if (bcbcounter < 14)
    	    	{
    	    	    linkbyte = testcode[(bcbcounter - 1)];
    	    	    bcbcounter += 1;
    	    	}
    	    	else if (bcbcounter == 14)
    	    	{
    	    	    linkbyte = 0x3;
    	    	    bcbcounter += 1;
    	    	}
    	    	else if (bcbcounter == 15)
    	    	{
    	    	    linkbyte = 0x2;
    	    	    bcbcounter += 1;
    	    	}
    	    	else if (bcbcounter < 29)
    	    	{
    	    	    linkbyte = testcode[(bcbcounter - 16)];
    	    	    bcbcounter += 1;
    	    	}
    	    	else if (bcbcounter == 29)
    	    	{
    	    	    linkbyte = 0x3;
    	    	    bcbcounter = 0;
    	    	    barcodeswiped = false;
    	    	    state = BCBState::Finished;
    	    	}
    	    }
    	    break;
    	}
    }
}
