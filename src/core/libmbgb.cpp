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

	cout << "DMGCore::Initialized" << endl;
    }

    bool DMGCore::loadROM(string filename)
    {
	streampos size;

	cout << "Loading ROM: " << filename << endl;
	
	ifstream file(filename.c_str(), ios::in | ios::binary | ios::ate);

	if (file.is_open())
	{
	    size = file.tellg();

	    if (size > 0x8000)
	    {
		cout << "MMU::Error - " << filename << " is too big." << endl;
		return false;
	    }

	    file.seekg(0, ios::beg);
	    file.read((char*)&coremmu.memorymap[0], size);
	    file.close();
	    cout << "MMU::" << filename << " succesfully loaded." << endl;
	    return true;
	}
	else
	{
	    cout << "MMU::" << filename << " could not be opened. Check file path or permissions." << endl;
	    return false;
	}
    }

    bool DMGCore::loadBIOS(string filename)
    {
	streampos size;

	cout << "Loading ROM: " << filename << endl;
	
	ifstream file(filename.c_str(), ios::in | ios::binary | ios::ate);

	if (file.is_open())
	{
	    size = file.tellg();

	    if (size > 256)
	    {
		cout << "MMU::Error - BIOS is too big." << endl;
		return false;
	    }

	    file.seekg(0, ios::beg);
	    file.read((char*)&coremmu.bios[0], size);
	    file.close();
	    cout << "BIOS succesfully loaded." << endl;
	    return true;
	}
	else
	{
	    cout << "MMU::BIOS could not be opened. Check file path or permissions." << endl;
	    return false;
	}
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

	    corecpu.dointerrupts();

	    if (!corecpu.stopped)
	    {
		coretimers.updatetimers(cycles);	        
		coregpu.updategraphics(cycles);
	    }
	}
    }
}
