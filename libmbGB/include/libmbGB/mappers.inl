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

	vector<uint8_t> saveBackup()
	{
	    return ram;
	}

	void loadBackup(vector<uint8_t> data)
	{
	    if (data.empty())
	    {
		return;
	    }

	    copy(data.begin(), data.end(), ram.begin());
	}

	void handleSavestate(mbGBSavestate &file)
	{
	    file.varInt(current_rom_bank);
	    file.varInt(current_rom0_bank);
	    file.varInt(current_ram_bank);
	    file.varBool32(is_multicart);
	    file.varBool32(is_sonar);
	    file.varVec8(ram);
	    file.varInt(lower_bank);
	    file.varInt(upper_bank);
	    file.varBool32(is_mode1);
	    file.varBool32(is_sonar_enable);
	    file.varBool32(prev_sonar_pulse);
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

// MBC2 mapper

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

	vector<uint8_t> saveBackup()
	{
	    return vector<uint8_t>(ram.begin(), ram.end());
	}

	void loadBackup(vector<uint8_t> data)
	{
	    if (data.empty())
	    {
		return;
	    }

	    copy(data.begin(), data.end(), ram.begin());
	}

	void handleSavestate(mbGBSavestate &file)
	{
	    file.varInt(current_rom_bank);
	    file.varBool32(is_ram_enabled);
	    file.varArray8(ram);
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
			// cout << "Reading value from RTC register of " << hex << int(current_rtc_reg) << endl;

			switch (current_rtc_reg)
			{
			    case 0x8: data = latch_secs; break;
			    case 0x9: data = latch_mins; break;
			    case 0xA: data = latch_hours; break;
			    case 0xB:
			    {
				data = (latch_day_counter & 0xFF);
			    }
			    break;
			    case 0xC:
			    {
				data = (testbit(latch_day_counter, 8) | (latch_rtc_halt << 6) | (latch_day_carry << 7));
			    }
			    break;
			}
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
		    if (!testbit(data, 0) && !is_rtc_latch)
		    {
			is_rtc_latch = true;
		    }
		    else if (testbit(data, 0) && is_rtc_latch)
		    {
			latchTimer();
			is_rtc_latch = false;
		    }
		}
	    }
	    else if (inRange(addr, 0xA000, 0xC000))
	    {
		if (is_ram_rtc_enabled)
		{
		    if (isRTC())
		    {
			switch (current_rtc_reg)
			{
			    case 0x8:
			    {
				tick_counter = 0;
				real_secs = (data & 0x3F);
			    }
			    break;
			    case 0x9: real_mins = (data & 0x3F); break;
			    case 0xA: real_hours = (data & 0x1F); break;
			    case 0xB:
			    {
				real_day_counter = ((real_day_counter & 0x100) | data);
			    }
			    break;
			    case 0xC:
			    {
				real_day_counter = ((real_day_counter & 0xFF) | (testbit(data, 0) << 8));
				is_rtc_halt = testbit(data, 6);
				real_day_carry = testbit(data, 7);
			    }
			    break;
			}

			// cout << "Writing value of " << hex << int(data) << " to RTC register of " << hex << int(current_rtc_reg) << endl;
		    }
		    else if (isRAM())
		    {
			uint32_t ram_addr = ((addr - 0xA000) + (current_ram_bank * 0x2000));
			ram.at(ram_addr) = data;
		    }
		}
	    }
	}

	vector<uint8_t> saveBackup()
	{
	    return ram;
	}

	void loadBackup(vector<uint8_t> data)
	{
	    if (data.empty())
	    {
		return;
	    }

	    copy(data.begin(), data.end(), ram.begin());
	}

	void handleSavestate(mbGBSavestate &file)
	{
	    file.varInt(current_rom_bank);
	    file.varInt(current_ram_bank);
	    file.varInt(current_rtc_reg);
	    file.varVec8(ram);
	    file.varInt(ram_rtc_reg);
	    file.varBool32(is_rtc_active);
	    file.varBool32(is_ram_rtc_enabled);
	}

	void tickMapper()
	{
	    if (!is_rtc_active)
	    {
		return;
	    }

	    if (is_rtc_halt)
	    {
		return;
	    }

	    tick_counter += 1;

	    if (tick_counter == 4194304)
	    {
		tick_counter = 0;
		tickRTC();
	    }
	}

    private:
	int current_rom_bank = 0;
	int current_ram_bank = 0;
	int current_rtc_reg = 0;

	vector<uint8_t> ram;

	int ram_rtc_reg = 0;

	bool is_rtc_active = false;

	int tick_counter = 0;

	bool is_rtc_latch = false;

	uint8_t latch_secs = 0;
	uint8_t latch_mins = 0;
	uint8_t latch_hours = 0;
	uint16_t latch_day_counter = 0;
	bool latch_rtc_halt = false;
	bool latch_day_carry = false;

	uint8_t real_secs = 0;
	uint8_t real_mins = 0;
	uint8_t real_hours = 0;
	uint16_t real_day_counter = 0;
	bool is_rtc_halt = false;
	bool real_day_carry = false;

	void tickRTC()
	{
	    real_secs = ((real_secs + 1) & 0x3F);

	    if (real_secs == 60)
	    {
		real_secs = 0;
		real_mins = ((real_mins + 1) & 0x3F);

		if (real_mins == 60)
		{
		    real_mins = 0;
		    real_hours = ((real_hours + 1) & 0x1F);

		    if (real_hours == 24)
		    {
			real_hours = 0;
			real_day_counter += 1;

			if (real_day_counter == 512)
			{
			    real_day_counter = 0;
			    real_day_carry = true;
			}
		    }
		}
	    }
	}

	void latchTimer()
	{
	    latch_secs = real_secs;
	    latch_mins = real_mins;
	    latch_hours = real_hours;
	    latch_day_counter = real_day_counter;
	    latch_rtc_halt = is_rtc_halt;
	    latch_day_carry = real_day_carry;
	}

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

	vector<uint8_t> saveBackup()
	{
	    return ram;
	}

	void loadBackup(vector<uint8_t> data)
	{
	    if (data.empty())
	    {
		return;
	    }

	    copy(data.begin(), data.end(), ram.begin());
	}

	void tickMapper()
	{
	    rumble_strength &= 0x7;
	    rumble_on_cycles += rumble_strength;
	    rumble_off_cycles += (rumble_strength ^ 7);
	}

	void handleVBlank()
	{
	    int total_cycles = (rumble_on_cycles + rumble_off_cycles);

	    if (total_cycles > 0)
	    {
		double strength = (rumble_on_cycles / double(total_cycles));
		callRumbleCallback(strength);
		rumble_on_cycles = 0;
		rumble_off_cycles = 0;
	    }
	}

	void handleSavestate(mbGBSavestate &file)
	{
	    file.varBool32(is_rumble_active);
	    file.varBool32(is_ram_enabled);
	    file.varVec8(ram);
	    file.varInt(current_rom_bank);
	    file.varInt(current_ram_bank);

	    file.varInt(rumble_on_cycles);
	    file.varInt(rumble_off_cycles);
	    file.varInt(rumble_strength);
	}

    private:
	bool is_rumble_active = false;
	bool is_ram_enabled = false;

	vector<uint8_t> ram;

	int current_rom_bank = 0;
	int current_ram_bank = 0;

	int rumble_on_cycles = 0;
	int rumble_off_cycles = 0;

	int rumble_strength = 0;

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
		rumble_strength = 7;
	    }
	    else
	    {
		rumble_strength = 0;
	    }
	}
};

// MBC7 mapper (used by Kirby Tilt-n-Tumble and Command Master) (WIP)
//
// TODO list:
// Figure out proper values for accelerometer

class GBMBC7 : public mbGBMapper
{
    public:
	GBMBC7()
	{

	}

	~GBMBC7()
	{

	}

	void configureMapper(int flags)
	{
	    is_4k_eeprom = testbit(flags, 0);
	}

	void initMapper()
	{
	    accel_xpos_latch = 0x8000;
	    accel_ypos_latch = 0x8000;
	    current_rom_bank = 1;

	    eeprom.init(is_4k_eeprom);
	}

	void shutdownMapper()
	{
	    eeprom.shutdown();
	}

	uint8_t readByte(uint16_t addr)
	{
	    uint8_t data = 0;
	    if (inRange(addr, 0, 0x4000))
	    {
		data = fetchROM(addr);
	    }
	    else if (inRange(addr, 0x4000, 0x8000))
	    {
		uint32_t rom_addr = ((addr - 0x4000) + (current_rom_bank * 0x4000));
		data = fetchROM(rom_addr);
	    }
	    else if (inRange(addr, 0xA000, 0xB000))
	    {
		if (is_ram_enable1 && is_ram_enable2)
		{
		    data = readReg(addr);
		}
		else
		{
		    data = 0xFF;
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
	    if (inRange(addr, 0x0000, 0x2000))
	    {
		is_ram_enable1 = ((data & 0xF) == 0xA);

		if (!is_ram_enable1)
		{
		    is_ram_enable2 = false;
		}
	    }
	    else if (inRange(addr, 0x2000, 0x4000))
	    {
		current_rom_bank = wrapROMBanks(data);
	    }
	    else if (inRange(addr, 0x4000, 0x6000))
	    {
		is_ram_enable2 = (data == 0x40);
	    }
	    else if (inRange(addr, 0xA000, 0xB000))
	    {
		if (is_ram_enable1 && is_ram_enable2)
		{
		    writeReg(addr, data);
		}
	    }
	    else
	    {
		mbGBMapper::writeByte(addr, data);
	    }
	}

	void updateAccel(float xpos, float ypos)
	{
	    accel_xpos = int(xpos * 0x70);
	    accel_ypos = int(ypos * 0x70);
	}

	vector<uint8_t> saveBackup()
	{
	    return eeprom.saveBackup();
	}

	void loadBackup(vector<uint8_t> data)
	{
	    eeprom.loadBackup(data);
	}

	void handleSavestate(mbGBSavestate &file)
	{
	    file.varInt(current_rom_bank);
	    file.varBool32(is_ram_enable1);
	    file.varBool32(is_ram_enable2);
	    file.varBool32(is_4k_eeprom);

	    file.var16(accel_xpos);
	    file.var16(accel_ypos);

	    file.var16(accel_xpos_latch);
	    file.var16(accel_ypos_latch);
	    file.varBool32(is_erased);

	    // TODO: Serialize EEPROM
	}

    private:
	int current_rom_bank = 0;
	bool is_ram_enable1 = false;
	bool is_ram_enable2 = false;

	bool is_4k_eeprom = false;

	MBC7EEPROM eeprom;

	uint16_t accel_xpos = 0;
	uint16_t accel_ypos = 0;

	uint16_t accel_xpos_latch = 0;
	uint16_t accel_ypos_latch = 0;

	bool is_erased = false;

	void eraseAccel()
	{
	    accel_xpos_latch = 0x8000;
	    accel_ypos_latch = 0x8000;
	    is_erased = true;
	}

	void latchAccel()
	{
	    accel_xpos_latch = (0x81D0 + accel_xpos);
	    accel_ypos_latch = (0x81D0 + accel_ypos);
	    is_erased = false;
	}

	uint8_t readReg(uint16_t addr)
	{
	    uint8_t data = 0;
	    int reg = ((addr >> 4) & 0xF);

	    switch (reg)
	    {
		case 0x2: data = (accel_xpos_latch & 0xFF); break;
		case 0x3: data = (accel_xpos_latch >> 8); break;
		case 0x4: data = (accel_ypos_latch & 0xFF); break;
		case 0x5: data = (accel_ypos_latch >> 8); break;
		case 0x8: data = eeprom.readIO(); break;
		default:
		{
		    cout << "Unrecognized read from MBC7 register of " << hex << int(reg) << endl;
		    throw runtime_error("mbGB error");
		}
		break;
	    }

	    return data;
	}

	void writeReg(uint16_t addr, uint8_t data)
	{
	    int reg = ((addr >> 4) & 0xF);

	    switch (reg)
	    {
		case 0x0:
		{
		    if (data == 0x55)
		    {
			eraseAccel();
		    }
		}
		break;
		case 0x1:
		{
		    if (is_erased && (data == 0xAA))
		    {
			latchAccel();
		    }
		}
		break;
		case 0x8: eeprom.writeIO(data); break;
		default:
		{
		    cout << "Unrecognized write of " << hex << int(data) << " to MBC7 register of " << hex << int(reg) << endl;
		    throw runtime_error("mbGB error");
		}
		break;
	    }
	}
};


// MMM01 mapper (used by various multicart games) (WIP)
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
};


// HuC1 mapper (used in several Hudson Soft games)
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

	void handleSavestate(mbGBSavestate &file)
	{
	    file.varInt(current_rom_bank);
	    file.varInt(current_ram_bank);

	    file.varVec8(ram);
	    file.varBool32(is_ir_mode);
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

	void writeByte(uint16_t addr, uint8_t data)
	{
	    if (inRange(addr, 0x6000, 0x8000))
	    {
		return;
	    }
	    else
	    {
		mbGBMapper::writeByte(addr, data);
	    }
	}
};

// Gameboy Camera mapper (WIP)
//
// TODO list:
// Implement camera registers
// Implement other camera functionality

class GBCamera : public mbGBMapper
{
    public:
	GBCamera()
	{

	}

	~GBCamera()
	{

	}

	void initMapper()
	{
	    current_rom_bank = 1;
	    current_ram_bank = 0;
	    is_ram_enabled = false;
	    ram.resize(getRAMSize(), 0);
	}

	void shutdownMapper()
	{
	    ram.clear();
	}

	uint8_t readByte(uint16_t addr)
	{
	    uint8_t data = 0;

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
		if (is_reg_active)
		{
		    cout << "Reading from camera register of " << hex << int(addr & 0x7F) << endl;
		    data = 0x00;
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
		is_ram_enabled = ((data & 0xF) == 0xA);
	    }
	    else if (inRange(addr, 0x2000, 0x4000))
	    {
		current_rom_bank = wrapROMBanks(data & 0x3F);
	    }
	    else if (inRange(addr, 0x4000, 0x6000))
	    {
		is_reg_active = testbit(data, 4);

		if (!is_reg_active)
		{
		    current_ram_bank = wrapRAMBanks(data & 0xF);
		}
	    }
	    else if (inRange(addr, 0xA000, 0xC000))
	    {
		if (is_reg_active)
		{
		    cout << "Writing value of " << hex << int(data) << " to camera register of " << hex << int(addr & 0x7F) << endl;
		    return;
		}
		else if (is_ram_enabled)
		{
		    uint32_t ram_addr = ((addr - 0xA000) + (current_ram_bank * 0x2000));
		    data = ram.at(ram_addr);
		}
	    }
	    else
	    {
		mbGBMapper::writeByte(addr, data);
	    }
	}

	void handleSavestate(mbGBSavestate &file)
	{
	    file.varInt(current_rom_bank);
	    file.varInt(current_ram_bank);
	    file.varBool32(is_reg_active);
	    file.varBool32(is_ram_enabled);
	    file.varVec8(ram);
	}

    private:
	int current_rom_bank = 0;
	int current_ram_bank = 0;
	bool is_reg_active = false;
	bool is_ram_enabled = false;

	vector<uint8_t> ram;
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

	void handleSavestate(mbGBSavestate &file)
	{
	    file.varInt(current_rom_bank);
	    file.varBool32(rom_bankswitch);
	}

    private:
	int current_rom_bank = 0;
	bool rom_bankswitch = false;
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