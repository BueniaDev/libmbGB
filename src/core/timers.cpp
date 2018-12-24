#include "../../include/libmbGB/timers.h"
#include <iostream>
using namespace gb;
using namespace std;

namespace gb
{
    Timers::Timers()
    {
	reset();
    }

    Timers::~Timers()
    {
	cout << "Timers::Shutting down..." << endl;
    }

    void Timers::reset()
    {
	timercounter = 0;
	dividerregister = 0;
	cout << "Timers::Initialized" << endl;
    }

    void Timers::updatetimers(int cycles)
    {
	dodividerregister(cycles);

	if (!TestBit(tmem->readByte(0xFF07), 2))
	{
	    return;
	}

	int currentfreq = getclockfreq((tmem->readByte(0xFF07) & 0x3));
	timercounter += cycles;

	while (timercounter >= currentfreq)
	{
	    if ((uint16_t)(tmem->memorymap[0xFF05] + 1) > 255)
	    {
		tmem->memorymap[0xFF05] = tmem->readByte(0xFF06);
		uint8_t req = tmem->readByte(0xFF0F);
	    	req = BitSet(req, 2);
	    	tmem->writeByte(0xFF0F, req);
	    }

	    tmem->memorymap[0xFF05] += 1;

	    timercounter -= currentfreq;
	}
    }


    void Timers::dodividerregister(int cycles)
    {
	dividerregister += cycles;
	if (dividerregister >= 255)
	{
	    tmem->memorymap[0xFF04]++;
	    dividerregister -= 256;
	}
    }

    int Timers::getclockfreq(uint8_t freq)
    {
	int counter = 0;
	switch (freq)
	{
	    case 0: counter = 1024; break;
	    case 1: counter = 16; break;
	    case 2: counter = 64; break;
	    case 3: counter = 256; break;
	}
	return counter;
    }
}
