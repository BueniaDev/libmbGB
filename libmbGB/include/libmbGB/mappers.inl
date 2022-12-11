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