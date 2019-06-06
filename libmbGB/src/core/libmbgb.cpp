#include "../../include/libmbGB/libmbgb.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
using namespace gb;
using namespace std;

struct stat info;

namespace gb
{
    DMGCore::DMGCore()
    {

    }

    DMGCore::~DMGCore()
    {
	cout << "DMGCore::Shutting down..." << endl;
    }

    void DMGCore::init()
    {
	corecpu.mem = &coremmu;
	coregpu.gmem = &coremmu;
	coretimers.tmem = &coremmu;
	coremmu.joypad = &coreinput;
    coremmu.audio = &coreapu;
	corecpu.timers = &coretimers;
	corecpu.gpu = &coregpu;
	reset();
	cout << "DMGCore::Initialized" << endl;
    }

    void DMGCore::reset()
    {
	resetcpu();
	coremmu.reset();
	coregpu.reset();
	coretimers.reset();
	coreapu.reset();
	coretimers.reset();
    }

    void DMGCore::resetcpu()
    {
	if (coremmu.isgbcenabled)
	{
	    corecpu.reset(true);
	}
	else
	{
	    corecpu.reset(false);
	}
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
	    cout << "--sys-dmg \t\t Plays ROMs in DMG mode." << endl;
	    cout << "--sys-gbc \t\t Plays ROMs in GBC mode." << endl;
	    cout << endl;
	    return false;
	}

        romname = argv[1];

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
            	    biosname = argv[i + 1];
		}
	    }

	    if ((strncmp(argv[i], "--sys-dmg", 9) == 0))
	    {
		coremmu.ismanual = true;
		coremmu.gbtype = 1;
	    }

	    if ((strncmp(argv[i], "--sys-gbc", 9) == 0))
	    {
		coremmu.ismanual = true;
		coremmu.gbtype = 2;
	    }
	}

	return true;
    }
    
    void DMGCore::loadstate(string id)
    {
        paused = true;
        
        string memstate = romname + id + ".mbmem";
        string cpustate = romname + id + ".mbcpu";
        
        if (!coremmu.loadmmu(memstate))
        {
            exit(1);
        }
        
        if (!corecpu.loadcpu(cpustate))
        {
            exit(1);
        }
        
        cout << "Loaded state of " << romname << " from files " << memstate << " and " << cpustate << endl;
        
        paused = false;
        coreapu.reset();
    }
    
    void DMGCore::savestate(string id)
    {
        paused = true;
        
        string memstate = romname + id + ".mbmem";
        string cpustate = romname + id + ".mbcpu";
        
        if (!coremmu.savemmu(memstate))
        {
            exit(1);
        }
        
        if (!corecpu.savecpu(cpustate))
        {
            exit(1);
        }
        
        cout << "Saved state of " << romname << " to files " << memstate << " and " << cpustate << endl;
        paused = false;
    }

    void DMGCore::runcore()
    {
        corecpu.m_cycles = 0;
        int maxcycles = (coremmu.doublespeed) ? 139810 : 69905;
	if (!paused)
	{
	    while (corecpu.m_cycles < maxcycles)
            {
		int corecycles = corecpu.m_cycles;
                corecpu.dointerrupts();
                corecpu.executenextopcode();
                int cycles = corecpu.m_cycles - corecycles;

		if ((coremmu.doublespeed))
		{
		    coregpu.updategraphics(cycles / 2);
		    coreapu.updateaudio((cycles * 0.66));
		}
		else
		{
		    coregpu.updategraphics(cycles);
		    coreapu.updateaudio((cycles * 1.22));
		}

		coretimers.updatetimers(cycles);
            }
	}
    }
}
