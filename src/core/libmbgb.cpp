#include "../../include/libmbGB/libmbgb.h"
#include <iostream>
using namespace gb;
using namespace std;

namespace gb
{
    DMGCore::DMGCore()
    {
	corecpu.mem = &coremmu;
	coregpu.gmem = &coremmu;
	coregpu.gcpu = &corecpu;

	cout << "DMGCore::Initialized" << endl;
    }

    DMGCore::~DMGCore()
    {
	cout << "DMGCore::Shutdown" << endl;
    }

    void DMGCore::loadROM(string filename)
    {
	FILE *rom;
	rom = fopen(filename.c_str(), "rb");
	fread(coremmu.memorymap, sizeof(uint8_t), 0x8000, rom);
	fclose(rom);
	cout << filename << " succesfully loaded." << endl;
    }

    void DMGCore::loadBIOS(string filename)
    {
	FILE *fh;
	fh = fopen(filename.c_str(), "rb");
	fread(coremmu.bios, sizeof(uint8_t), 0x100, fh);
	fclose(fh);
	cout << "BIOS succesfully loaded." << endl;
    }

    void DMGCore::runcore()
    {
	corecpu.m_cycles = 0;
	int maxcycles = 69905;
	while (corecpu.m_cycles < maxcycles)
	{
	    corecpu.executenextopcode();
	    if (!corecpu.stopped)
	    {
	        coregpu.updategraphics(corecpu.m_cycles);
	        corecpu.dointerrupts();
	    }
	}
    }
}
