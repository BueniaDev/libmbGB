// This file is part of libmbGB.
// Copyright (C) 2019 Buenia.
//
// libmbGB is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// libmbGB is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.

#include "../../include/libmbGB/libmbgb.h"
using namespace gb;
using namespace std;

namespace gb
{
    GBCore::GBCore()
    {
	coremmu = make_unique<MMU>();
	coregpu = make_unique<GPU>(*coremmu);
	coretimers = make_unique<Timers>(*coremmu);
	coreinput = make_unique<Input>(*coremmu);
	coreserial = make_unique<Serial>(*coremmu);
	corecpu = make_unique<CPU>(*coremmu, *coregpu, *coretimers, *coreserial);
    }

    GBCore::~GBCore()
    {

    }

    void GBCore::preinit()
    {
	coremmu->init();
    }

    void GBCore::init()
    {
	corecpu->init();
	coregpu->init();
	coretimers->init();
	coreinput->init();
	coreserial->init();
	cout << "mbGB::Initialized" << endl;
    }

    void GBCore::shutdown()
    {
	coreserial->shutdown();
	coreinput->shutdown();
	coretimers->shutdown();
	coregpu->shutdown();
	corecpu->shutdown();
	coremmu->shutdown();
	cout << "mbGB::Shutting down..." << endl;
    }

    void GBCore::printusage(char *argv)
    {
	cout << "Usage: " << argv << " ROM [options]" << endl;
	cout << endl;
	cout << "Options:" << endl;
	cout << "-b [FILE], --bios [FILE] \t\t Loads and uses a BIOS file." << endl;
	cout << "-h, --help \t\t Displays this help message." << endl;
	cout << endl;
    }

    bool GBCore::biosload()
    {
	return coremmu->biosload;
    }

    bool GBCore::getoptions(int argc, char* argv[])
    {
	if (argc < 2)
	{
	    printusage(argv[0]);
	    return false;
	}

	for (int i = 1; i < argc; i++)
	{
	    if ((strncmp(argv[i], "-h", 2) == 0) || (strncmp(argv[i], "--help", 6) == 0))
	    {
		printusage(argv[0]);
		return false;
	    }
	}

	romname = argv[1];

	for (int i = 2; i < argc; i++)
	{
	    if ((strncmp(argv[i], "-b", 2) == 0) || (strncmp(argv[i], "--bios", 6) == 0))
	    {
		if ((i + 1) == argc)
		{
		    cout << "Error - No BIOS file in arguments" << endl;
		    return false;
		}
		else
		{
		    coremmu->biosload = true;
		    biosname = argv[i + 1];
		}
	    }
	}

	return true;
    }

    bool GBCore::loadBIOS(string filename)
    {
	return coremmu->loadBIOS(filename);
    }

    bool GBCore::loadROM(string filename)
    {
	return coremmu->loadROM(filename);
    }

    RGB GBCore::getpixel(int x, int y)
    {
	return coregpu->framebuffer[x + (y * 160)];
    }

    void GBCore::keypressed(Button button)
    {
	coreinput->keypressed(button);
    }

    void GBCore::keyreleased(Button button)
    {
	coreinput->keyreleased(button);
    }

    void GBCore::runcore()
    {
	overspentcycles = corecpu->runfor(70224 + overspentcycles);
    }

};