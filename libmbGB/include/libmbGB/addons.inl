// Serial device used for debugging
class LIBMBGB_API GBSerialDebug : public GBSerialDevice
{
    public:
	GBSerialDebug()
	{

	}

	~GBSerialDebug()
	{

	}

	void sendByte(uint8_t data, bool is_mode)
	{
	    logData(data, is_mode);
	    if (is_mode)
	    {
		transfer();
	    }
	}

    private:
	void logData(uint8_t data, bool is_mode)
	{
	    string mode_str = is_mode ? "internal" : "external";
	    cout << "Sending data of " << hex << int(data) << " over " << mode_str << " clock" << endl;
	}
};

// Game Boy Printer device (WIP)
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

class LIBMBGB_API GBPrinter : public GBSerialDevice
{
    public:
	GBPrinter()
	{

	}

	~GBPrinter()
	{

	}

	void sendByte(uint8_t data, bool is_mode)
	{
	    if (is_mode)
	    {
		sent_byte = data;
		processByte();
		transfer();
	    }
	}

	uint8_t getByte()
	{
	    return rec_byte;
	}

    private:
	uint8_t sent_byte = 0;
	uint8_t rec_byte = 0;

	enum GBPrinterState : int
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

	GBPrinterState printer_state = MagicBytes;

	void processByte()
	{
	    switch (printer_state)
	    {
		default:
		{
		    cout << "Unrecognized printer state of " << dec << int(printer_state) << endl;
		    throw runtime_error("Serial addon error");
		}
		break;
	    }
	}
};