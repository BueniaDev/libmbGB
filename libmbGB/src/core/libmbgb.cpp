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
	coremmu->resetio();
	coremmu->initio();	
	corecpu->init();
	coregpu->init();
	coretimers->init();
	coreinput->init();
	coreserial->init();
	loadbackup();
	cout << "mbGB::Initialized" << endl;
    }

    void GBCore::shutdown()
    {
	coreserial->shutdown();
	coreinput->shutdown();
	coretimers->shutdown();
	coregpu->shutdown();
	corecpu->shutdown();
	savebackup();
	coremmu->shutdown();
	cout << "mbGB::Shutting down..." << endl;
    }

    void GBCore::printusage(char *argv)
    {
	cout << "Usage: " << argv << " ROM [options]" << endl;
	cout << endl;
	cout << "Options:" << endl;
	cout << "-b [FILE], --bios [FILE] \t\t Loads and uses a BIOS file." << endl;
	cout << "--sys-dmg \t\t Plays ROMs in DMG mode." << endl;
	cout << "--sys-gbc \t\t Plays ROMs in GBC mode (HUGE WIP)." << endl;
	cout << "--sys-hybrid \t\t Plays ROMs in hybrid DMG/GBC mode (HUGE WIP)." << endl;
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

	    if ((strncmp(argv[i], "--sys-dmg", 9) == 0))
	    {
		coremmu->ismanual = true;
		coremmu->gameboy = Console::DMG;
		coremmu->gbmode = Mode::DMG;
	    }

	    if ((strncmp(argv[i], "--sys-gbc", 9) == 0))
	    {
		coremmu->ismanual = true;
		coremmu->gameboy = Console::CGB;
		coremmu->gbmode = Mode::CGB;
	    }

	    if ((strncmp(argv[i], "--sys-hybrid", 12) == 0))
	    {
		coremmu->hybrid = true;
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

    bool GBCore::loadbackup()
    {
	stringstream saveram;

	saveram << romname << ".sav";

	return coremmu->loadbackup(saveram.str());
    }

    bool GBCore::savebackup()
    {
	stringstream saveram;

	saveram << romname << ".sav";

	return coremmu->savebackup(saveram.str());
    }

    bool GBCore::loadstate()
    {
	stringstream savestate;

	savestate << romname << ".mbsave";

	string savename = savestate.str();

	bool ret = false;

	if (!corecpu->loadcpu(savename))
	{
	    cout << "mbGB::Save state could not be loaded." << endl;
	    ret = false;
	}

	int offs = corecpu->cpusize();

	if (!coremmu->loadmmu(offs, savename))
	{
	    cout << "mbGB::Save state could not be loaded." << endl;
	    ret = false;
	}

	cout << "mbGB::Save state succesfully loaded." << endl;
	ret = true;
	
	return ret;
    }

    bool GBCore::savestate()
    {
	stringstream savestate;

	savestate << romname << ".mbsave";

	string savename = savestate.str();

	cout << savename << endl;

	bool ret = false;

	if (!corecpu->savecpu(savename))
	{
	    cout << "mbGB::CPU save state could not be written." << endl;
	    ret = false;
	}

	if (!coremmu->savemmu(savename))
	{
	    cout << "mbGB::MMU save state could not be written." << endl;
	    ret = false;
	}

	cout << "mbGB::Save state succesfully written." << endl;
	ret = true;
	
	return ret;
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

    bool GBCore::dumpvram(string filename)
    {
	fstream file(filename.c_str(), ios::out | ios::binary);

	if (file.is_open())
	{
	    file.seekp(0, ios::beg);
	    file.write((char*)&coremmu->vram[0], 0x4000);
	    file.close();
	    cout << "mbGB::VRAM dumped" << endl;
	    return true;
	}
	else
	{
	    cout << "mbGB::Error - VRAM could not be dumped" << endl;
	    return false;
	}
    }

    bool GBCore::dumpmemory(string filename)
    {
	fstream file(filename.c_str(), ios::out | ios::binary);

	uint8_t memorymap[0x10000];

	for (int i = 0; i < 0x10000; i++)
	{
	    memorymap[i] = coremmu->readByte(i);
	}

	if (file.is_open())
	{
	    file.seekp(0, ios::beg);
	    file.write((char*)&memorymap[0], 0x10000);
	    file.close();
	    cout << "mbGB::Memory dumped" << endl;
	    return true;
	}
	else
	{
	    cout << "mbGB::Error - Memory could not be dumped" << endl;
	    return false;
	}
    }

    void GBCore::runcore()
    {
	overspentcycles = corecpu->runfor((70224 << coremmu->doublespeed) + overspentcycles);
    }

};
