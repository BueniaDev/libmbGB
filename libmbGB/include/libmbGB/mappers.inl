// ROM only mapper
class RomOnly : public mbGBMapper
{
    public:
	RomOnly()
	{

	}

	~RomOnly()
	{

	}

	uint8_t readByte(uint16_t addr)
	{
	    uint8_t data = 0xFF;

	    if (inRange(addr, 0, 0x8000))
	    {
		data = fetchROM(addr);
	    }

	    return data;
	}

	void writeByte(uint16_t, uint8_t)
	{
	    return;
	}
};

// MBC1 mapper
//
// TODO list:
// Finish up sonar support

class GBMBC1 : public mbGBMapper
{
    public:
	GBMBC1()
	{

	}

	~GBMBC1()
	{

	}

	void configureMapper(int flags)
	{
	    is_multicart = testbit(flags, 0);
	    is_sonar = testbit(flags, 1);
	}

	void initMapper()
	{
	    writeLowerBank(0);
	    writeUpperBank(0);
	    ram.resize(getRAMSize(), 0);
	}

	void shutdownMapper()
	{
	    ram.clear();
	}

	uint8_t readByte(uint16_t addr)
	{
	    uint8_t data = 0xFF;
	    if (inRange(addr, 0, 0x4000))
	    {
		uint32_t rom_addr = addr;

		if (is_mode1)
		{
		    rom_addr += (current_rom0_bank * 0x4000);
		}

		data = fetchROM(rom_addr);
	    }
	    else if (inRange(addr, 0x4000, 0x8000))
	    {
		uint32_t rom_addr = ((addr - 0x4000) + (current_rom_bank * 0x4000));
		data = fetchROM(rom_addr);
	    }
	    else if (inRange(addr, 0xA000, 0xC000))
	    {
		if (is_ram_enabled)
		{
		    uint32_t ram_addr = (addr - 0xA000);

		    if (is_mode1)
		    {
			ram_addr += (current_ram_bank * 0x2000);
		    }

		    data = ram.at(ram_addr);
		}
	    }

	    return data;
	}

	void writeByte(uint16_t addr, uint8_t data)
	{
	    if (inRange(addr, 0x0000, 0x2000))
	    {
		if (isRAMActive())
		{
		    is_ram_enabled = ((data & 0xF) == 0xA);
		}
	    }
	    else if (inRange(addr, 0x2000, 0x4000))
	    {
		writeLowerBank(data);
	    }
	    else if (inRange(addr, 0x4000, 0x6000))
	    {
		writeUpperBank(data);
	    }
	    else if (inRange(addr, 0x6000, 0x8000))
	    {
		writeMode(data);
	    }
	    else if (inRange(addr, 0xA000, 0xC000))
	    {
		if (is_ram_enabled)
		{
		    uint32_t ram_addr = (addr - 0xA000);

		    if (is_mode1)
		    {
			ram_addr += (current_ram_bank * 0x2000);
		    }

		    ram.at(ram_addr) = data;
		}
		else if (is_sonar)
		{
		    data = readSonar();
		}
	    }
	}

    private:
	int current_rom_bank = 0;
	int current_rom0_bank = 0;
	int current_ram_bank = 0;
	bool is_multicart = false;
	bool is_sonar = false;

	vector<uint8_t> ram;

	int lower_bank = 0;
	int upper_bank = 0;
	bool is_mode1 = false;

	bool is_sonar_enable = false;
	bool prev_sonar_pulse = false;

	void writeLowerBank(uint8_t data)
	{
	    lower_bank = (data & 0x1F);

	    if (lower_bank == 0)
	    {
		lower_bank = 1;
	    }

	    createROMBank();
	}

	void writeUpperBank(uint8_t data)
	{
	    if (is_sonar)
	    {
		if (is_sonar_enable)
		{
		    if (prev_sonar_pulse && !testbit(data, 0))
		    {
			pulseSonar();
		    }
		}

		prev_sonar_pulse = testbit(data, 0);

		return;
	    }

	    upper_bank = (data & 0x3);
	    current_ram_bank = wrapRAMBanks(upper_bank);
	    createROMBank();
	}

	void pulseSonar()
	{
	    // cout << "Pulsing sonar..." << endl;
	    return;
	}

	uint8_t readSonar()
	{
	    return 0x00;
	}

	void writeMode(uint8_t data)
	{
	    if (is_sonar)
	    {
		is_sonar_enable = testbit(data, 0);
		return;
	    }

	    is_mode1 = testbit(data, 0);
	}

	void createROMBank()
	{
	    int bank_msb = 0;
	    int bank_lsb = 0;
	    if (is_multicart)
	    {
		bank_msb = (upper_bank << 4);
		bank_lsb = (lower_bank & 0xF);
	    }
	    else
	    {
		bank_msb = (upper_bank << 5);
		bank_lsb = lower_bank;
	    }

	    current_rom0_bank = wrapROMBanks(bank_msb);
	    current_rom_bank = wrapROMBanks(bank_msb | bank_lsb);
	}

	bool is_ram_enabled = false;
};

// MBC2 mapper (WIP)
//
// TODO list:
// Literally everything

class GBMBC2 : public mbGBMapper
{
    public:
	GBMBC2()
	{

	}

	~GBMBC2()
	{

	}

	void initMapper()
	{
	    current_rom_bank = 1;
	    ram.fill(0);
	}

	uint8_t readByte(uint16_t addr)
	{
	    uint8_t data = 0xFF;

	    if (inRange(addr, 0, 0x4000))
	    {
		data = fetchROM(addr);
	    }
	    else if (inRange(addr, 0x4000, 0x8000))
	    {
		uint32_t rom_addr = ((addr - 0x4000) + (current_rom_bank * 0x4000));
		data = fetchROM(rom_addr);
	    }
	    else if (inRange(addr, 0xA000, 0xC000))
	    {
		if (is_ram_enabled)
		{
		    data = (0xF0 | ram.at(addr & 0x1FF));
		}
	    }

	    return data;
	}

	void writeByte(uint16_t addr, uint8_t data)
	{
	    if (inRange(addr, 0, 0x4000))
	    {
		if (!testbit(addr, 8))
		{
		    is_ram_enabled = ((data & 0xF) == 0xA);
		}
		else
		{
		    writeROMBank(data);
		}
	    }
	    else if (inRange(addr, 0xA000, 0xC000))
	    {
		if (is_ram_enabled)
		{
		    ram.at(addr & 0x1FF) = (data & 0xF);
		}
	    }
	}

    private:
	int current_rom_bank = 0;
	bool is_ram_enabled = false;

	array<uint8_t, 512> ram;

	void writeROMBank(uint8_t data)
	{
	    current_rom_bank = (data & 0xF);

	    if (current_rom_bank == 0)
	    {
		current_rom_bank = 1;
	    }

	    current_rom_bank = wrapROMBanks(current_rom_bank);
	}
};

// MBC3 mapper
//
// TODO list:
// Implement RTC support

class GBMBC3 : public mbGBMapper
{
    public:
	GBMBC3()
	{

	}

	~GBMBC3()
	{

	}

	void configureMapper(int flags)
	{
	    is_rtc_active = testbit(flags, 0);
	}

	void initMapper()
	{
	    writeROMBank(0);
	    ram.resize(getRAMSize(), 0);
	}

	void shutdownMapper()
	{
	    ram.clear();
	}

	uint8_t readByte(uint16_t addr)
	{
	    uint8_t data = 0xFF;

	    if (inRange(addr, 0, 0x4000))
	    {
		data = fetchROM(addr);
	    }
	    else if (inRange(addr, 0x4000, 0x8000))
	    {
		uint32_t rom_addr = ((addr - 0x4000) + (current_rom_bank * 0x4000));
		data = fetchROM(rom_addr);
	    }
	    else if (inRange(addr, 0xA000, 0xC000))
	    {
		if (is_ram_rtc_enabled)
		{
		    if (isRTC())
		    {
			cout << "Reading value from RTC register of " << hex << int(current_rtc_reg) << endl;
		    }
		    else if (isRAM())
		    {
			uint32_t ram_addr = ((addr - 0xA000) + (current_ram_bank * 0x2000));
			data = ram.at(ram_addr);
		    }
		}
	    }

	    return data;
	}

	void writeByte(uint16_t addr, uint8_t data)
	{
	    if (inRange(addr, 0, 0x2000))
	    {
		if (isRAMActive() || is_rtc_active)
		{
		    is_ram_rtc_enabled = ((data & 0xF) == 0xA);
		}
	    }
	    else if (inRange(addr, 0x2000, 0x4000))
	    {
		writeROMBank(data);
	    }
	    else if (inRange(addr, 0x4000, 0x6000))
	    {
		writeRAMRTCBank(data);
	    }
	    else if (inRange(addr, 0x6000, 0x8000))
	    {
		if (is_rtc_active)
		{
		    cout << "Writing value of " << hex << int(data) << " to RTC latch register" << endl;
		}
	    }
	    else if (inRange(addr, 0xA000, 0xC000))
	    {
		if (is_ram_rtc_enabled)
		{
		    if (isRTC())
		    {
			cout << "Writing value of " << hex << int(data) << " to RTC register of " << hex << int(current_rtc_reg) << endl;
		    }
		    else if (isRAM())
		    {
			uint32_t ram_addr = ((addr - 0xA000) + (current_ram_bank * 0x2000));
			ram.at(ram_addr) = data;
		    }
		}
	    }
	}

    private:
	int current_rom_bank = 0;
	int current_ram_bank = 0;
	int current_rtc_reg = 0;

	vector<uint8_t> ram;

	int ram_rtc_reg = 0;

	bool is_rtc_active = false;

	bool isRAM()
	{
	    return (isRAMActive() && inRangeEx(ram_rtc_reg, 0x0, 0x3));
	}

	bool isRTC()
	{
	    return (is_rtc_active && inRangeEx(ram_rtc_reg, 0x8, 0xC));
	}

	bool is_ram_rtc_enabled = false;

	void writeROMBank(uint8_t data)
	{
	    int rom_bank = (data & 0x7F);

	    if (rom_bank == 0)
	    {
		rom_bank = 1;
	    }

	    current_rom_bank = wrapROMBanks(rom_bank);
	}

	void writeRAMRTCBank(uint8_t data)
	{
	    ram_rtc_reg = (data & 0xF);

	    if (isRTC())
	    {
		current_rtc_reg = ram_rtc_reg;
	    }
	    else if (isRAM())
	    {
		current_ram_bank = wrapRAMBanks(ram_rtc_reg);
	    }
	}
};

// MBC5 mapper
//
// TODO list:
// Implement proper rumble support

class GBMBC5 : public mbGBMapper
{
    public:
	GBMBC5()
	{

	}

	~GBMBC5()
	{

	}

	void configureMapper(int flags)
	{
	    is_rumble_active = testbit(flags, 0);
	}

	void initMapper()
	{
	    current_rom_bank = 1;
	    current_ram_bank = 0;
	    ram.resize(getRAMSize(), 0);
	}

	void shutdownMapper()
	{
	    ram.clear();
	}

	uint8_t readByte(uint16_t addr)
	{
	    uint8_t data = 0xFF;

	    if (inRange(addr, 0, 0x4000))
	    {
		data = fetchROM(addr);
	    }
	    else if (inRange(addr, 0x4000, 0x8000))
	    {
		uint32_t rom_addr = ((addr - 0x4000) + (current_rom_bank * 0x4000));
		data = fetchROM(rom_addr);
	    }
	    else if (inRange(addr, 0xA000, 0xC000))
	    {
		if (is_ram_enabled)
		{
		    uint32_t ram_addr = ((addr - 0xA000) + (current_ram_bank * 0x2000));
		    data = ram.at(ram_addr);
		}
	    }

	    return data;
	}

	void writeByte(uint16_t addr, uint8_t data)
	{
	    if (inRange(addr, 0, 0x2000))
	    {
		if (isRAMActive())
		{
		    is_ram_enabled = ((data & 0xF) == 0xA);
		}
	    }
	    else if (inRange(addr, 0x2000, 0x3000))
	    {
		writeLowerROMBank(data);
	    }
	    else if (inRange(addr, 0x3000, 0x4000))
	    {
		writeUpperROMBank(data);
	    }
	    else if (inRange(addr, 0x4000, 0x6000))
	    {
		writeRAMRumbleBank(data);
	    }
	    else if (inRange(addr, 0xA000, 0xC000))
	    {
		if (is_ram_enabled)
		{
		    uint32_t ram_addr = ((addr - 0xA000) + (current_ram_bank * 0x2000));
		    ram.at(ram_addr) = data;
		}
	    }
	}

    private:
	bool is_rumble_active = false;
	bool is_ram_enabled = false;

	vector<uint8_t> ram;

	int current_rom_bank = 0;
	int current_ram_bank = 0;

	void writeLowerROMBank(uint8_t data)
	{
	    current_rom_bank = ((current_rom_bank & 0x100) | data);
	    current_rom_bank = wrapROMBanks(current_rom_bank);
	}

	void writeUpperROMBank(uint8_t data)
	{
	    current_rom_bank = ((current_rom_bank & 0xFF) | ((data & 0x1) << 8));
	    current_rom_bank = wrapROMBanks(current_rom_bank);
	}

	void writeRAMRumbleBank(uint8_t data)
	{
	    int ram_mask = (is_rumble_active) ? 0x7 : 0xF;
	    int ram_bank = (data & ram_mask);

	    current_ram_bank = wrapRAMBanks(ram_bank);

	    if (is_rumble_active)
	    {
		writeRumble(testbit(data, 3));
	    }
	}

	void writeRumble(bool is_enabled)
	{
	    if (is_enabled)
	    {
		cout << "Starting rumble..." << endl;
	    }
	    else
	    {
		cout << "Stopping rumble..." << endl;
	    }
	}
};

// MBC7 mapper (used by Kirby Tilt-n-Tumble and Command Master) (WIP)
//
// TODO list:
// Literally everything

class GBMBC7 : public mbGBMapper
{
    public:
	GBMBC7()
	{

	}

	~GBMBC7()
	{

	}

	void initMapper()
	{
	    current_rom_bank = 1;
	}

	uint8_t readByte(uint16_t addr)
	{
	    uint8_t data = 0;
	    if (inRange(addr, 0, 0x4000))
	    {
		data = fetchROM(addr);
	    }
	    else
	    {
		data = mbGBMapper::readByte(addr);
	    }

	    return data;
	}

	void writeByte(uint16_t addr, uint8_t data)
	{
	    if (inRange(addr, 0x2000, 0x400))
	    {
		writeROMBank(data);
	    }
	    else
	    {
		mbGBMapper::writeByte(addr, data);
	    }
	}

    private:
	int current_rom_bank = 0;

	void writeROMBank(uint8_t data)
	{
	    current_rom_bank = wrapROMBanks(data & 0x7F);
	}
};


// MMM01 mapper (used by various multicart games) (WIP)
//
// TODO list:
// Literally everything

class GBMMM01 : public mbGBMapper
{
    public:
	GBMMM01()
	{

	}

	~GBMMM01()
	{

	}

	void initMapper()
	{
	    is_mapped = false;
	}

	uint8_t readByte(uint16_t addr)
	{
	    uint8_t data = 0xFF;
	    if (!is_mapped)
	    {
		if (inRange(addr, 0, 0x8000))
		{
		    uint32_t rom_addr = (addr + (getROMSize() - 0x8000));
		    data = fetchROM(rom_addr);
		}
	    }
	    else
	    {
		cout << "Reading in mapped mode..." << endl;
		data = mbGBMapper::readByte(addr);
	    }

	    return data;
	}

    private:
	bool is_mapped = false;
};


// HuC1 mapper (used in several Hudson Soft games) (WIP)
//
// TODO list:
// Infrared support

class GBHuC1 : public mbGBMapper
{
    public:
	GBHuC1()
	{

	}

	~GBHuC1()
	{

	}

	void initMapper()
	{
	    current_rom_bank = 1;
	    current_ram_bank = 0;

	    ram.resize(getRAMSize(), 0);
	}

	uint8_t readByte(uint16_t addr)
	{
	    uint8_t data = 0xFF;
	    if (inRange(addr, 0, 0x4000))
	    {
		data = fetchROM(addr);
	    }
	    else if (inRange(addr, 0x4000, 0x8000))
	    {
		uint32_t rom_addr = ((addr - 0x4000) + (current_rom_bank * 0x4000));
		data = fetchROM(rom_addr);
	    }
	    else if (inRange(addr, 0xA000, 0xC000))
	    {
		if (is_ir_mode)
		{
		    data = readIR();
		}
		else
		{
		    uint32_t ram_addr = ((addr - 0xA000) + (current_ram_bank * 0x2000));
		    data = ram.at(ram_addr);
		}
	    }
	    else
	    {
		data = mbGBMapper::readByte(addr);
	    }

	    return data;
	}

	void writeByte(uint16_t addr, uint8_t data)
	{
	    if (inRange(addr, 0, 0x2000))
	    {
		is_ir_mode = ((data & 0xF) == 0xE);
	    }
	    else if (inRange(addr, 0x2000, 0x4000))
	    {
		writeROMBank(data);
	    }
	    else if (inRange(addr, 0x4000, 0x6000))
	    {
		writeRAMBank(data);
	    }
	    else if (inRange(addr, 0x6000, 0x8000))
	    {
		return;
	    }
	    else if (inRange(addr, 0xA000, 0xC000))
	    {
		if (is_ir_mode)
		{
		    writeIR(data);
		}
		else
		{
		    uint32_t ram_addr = ((addr - 0xA000) + (current_ram_bank * 0x2000));
		    ram.at(ram_addr) = data;
		}
	    }
	    else
	    {
		mbGBMapper::writeByte(addr, data);
	    }
	}

    private:
	int current_rom_bank = 0;
	int current_ram_bank = 0;

	vector<uint8_t> ram;

	bool is_ir_mode = false;

	void writeROMBank(uint8_t data)
	{
	    current_rom_bank = (data & 0x3F);

	    if (current_rom_bank == 0)
	    {
		current_rom_bank = 1;
	    }

	    current_rom_bank = wrapROMBanks(current_rom_bank);
	}

	void writeRAMBank(uint8_t data)
	{
	    current_ram_bank = wrapRAMBanks(data & 0x3);
	}

	uint8_t readIR()
	{
	    return (0xC0 | recIRSignal());
	}

	void writeIR(uint8_t data)
	{
	    sendIRSignal(testbit(data, 0));
	}

	bool recIRSignal()
	{
	    return false;
	}

	void sendIRSignal(bool)
	{
	    return;
	}
};

// HuC3 mapper (used in several Hudson Soft games) (WIP)
//
// TODO list:
// Literally everything

class GBHuC3 : public mbGBMapper
{
    public:
	GBHuC3()
	{

	}

	~GBHuC3()
	{

	}
};

// Gameboy Camera mapper (WIP)
//
// TODO list:
// Literally everything

class GBCamera : public mbGBMapper
{
    public:
	GBCamera()
	{

	}

	~GBCamera()
	{

	}

	uint8_t readByte(uint16_t addr)
	{
	    uint8_t data = 0xFF;

	    if (inRange(addr, 0, 0x4000))
	    {
		data = fetchROM(addr);
	    }
	    else
	    {
		data = mbGBMapper::readByte(addr);
	    }

	    return data;
	}
};


// M161 mapper (used by Mani 4-in-1 (DMG-601 CHN))
class GBM161 : public mbGBMapper
{
    public:
	GBM161()
	{

	}

	~GBM161()
	{

	}

	void initMapper()
	{
	    current_rom_bank = 0;
	    rom_bankswitch = false;
	}

	uint8_t readByte(uint16_t addr)
	{
	    uint8_t data = 0xFF;

	    if (inRange(addr, 0, 0x8000))
	    {
		uint32_t rom_addr = (addr + (current_rom_bank * 0x8000));
		data = fetchROM(rom_addr);
	    }

	    return data;
	}

	void writeByte(uint16_t addr, uint8_t data)
	{
	    if (inRange(addr, 0, 0x8000))
	    {
		if (!rom_bankswitch)
		{
		    current_rom_bank = (data & 0x7);
		    rom_bankswitch = true;
		}
	    }
	}

    private:
	int current_rom_bank = 0;
	int rom_bankswitch = false;
};


// Template for other mapper implementations
// (DO NOT uncomment)

/*
class GBTemplate : public mbGBMapper
{
    public:
	GBTemplate()
	{

	}

	~GBTemplate()
	{

	}
};
*/