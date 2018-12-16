#include "../../include/libmbGB/libmbgb.h"
#include <iostream>
#include <cstring>
#include <vector>
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
	cout << "DMGCore::Shutting down..." << endl;
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

    bool DMGCore::getoptions(int argc, char* argv[])
    {
	if (argc < 2)
	{
	    cout << "Usage: " << argv[0] << " ROM [options]" << endl;
	    cout << endl;
	    cout << "Options:" << endl;
	    cout << endl;
	    cout << "-b [FILE], --bios [FILE] \t\t Loads and uses a BIOS file" << endl;
	    cout << endl;
	    return false;
	}

	for (int i = 2; i < argc; i++)
	{
	    if ((strncmp(argv[i], "-b", 2) == 0 || (strncmp(argv[i], "--bios", 6) == 0)))
	    {
		if ((i + 1) == argc)
		{
		    cout << "Error::No BIOS file in arguments" << endl;
		    return false;
		}
		else
		{
		    coremmu.biosload = true;
		}
	    }
	}

	return true;
    }

    void DMGCore::runcore()
    {
	corecpu.m_cycles = 0;
	int maxcycles = 69905;
	while (corecpu.m_cycles < maxcycles)
	{
	    int corecycles = corecpu.m_cycles;
	    corecpu.executenextopcode();
	    int cycles = corecpu.m_cycles - corecycles;

	    if (!corecpu.stopped)
	    {
	        coregpu.updategraphics(cycles);
	        corecpu.dointerrupts();
	    }
	}
    }
}
