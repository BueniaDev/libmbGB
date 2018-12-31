#include "../../include/libmbGB/libmbgb.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
using namespace gb;
using namespace std;

namespace gb
{
    DMGCore::DMGCore()
    {
	reset();
    }

    DMGCore::~DMGCore()
    {
	cout << "DMGCore::Shutting down..." << endl;
    }

    void DMGCore::reset()
    {
	corecpu.mem = &coremmu;
	coregpu.gmem = &coremmu;
	coretimers.tmem = &coremmu;
	coremmu.joypad = &coreinput;
	corecpu.timers = &coretimers;
	corecpu.gpu = &coregpu;

	cout << "DMGCore::Initialized" << endl;
    }

    bool DMGCore::loadROM(string filename)
    {
	return coremmu.loadROM(filename);
    }

    bool DMGCore::loadBIOS(string filename)
    {
	return coremmu.loadBIOS(filename);
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
	    corecpu.dointerrupts();
	    corecpu.executenextopcode();
	    int cycles = corecpu.m_cycles - corecycles;

	    coretimers.updatetimers(cycles);

	    if (!corecpu.stopped)
	    {	        
		coregpu.updategraphics(cycles);
	    }
	}
    }
}
