/*
    This file is part of libmbGB.
    Copyright (C) 2021 BueniaDev.

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

#include "addons.h"
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

    // Constructor/deconstructor definitions of SerialDebug "serial device"
    SerialDebug::SerialDebug()
    {
    
    }
    
    SerialDebug::~SerialDebug()
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
			// Set previous printer command
			prev_cmd = print_cmd;
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

		// Process compressed data
		if (compress_flag)
		{
		    // Handle compressed data
		    if (run_length == 0)
		    {
			// Bit 7 determines whether the run is compressed,
			// while bits 6-0 contain the length of the run
			is_compressed_run = TestBit(sentbyte, 7);
			run_length = ((sentbyte & 0x7F) + 1 + ((is_compressed_run) ? 1 : 0));
		    }
		    else if (is_compressed_run)
		    {
			// Handle compressed run
			while (run_length > 0)
			{
			    // Append current byte to printer data the same number of times as the run length
			    printer_data.push_back(sentbyte);
			    run_length -= 1;

			    // The maximum size of the printer data is 0x280 bytes
			    if (printer_data.size() == 0x280)
			    {
				run_length = 0;
			    }
			}
		    }
		    else
		    {
			// Handle uncompressed run
			printer_data.push_back(sentbyte);
			run_length -= 1;
		    }
		}
		// Process uncompressed data
		else
		{
		    printer_data.push_back(sentbyte);
		}

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
		// Process printer command
		processcmd();

		// Reply byte here is the status byte
		linkbyte = status_byte;

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
		printer_data.clear();
		image_data.clear();
		printer_data_index = 0;
		status_byte = 0;
	    }
	    break;
	    // PRINT
	    case 0x02:
	    {
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

		    // Append printer data to vector containing image data

		    int prev_size = ((int)image_data.size());

		    image_data.resize((image_data.size() + printer_data.size()), 0);

		    for (int i = 0; i < (int)printer_data.size(); i++)
		    {
			image_data[(prev_size + i)] = printer_data[i];
		    }

		    printer_data.clear();

		    if (prev_cmd == 0x04)
		    {
			status_byte = BitSet(status_byte, 3);
		    }
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
		printer_data.clear();
		image_data.clear();
		printer_data_index = 0;
		status_byte = 0;
	    }
	    break;
	    // STATUS
	    case 0x0F:
	    {
		printer_data.clear();

		if (prev_cmd == 0x02)
		{
		    // Update status byte
		    status_byte = BitReset(status_byte, 3);
		    status_byte = BitSet(status_byte, 2);
		    status_byte = BitSet(status_byte, 1);
		}
		else if (prev_cmd == 0x0F)
		{
		    // Printing is done instantly, but we leave the "busy" bit set for one iteration,
		    // just for the sake of compatibility

		    if (TestBit(status_byte, 2) && TestBit(status_byte, 1) && !TestBit(status_byte, 4))
		    {
			status_byte = BitReset(status_byte, 1);
		    }
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

	if ((lastmargins != static_cast<uint8_t>(-1)) && ((lastmargins & 0xF) == 0) && ((printermargins >> 4) == 0))
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

    // Function definitions for Barcode Boy "serial device"
    // TODO: Add means of loading barcode data on-the-fly
    BarcodeBoy::BarcodeBoy()
    {
    
    }
    
    BarcodeBoy::~BarcodeBoy()
    {
    
    }

    // Main loop function
    void BarcodeBoy::update()
    {
	processbyte();
	transfer();
    }

    // Process byte sent from Game Boy
    void BarcodeBoy::processbyte()
    {
	// Handle edge cases of Barcode Boy logic
	if ((bcbstate != BCBState::Init) && ((sentbyte == 0x10) || (sentbyte == 0x07)))
	{
	    if (bcbstate == BCBState::Active)
	    {
		linkbyte = 0xFF;
		return;
	    }
	    else if (bcbstate == BCBState::Finished)
	    {
		bcbstate = BCBState::Init;
		bcb_counter = 0;
	    }
	}

	// Proccess data sent to the Barcode Boy
	switch (bcbstate)
	{
	    // Barcode Boy is being initialized
	    case BCBState::Init:
	    {
		// Opening handshake is sent via internal clock
		if (isintclk)
		{
		    // Game Boy sends 0x10 0x07 0x10 0x07, one at a time,
		    // and the Barcode Boy is expected to respond with
		    // 0xFF 0xFF 0x10 0x07
		    switch (bcb_counter)
		    {
			case 0:
			case 1: linkbyte = 0xFF; break;
			case 2:
			{
			    if (sentbyte == 0x10)
			    {
				linkbyte = 0x10;
			    }
			    else
			    {
				linkbyte = 0x00;
			    }
			}
			break;
			case 3:
			{
			    if (sentbyte == 0x07)
			    {
				linkbyte = 0x07;
			    }
			    else
			    {
				linkbyte = 0x00;
			    }
			}
			break;
			default: linkbyte = 0x00; break;
		    }

		    bcb_counter += 1;

		    if (bcb_counter == 4)
		    {
			bcb_counter = 0;
			bcbstate = BCBState::Active;
		    }

		    istransfer = true;
		}
	    }
	    break;
	    // Wait for barcode to be swiped
	    case BCBState::Active: break;
	    // Actually send barcode to Game Boy
	    case BCBState::SendBarcode:
	    {
		if (!isintclk)
		{
		    switch (bcb_counter)
		    {
			// 0x02 starts the barcode data transmission
			case 0:
			case 15: linkbyte = 0x02; break;
			// 0x03 ends the barcode data transmission
			case 14:
			case 29: linkbyte = 0x03; break;
			default:
			{
			    if (bcb_counter < 14)
			    {
				linkbyte = testcode[(bcb_counter - 1)];
			    }
			    else if (bcb_counter < 29)
			    {
				linkbyte = testcode[(bcb_counter - 16)];
			    }
			    // This shouldn't happen
			    else
			    {
				linkbyte = 0x00;
			    }
			}
			break;
		    }

		    bcb_counter += 1;

		    if (bcb_counter == 30)
		    {
			bcb_counter = 0;
			barcode_swiped = false;
			bcbstate = BCBState::Finished;
		    }

		    istransfer = true;
		}
	    }
	    break;
	    // This add-on state is already handled outside this switch statement,
	    // so do nothing here
	    case BCBState::Finished: break;
	}
    }

    void BarcodeBoy::transfer()
    {
	if (bcblink)
	{
	    // The response byte will only be transfered
	    // to the Game Boy under certain conditions
	    if (istransfer)
	    {
		bcblink(linkbyte);
		istransfer = false;
	    }
	}
    }

    // Function definitions for Mobile Adapter GB "serial device"
    MobileAdapterGB::MobileAdapterGB()
    {
	config_memory.fill(0);
    }

    MobileAdapterGB::~MobileAdapterGB()
    {

    }

    // Main loop function
    void MobileAdapterGB::update()
    {
	processbyte();
	transfer();
    }

    // Process byte sent from Game Boy
    void MobileAdapterGB::processbyte()
    {
	// Proccess data sent to the Mobile Adapter GB
	switch (madaptstate)
	{
	    // "Magic bytes" are 0x99 and 0x66, in that order
	    case MobileAdapterState::MagicBytes:
	    {
		calc_checksum = 0x00;
		response_data.clear();
		switch (madapt_counter)
		{
		    case 0:
		    {
			if (sent_byte == 0x99)
			{
			    link_byte = 0xD2;
			    madapt_counter = 1;
			}
			else
			{
			    link_byte = 0xD2;
			    madapt_counter = 0;
			}
		    }
		    break;
		    case 1:
		    {
			if (sent_byte == 0x66)
			{
			    link_byte = 0xD2;
			    madaptstate = MobileAdapterState::PacketHeader;
			}
			
			madapt_counter = 0;
		    }
		    break;
		}
	    }
	    break;
	    case MobileAdapterState::PacketHeader:
	    {
		switch (madapt_counter)
		{
		    case 0: adapter_cmd = sent_byte; break;
		    case 3: packet_length = sent_byte; break;
		    default: break;
		}

		calc_checksum += sent_byte;

		link_byte = 0xD2;

		madapt_counter += 1;

		if (madapt_counter == 4)
		{
		    madapt_counter = 0;
		    packet_data.clear();
		    if (packet_length == 0)
		    {
			madaptstate = MobileAdapterState::PacketChecksum;
		    }
		    else
		    {
			madaptstate = MobileAdapterState::PacketData;
		    }
		}
	    }
	    break;
	    case MobileAdapterState::PacketData:
	    {
		packet_data.push_back(sent_byte);
		calc_checksum += sent_byte;

		if (packet_data.size() == packet_length)
		{
		    madaptstate = MobileAdapterState::PacketChecksum;
		}

		link_byte = 0xD2;
	    }
	    break;
	    case MobileAdapterState::PacketChecksum:
	    {
		switch (madapt_counter)
		{
		    case 0: compare_checksum = ((uint16_t)sent_byte << 8); madapt_counter = 1; break;
		    case 1: 
		    {
			compare_checksum |= sent_byte;

			if (calc_checksum != compare_checksum)
			{
			    cout << "Checksum error" << endl;
			    link_byte = 0xF1;
			    madaptstate = MobileAdapterState::MagicBytes;
			}
			else
			{
			    link_byte = 0xD2;
			    madaptstate = MobileAdapterState::PacketAcknowledge;
			}

			madapt_counter = 0;
		    }
		    break;
		    default: break;
		}
	    }
	    break;
	    case MobileAdapterState::PacketAcknowledge:
	    {
		switch (madapt_counter)
		{
		    case 0:
		    {
			processcmd();
			link_byte = adapter_id;
			madapt_counter = 1;
		    }
		    break;
		    case 1:
		    {
			link_byte = (adapter_cmd ^ 0x80);
			madapt_counter = 0;
			madaptstate = MobileAdapterState::PacketReceive;
		    }
		    break;
		}
	    }
	    break;
	    case MobileAdapterState::PacketReceive:
	    {
		link_byte = response_data[madapt_counter];
		madapt_counter += 1;

		if (madapt_counter == static_cast<int>(response_data.size()))
		{
		    madapt_counter = 0;
		    madaptstate = MobileAdapterState::MagicBytes;
		}
	    }
	    break;
	}
    }

    void MobileAdapterGB::processcmd()
    {
	switch (adapter_cmd)
	{
	    // Begin Session
	    case 0x10:
	    {
		array<uint8_t, 8> nintendo_logo = {0x4E, 0x49, 0x4E, 0x54, 0x45, 0x4E, 0x44, 0x4F};

		if ((packet_length & packet_data.size()) != 8)
		{
		    cout << "Invalid packet size for command 0x10, should be 8" << endl;
		    prepareresponse(0x10, 0x02);
		    return;
		}

		bool is_valid = true;

		for (int i = 0; i < 8; i++)
		{
		    if (packet_data[i] != nintendo_logo[i])
		    {
			is_valid = false;
			break;
		    }
		}

		if (!is_valid)
		{
		    cout << "Invalid packet contents for command 0x10" << endl;
		    prepareresponse(0x10, 0x02);
		    return;
		}
		
		if (!begun_session)
		{
		    prepareresponse(0x10, nintendo_logo);
		    begun_session = true;
		}
		else
		{
		    cout << "Command 0x10 sent twice" << endl;
		    prepareresponse(0x10, 0x01);
		}
	    }
	    break;
	    // End Session
	    case 0x11:
	    {
		// Response packet contains empty packet data
		prepareresponse(0x11);
		begun_session = false;
	    }
	    break;
	    // Dial Telephone
	    case 0x12:
	    {
		// Dial telephone number
		// TODO: Implement a proper server-replacement protocol
		// This code will serve as a placeholder for now

		cout << "Dialing telephone number of ";

		for (int i = 2; i < static_cast<int>(packet_data.size()); i++)
		{
		    cout << ((char)packet_data[i]);

		    if (i == (static_cast<int>(packet_data.size()) - 1))
		    {
			cout << endl;
		    }
		}

		cout << endl;
		
		// The line becomes busy once the telephone number is dialed
		line_busy = true;

		// Response packet contains empty packet data
		prepareresponse(0x12);
	    }
	    break;
	    // Hang Up Telephone
	    case 0x13:
	    {
		line_busy = false;
		prepareresponse(0x13);
	    }
	    break;
	    // Wait For Telephone Call
	    case 0x14:
	    {
		prepareresponse(0x14);
	    }
	    break;
	    // Transfer Data
	    case 0x15:
	    {
		switch (port_num)
		{
		    case 25: processsmtp(); break;
		    case 80: processhttp(); break;
		    case 110: processpop(); break;
		    default: cout << "Unrecognized port number of " << dec << (int)(port_num) << endl; break;
		}
	    }
	    break;
	    // Telephone Status
	    case 0x17:
	    {
		vector<uint8_t> phone_data;

		if (line_busy)
		{
		    phone_data.push_back(0x04);
		}
		else
		{
		    phone_data.push_back(0x00);
		}

		phone_data.push_back(0x00);
		phone_data.push_back(0x00);

		prepareresponse(0x17, phone_data);
	    }
	    break;
	    // Read Configuration Data
	    case 0x19:
	    {
		int read_offset = packet_data[0];
		int read_length = packet_data[1];

		vector<uint8_t> config_read;

		config_read.push_back(0x00);

		bool is_config_valid = true;

		for (int i = 0; i < read_length; i++)
		{
		    int config_offs = (read_offset + i);

		    if (config_offs >= 192)
		    {
			cout << "Performed read outside of config area" << endl;
			is_config_valid = false;
			break;
		    }

		    config_read.push_back(config_memory[config_offs]);
		}

		if (is_config_valid)
		{
		    prepareresponse(0x19, config_read);
		}
		else
		{
		    prepareresponse(0x19, 0x02);
		}
	    }
	    break;
	    // Write Configuration Data
	    case 0x1A:
	    {
		bool is_config_valid = true;

		int write_offset = packet_data[0];

		for (int i = 1; i < static_cast<int>(packet_length); i++)
		{
		    int config_offs = (write_offset + (i - 1));

		    if (config_offs >= 192)
		    {
			cout << "Performed read outside of config area" << endl;
			is_config_valid = false;
			break;
		    }

		    config_memory[config_offs] = packet_data[i];
		}

		if (is_config_valid)
		{
		    prepareresponse(0x1A);
		}
		else
		{
		    prepareresponse(0x1A, 0x02);
		}
	    }
	    break;
	    // ISP Login
	    case 0x21:
	    {
		// TODO: Implement a proper server-replacement protocol
		// This code will serve as a placeholder for now

		// If this command is called and the line is not busy,
		// we assume that the emulated phone is disconnected
		if (!line_busy)
		{
		    cout << "ISP login with disconnected phone" << endl;
		    prepareresponse(0x21, 0x01);
		    return;
		}

		// Fetch login ID (as well as its length)
		int login_id_length = packet_data[0];

		cout << "Login ID: ";

		for (int i = 0; i < login_id_length; i++)
		{
		    cout << ((char)packet_data[(1 + i)]);
		}

		cout << endl;

		int isp_login_count = (login_id_length + 1);

		// Fetch password (as well as its length)
		int password_length = packet_data[isp_login_count];

		isp_login_count += 1;

		cout << "Password: ";

		for (int i = 0; i < password_length; i++)
		{
		    cout << ((char)packet_data[(isp_login_count + i)]);
		}

		cout << endl;

		isp_login_count += password_length;

		// Fetch primary DNS address
		cout << "Primary DNS: ";

		for (int i = 0; i < 4; i++)
		{
		    cout << dec << ((int)packet_data[(isp_login_count + i)]);

		    if (i < 3)
		    {
			cout << ".";
		    }
		}

		cout << endl;

		isp_login_count += 4;

		// Fetch secondary DNS address
		cout << "Secondary DNS: ";

		for (int i = 0; i < 4; i++)
		{
		    cout << dec << ((int)packet_data[(isp_login_count + i)]);

		    if (i < 3)
		    {
			cout << ".";
		    }
		    else
		    {
			cout << endl;
		    }
		}

		cout << endl;

		// Return placeholder IP address of 127.0.0.1 for now
		array<uint8_t, 4> ip_addr = {0x7F, 0x00, 0x00, 0x01};

		prepareresponse(0x21, ip_addr);
	    }
	    break;
	    // ISP Logout
	    case 0x22:
	    {
		prepareresponse(0x22);
	    }
	    break;
	    // Open TCP Connection
	    case 0x23:
	    {
		cout << "Opening TCP connection..." << endl;

		cout << "IP address: ";

		for (int i = 0; i < 4; i++)
		{
		    cout << dec << ((int)packet_data[i]);

		    if (i < 3)
		    {
			cout << ".";
		    }
		    else
		    {
			cout << endl;
		    }
		}

		port_num = ((packet_data[4] << 8) | packet_data[5]);

		cout << "Port number: " << dec << (int)(port_num) << endl;
		cout << endl;

		vector<uint8_t> packet;
		packet.push_back(0x00);

		// The command id when opening a TCP connection is 0xA3 (0x23 OR 0x80)
		prepareresponse(0xA3, packet);
	    }
	    break;
	    // Close TCP Connection
	    case 0x24:
	    {
		// Reset all sessions when closing the TCP connection
		begun_pop_session = false;
		begun_smtp_session = false;
		transfer_state = 0;

		vector<uint8_t> packet;
		packet.push_back(0x00);
		prepareresponse(0x24, packet);
	    }
	    break;
	    // DNS Query
	    case 0x28:
	    {
		cout << "DNS query for domain name of ";

		for (int i = 0; i < static_cast<int>(packet_data.size()); i++)
		{
		    cout << ((char)packet_data[i]);

		    if (i == (static_cast<int>(packet_data.size()) - 1))
		    {
			cout << endl;
		    }
		}

		cout << endl;

		// Return placeholder IP address of 8.8.8.8 for now
		array<uint8_t, 4> ip_addr = {0x08, 0x08, 0x08, 0x08};

		prepareresponse(0x28, ip_addr);
	    }
	    break;
	    default: cout << "Unrecognized command of " << hex << (int)(adapter_cmd) << endl; break;
	}
    }

    void MobileAdapterGB::processhttp()
    {
	// TODO: Implement a proper server-replacement protocol
	// Placeholder code will be used here for now

	string response_text = "";
	uint8_t response_id = 0x00;
	bool notfound = true;

	bool ishtml = (http_data.find(".html") != string::npos);

	http_data += string(packet_data.begin(), packet_data.end());
	cout << "HTTP data: " << http_data << endl;

	if (http_data.find("\r\n\r\n") == string::npos)
	{
	    response_text = "";
	    response_id = 0x95;
	}
	else if (transfer_state == 0)
	{
	    transfer_state = 1;

	    if (http_data.find("GET") != string::npos)
	    {
		if (http_data.find(serverin[0]) != string::npos)
		{
		    notfound = false;
		}
	    }
	}

	switch (transfer_state)
	{
	    case 0x1:
	    {
		if (notfound)
		{
		    cout << "HTTP Error: 404 Not Found" << endl;
		    response_text = "HTTP/1.0 404 Not Found\r\n";
		}
		else
		{
		    response_text = "HTTP/1.0 200 OK\r\n";
		}

		transfer_state = 2;
	    }
	    break;
	    case 0x2:
	    {
		if (!notfound)
		{
		    response_text = "";
		    response_id = 0x9F;
		    transfer_state = 0;
		    http_data = "";
		}
		else if (ishtml)
		{
		    response_text = "Content-Type: text/html\r\n\r\n";
		    response_id = 0x95;
		    transfer_state = 3;
		    http_data = "";
		}
	    }
	    break;
	    case 0x3:
	    {
		for (int i = 0; i < 254; i++)
		{
		    if (http_data_index < static_cast<int>(html_text.size()))
		    {
			response_text += html_text[http_data_index++];
		    }
		    else
		    {
			i = 255;
		    }
		}

		response_id = 0x95;
		transfer_state = (http_data_index < static_cast<int>(html_text.size())) ? 3 : 4;
	    }
	    break;
	    case 0x4:
	    {
		response_text = "";
		response_id = 0x9F;
		transfer_state = 0;
		http_data = "";
	    }
	    break;
	}

	vector<uint8_t> pop_response;

	pop_response.push_back(0x00);

	for (int i = 0; i < static_cast<int>(response_text.size()); i++)
	{
	    pop_response.push_back(response_text[i]);
	}

	prepareresponse(response_id, pop_response);
	
    }

    void MobileAdapterGB::processpop()
    {
	// TODO: Implement a proper server-replacement protocol
	// Placeholder code will be used here for now

	string response_text = "";
	uint8_t response_id = 0x00;
	if (packet_length == 1)
	{
	    response_text = "+OK\r\n";

	    begun_pop_session = !begun_pop_session;

	    if (begun_pop_session)
	    {
		response_id = 0x95;
	    }
	    else
	    {
		response_id = 0x9F;
	    }
	}
	else
	{
	    string pop_data = string(packet_data.begin(), packet_data.end());

	    int pop_cmd = -1;

	    if (pop_data.find("USER") != string::npos)
	    {
		pop_cmd = 0;
	    }
	    else if (pop_data.find("PASS") != string::npos)
	    {
		pop_cmd = 1;
	    }
	    else if (pop_data.find("QUIT") != string::npos)
	    {
		pop_cmd = 2;
	    }
	    else if (pop_data.find("STAT") != string::npos)
	    {
		pop_cmd = 3;
	    }
	    else if (pop_data.find("TOP") != string::npos)
	    {
		pop_cmd = 4;
	    }
	    else if (pop_data.find("RETR") != string::npos)
	    {
		pop_cmd = 5;
	    }
	    else if (pop_data.find("DELE") != string::npos)
	    {
		pop_cmd = 6;
	    }

	    switch (pop_cmd)
	    {
		case 0:
		{
		    string user_name = pop_data.substr(6, string::npos);
		    cout << "POP3 username is " << user_name << endl;
		    response_text = "+OK\r\n";
		    response_id = 0x95;
		}
		break;
		case 1:
		{
		    string password = pop_data.substr(6, string::npos);
		    cout << "POP3 password is " << password << endl;
		    response_text = "+OK\r\n";
		    response_id = 0x95;
		}
		break;
		case 2:
		{
		    cout << "Logging out of POP3 server" << endl;
		    response_text = "+OK\r\n";
		    response_id = 0x95;
		}
		break;
		case 3:
		{
		    // When we're not connecting to a real server,
		    // we fake an inbox with 1 message in it
		    response_text = "+OK 1\r\n";
		    response_id = 0x95;
		}
		break;
		case 4:
		{
		    // When we're not connecting to a real server,
		    // we fake a message
		    response_text = "+OK\r\n";
		    response_id = 0x95;
		    transfer_state = 0x1;
		}
		break;
		case 5:
		{
		    response_text = "+OK\r\n";
		    response_id = 0x95;
		    transfer_state = 0x11;
		}
		break;
		case 6:
		{
		    response_text = "+OK\r\n";
		    response_id = 0x95;
		}
		break;
		default:
		{
		    cout << "Unrecognized POP3 data of " << pop_data << endl;
		    response_text = "-ERR\r\n";
		    response_id = 0x95;
		}
		break;
	    }
	}

	switch (transfer_state)
	{
	    case 0x1:
	    case 0x11:
	    {
		transfer_state = (transfer_state & 0x10) ? 0x12 : 0x2;
	    }
	    break;
	    case 0x2:
	    case 0x12:
	    {
		response_text = "Date: Sat, 3 Jan 2021, 12:00:00 -0600\r\n";
		transfer_state = (transfer_state & 0x10) ? 0x13 : 0x3;
		response_id = 0x95;
	    }
	    break;
	    case 0x3:
	    case 0x13:
	    {
		response_text = "Subject: This is a test\r\n";
		transfer_state = (transfer_state & 0x10) ? 0x14 : 0x4;
		response_id = 0x95;
	    }
	    break;
	    case 0x4:
	    case 0x14:
	    {
		response_text = "From: kujomoza@libmbgb.com\r\n";
		transfer_state = (transfer_state & 0x10) ? 0x15 : 0x5;
		response_id = 0x95;
	    }
	    break;
	    case 0x5:
	    case 0x15:
	    {
		response_text = "From: user@libmbgb.com\r\n";
		transfer_state = (transfer_state & 0x10) ? 0x16 : 0x6;
		response_id = 0x95;
	    }
	    break;
	    case 0x6:
	    case 0x16:
	    {
		response_text = "Content-Type: text/plain\r\n\r\n";
		transfer_state = (transfer_state & 0x10) ? 0x17 : 0x7;
		response_id = 0x95;
	    }
	    break;
	    case 0x7:
	    {
		response_text = ".\r\n";
		transfer_state = 0;
		response_id = 0x95;
	    }
	    break;
	    case 0x17:
	    {
		response_text = "Hello from Karen Kujo, your FAVORITE waifu!\r\n\r\n";
		transfer_state = 0x18;
		response_id = 0x95;
	    }
	    break;
	    case 0x18:
	    {
		response_text = ".\r\n";
		transfer_state = 0x0;
		response_id = 0x95;
	    }
	    break;
	}

	vector<uint8_t> pop_response;

	pop_response.push_back(0x00);

	for (int i = 0; i < static_cast<int>(response_text.size()); i++)
	{
	    pop_response.push_back(response_text[i]);
	}

	prepareresponse(response_id, pop_response);
    }

    void MobileAdapterGB::processsmtp()
    {
	// TODO: Implement a proper server-replacement protocol
	// Placeholder code will be used here for now

	string response_text = "";

	uint8_t response_id = 0;
	uint8_t smtp_cmd = 0xFF;

	if ((packet_length == 1) && (transfer_state == 0) && (!begun_smtp_session))
	{
	    begun_smtp_session = true;
	    smtp_cmd = 0;
	    response_text = "220 OK\r\n";
	    response_id = 0x95;
	}
	else
	{
	    string smtp_data = string(packet_data.begin(), packet_data.end());

	    if (smtp_data.find("HELO") != string::npos)
	    {
		smtp_cmd = 1;
	    }
	    else if (smtp_data.find("MAIL FROM") != string::npos)
	    {
		smtp_cmd = 2;
	    }
	    else if (smtp_data.find("RCPT TO") != string::npos)
	    {
		smtp_cmd = 3;
	    }
	    else if (smtp_data.find("DATA") != string::npos)
	    {
		smtp_cmd = 4;
	    }
	    else if (smtp_data.find("QUIT") != string::npos)
	    {
		smtp_cmd = 5;
	    }
	    else if (smtp_data.find("\r\n.\r\n") != string::npos)
	    {
		smtp_cmd = 6;
	    }

	    switch (smtp_cmd)
	    {
		case 1:
		{
		    string sender_domain_name = smtp_data.substr(6, string::npos);
		    cout << "Starting SMTP connection with sender domain name of " << sender_domain_name << endl;
		    response_text = "250 OK\r\n";
		    response_id = 0x95;
		}
		break;
		case 2:
		{
		    string server_name = smtp_data.substr(12, (smtp_data.find_last_of(">") - 12));
		    cout << "Email getting sent from SMTP server of " << server_name << endl;
		    response_text = "250 OK\r\n";
		    response_id = 0x95;
		}
		break;
		case 3:
		{
		    string recipient_name = smtp_data.substr(10, (smtp_data.find_last_of(">") - 10));
		    cout << "Email getting sent to " << recipient_name << endl;
		    response_text = "250 OK\r\n";
		    response_id = 0x95;
		}
		break;
		case 4:
		{
		    cout << "Transmitting data..." << endl;
		    response_text = "354\r\n";
		    response_id = 0x95;
		}
		break;
		case 5:
		{
		    cout << "Ending session..." << endl;
		    response_text = "221\r\n";
		    response_id = 0x95;
		}
		break;
		case 6:
		{
		    cout << "End of data transmission" << endl;
		    response_text = "220 OK\r\n";
		    response_id = 0x95;
		}
		break;
		default:
		{
		    cout << "Unrecognized SMTP command of " << smtp_data << endl;
		    response_text = "250 OK\r\n";
		    response_id = 0x95;
		}
		break;
	    }
	}

	vector<uint8_t> smtp_response;

	smtp_response.push_back(0x00);

	for (int i = 0; i < static_cast<int>(response_text.size()); i++)
	{
	    smtp_response.push_back(response_text[i]);
	}

	prepareresponse(response_id, smtp_response);
    }

    void MobileAdapterGB::prepareresponse(uint8_t cmd, vector<uint8_t> data)
    {
	if (data.size() >= 255)
	{
	    cout << "Invalid data size of " << dec << (int)(data.size()) << endl;
	    exit(1);
	}

	uint8_t data_size = static_cast<uint8_t>(data.size());

	uint16_t checksum = (cmd + data_size);
	response_data.clear();
	response_data.push_back(0x99);
	response_data.push_back(0x66);
	response_data.push_back(static_cast<uint8_t>(cmd));
	response_data.push_back(0x00);
	response_data.push_back(0x00);
	response_data.push_back(data_size);

	for (int i = 0; i < static_cast<int>(data.size()); i++)
	{
	    response_data.push_back(data[i]);
	    checksum += data[i];
	}

	response_data.push_back((checksum >> 8));
	response_data.push_back((checksum & 0xFF));

	response_data.push_back(adapter_id);
	response_data.push_back(0x00);

	/*
	cout << "Response packet: " << endl;

	for (int i = 0; i < static_cast<int>(response_data.size()); i++)
	{
	    cout << hex << (int)(response_data[i]) << endl;
	}

	cout << endl;
	*/
    }

    // Function definitions for Turbo File GB "serial device"
    TurboFileGB::TurboFileGB()
    {

    }

    TurboFileGB::~TurboFileGB()
    {

    }

    void TurboFileGB::update()
    {
	processbyte();
	transfer();
    }

    void TurboFileGB::processbyte()
    {
	switch (turbfstate)
	{
	    case TurboFileGBState::SyncSignal1:
	    {
		if (sentbyte == 0x6C)
		{
		    linkbyte = 0xC6;
		    turbfstate =  TurboFileGBState::MagicByte;
		}
	    }
	    break;
	    case TurboFileGBState::MagicByte:
	    {
		if (sentbyte == 0x5A)
		{
		    linkbyte = 0x00;
		    turbfstate = TurboFileGBState::Command;
		}
		else
		{
		    cout << "Unrecognized magic byte of " << hex << (int)(sentbyte) << endl;
		    linkbyte = 0x00;
		    turbfstate = TurboFileGBState::SyncSignal1;
		}
	    }
	    break;
	    case TurboFileGBState::Command:
	    {
		if (turbo_file_counter == 0)
		{
		    packet_cmd = sentbyte;
		}
		else if (turbo_file_counter >= 1)
		{
		    switch (packet_cmd)
		    {
			default:
			{
			    cout << "Unrecognized Turbo File command of " << hex << (int)(packet_cmd) << endl;
			    exit(0);
			}
			break;
		    }
		}

		turbo_file_counter += 1;
	    }
	    break;
	}
    }
}
