// Game Boy Printer device (WIP)
/* TODO: Emulation is VERY preliminary and basic at this point, and it assumes that the ROM correctly uses the printer's interface.

Incorrect usage and any "undefined behavior" are not correctly emulated, mainly for two key reasons:

1. Said usage and "undefined behavior" are completely undocumented, and:
2. I currently don't have a Game Boy Printer of my own to figure out that behavior myself.

This emulation also does not currently emulate communication timeout, which means that a bug
might prevent proper printer operation until the emulated GameBoy is restarted.

Most of the Game Boy Printers released into the market come with a Toshiba TMP87CM40AF 8-bit MCU that contains 32 KB of internal ROM and 1024 bytes of internal ram extended by an additional 8192 byte ram chip.

In terms of any incorrect usage and "undefined behavior" listed below, only a dissassembly of the MCU's 32KB ROM can confirm all this. However, to the best of all our collective knowledge, that has never been done or even attempted before.

If you have any information about the internal ROM that the Game Boy Printer's MCU uses,
drop me a line at buenia.mbemu@gmail.com.
*/

class GBPrinter : public GBSerialDevice
{
    public:
	GBPrinter()
	{

	}

	~GBPrinter()
	{

	}

	void sendBit(bool bit)
	{
	    sent_byte = ((sent_byte << 1) | bit);

	    shift_counter += 1;

	    if (shift_counter == 8)
	    {
		shift_counter = 0;
		processByte();
	    }
	}

	bool getBit()
	{
	    bool bit = testbit(rec_byte, 7);
	    rec_byte <<= 1;
	    return bit;
	}

    private:
	uint8_t sent_byte = 0;
	uint8_t rec_byte = 0;

	int shift_counter = 0;

	int printer_cmd = 0;

	enum GBPrinterState
	{
	    MagicBytes = 0,
	    Command = 1,
	    Compression = 2,
	    PacketLength = 3,
	    PacketData = 4,
	    Checksum = 5,
	    Acknowledge = 6,
	    Status = 7
	};

	GBPrinterState printer_state = MagicBytes;

	bool is_first_magic_byte = false;

	bool is_compression = false;

	uint16_t packet_length = 0;

	int data_length = 0;
	int data_counter = 0;

	bool is_byte_msb = false;

	uint16_t calc_checksum = 0;
	uint8_t status_byte = 0;

	void processByte()
	{
	    switch (printer_state)
	    {
		case MagicBytes:
		{
		    if (!is_first_magic_byte && (sent_byte == 0x88))
		    {
			cout << "Sending first magic byte..." << endl;
			is_first_magic_byte = true;
		    }
		    else if (is_first_magic_byte && (sent_byte == 0x33))
		    {
			cout << "Sending second magic byte..." << endl;
			is_first_magic_byte = false;
			printer_state = Command;
		    }

		    rec_byte = 0x00;
		}
		break;
		case Command:
		{
		    printer_cmd = sent_byte;
		    printer_state = Compression;
		    calc_checksum = sent_byte;
		    rec_byte = 0x00;
		}
		break;
		case Compression:
		{
		    calc_checksum += sent_byte;

		    if (sent_byte == 0x00)
		    {
			cout << "Compression disabled" << endl;
			is_compression = false;
		    }
		    else
		    {
			cout << "Compression enabled" << endl;
			is_compression = true;
		    }

		    printer_state = PacketLength;
		    rec_byte = 0x00;
		}
		break;
		case PacketLength:
		{
		    calc_checksum += sent_byte;

		    if (!is_byte_msb)
		    {
			packet_length = sent_byte;
			is_byte_msb = true;
		    }
		    else
		    {
			packet_length |= (sent_byte << 8);
			is_byte_msb = false;

			if ((packet_length >= 0) && (packet_length <= 640))
			{
			    data_length = packet_length;
			    data_counter = 0;

			    if ((printer_cmd == 0x02) || ((printer_cmd == 0x04) && (packet_length > 0)))
			    {
				printer_state = PacketData;
			    }
			    else
			    {
				printer_state = Checksum;
			    }
			}
			else
			{
			    cout << "Invalid packet length of " << dec << int(packet_length) << endl;
			    status_byte = setbit(status_byte, 4);
			    printer_state = MagicBytes;
			}
		    }

		    rec_byte = 0x00;
		}
		break;
		default:
		{
		    cout << "Unrecognized printer state of " << dec << int(printer_state) << ", byte of " << hex << int(sent_byte) << endl;
		    throw runtime_error("mbGB error");
		}
		break;
	    }
	}
};