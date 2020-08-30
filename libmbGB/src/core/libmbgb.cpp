// This file is part of libmbGB.
// Copyright (C) 2020 Buenia.
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
	coreapu = make_unique<APU>(*coremmu);
	corecpu = make_unique<CPU>(*coremmu, *coregpu, *coretimers, *coreserial, *coreapu);
    }

    GBCore::~GBCore()
    {

    }

    void GBCore::preinit()
    {
	if (isxmas())
	{
	    cout << "Happy holidays from libmbGB!" << endl;
	}

	coremmu->init();
    }

    void GBCore::init()
    {
	coremmu->resetio();

	if (!coremmu->biosload)
	{
	    coremmu->initio();
	}

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
        shutdown(true);
    }

    void GBCore::shutdown(bool frontend)
    {
	coreserial->shutdown();
	coreinput->shutdown();
	coretimers->shutdown();
	coregpu->shutdown();
	corecpu->shutdown();
	savebackup();
	
	coremmu->shutdown();
	
	if (frontend && front != NULL)
	{
	    front->shutdown();
	}
	
	cout << "mbGB::Shutting down..." << endl;
    }

    void GBCore::printusage(char *argv)
    {
	cout << "Usage: " << argv << " ROM [options]" << endl;
	cout << endl;
	cout << "Options:" << endl;
	cout << "-b [FILE], --bios [FILE] \t\t Loads and uses a BIOS file." << endl;
	cout << "--sys-dmg \t\t Plays ROMs in DMG mode (GB/GBC ROMs only)." << endl;
	cout << "--sys-gbc \t\t Plays ROMs in GBC mode (GB/GBC ROMs only)." << endl;
	cout << "--sys-gba \t\t Plays ROMs in GBA mode." << endl;
	cout << "--sys-hybrid \t\t Plays ROMs in hybrid DMG/GBC mode. (GB/GBC ROMs only)." << endl;
	cout << "--dotrender \t\t Enables the more accurate dot-based renderer." << endl;
	cout << "--accurate-colors \t\t Improves the accuracy of the displayed colors (GBC only)." << endl;
	cout << "--mbc1m \t\t Enables the MBC1 multicart mode, if applicable." << endl;
	cout << "--printer \t\t Emulates the Game Boy Printer." << endl;
	cout << "--mobile \t\t Emulates the Mobile Adapter GB (currently WIP)." << endl;
	cout << "--power \t\t Emulates the Power Antenna / Bug Sensor." << endl;
	cout << "--bcb \t\t Emulates the Barcode Boy (DMG only)." << endl;
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
	    if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
	    {
		printusage(argv[0]);
		return false;
	    }
	}

	romname = argv[1];

	for (int i = 2; i < argc; i++)
	{
	    if ((strcmp(argv[i], "-b") == 0) || (strcmp(argv[i], "--bios") == 0))
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

	    if ((strcmp(argv[i], "--sys-dmg") == 0))
	    {
		coremmu->gameboy = Console::DMG;
	    }

	    if ((strcmp(argv[i], "--sys-gbc") == 0))
	    {
		coremmu->gameboy = Console::CGB;
	    }

	    if ((strcmp(argv[i], "--sys-gba") == 0))
	    {
		coremmu->agbmode = true;
		coremmu->gameboy = Console::AGB;
	    }

	    if ((strcmp(argv[i], "--sys-hybrid") == 0))
	    {
		coremmu->hybrid = true;
	    }

	    if ((strcmp(argv[i], "--dotrender") == 0))
	    {
		setdotrender(true);
	    }
	    else
	    {
		setdotrender(false);
	    }

	    if ((strcmp(argv[i], "--accurate-colors") == 0))
	    {
		coregpu->accuratecolors = true;
	    }
	    else
	    {
		coregpu->accuratecolors = false;
	    }

	    if ((strcmp(argv[i], "--mbc1m") == 0))
	    {
		coremmu->ismulticart = true;
	    }
	    else
	    {
		coremmu->ismulticart = false;
	    }

	    if ((strcmp(argv[i], "--printer") == 0))
	    {
		connectserialdevice(new Disconnected());
	    }
		
	    if ((strcmp(argv[i], "--mobile") == 0))
	    {
		connectserialdevice(new Disconnected());
	    }
	    
	    if ((strcmp(argv[i], "--power") == 0))
	    {
		connectserialdevice(new Disconnected());
	    }
	    
	    if ((strcmp(argv[i], "--bcb") == 0))
	    {
		connectserialdevice(new BarcodeBoy());
	    }
	}

	if (!isagbmode())
	{
	    screenwidth = 160;
	    screenheight = 144;
	}
	else
	{
	    screenwidth = 240;
	    screenheight = 160;
	}

	if (dev == NULL)
	{
	    connectserialdevice(new Disconnected());
	}

	return true;
    }
    
    void GBCore::connectserialdevice(SerialDevice *cb)
    {
    	dev = NULL;
	auto serial = bind(&Serial::recieve, &*coreserial, _1);
	cb->setlinkcallback(serial);
	dev = cb;
	coreserial->setserialdevice(dev);
    }
    
    void GBCore::setfrontend(mbGBFrontend *cb)
    {
        front = cb;
        
        if (front != NULL)
        {
            setaudiocallback(bind(&mbGBFrontend::audiocallback, cb, _1, _2));
            setrumblecallback(bind(&mbGBFrontend::rumblecallback, cb, _1));
            setsensorcallback(bind(&mbGBFrontend::sensorcallback, cb, _1, _2));
            setpixelcallback(bind(&mbGBFrontend::pixelcallback, cb));
        }
    }

    bool GBCore::loadBIOS(string filename)
    {
	cout << "mbGB::Loading BIOS..." << endl;
        if (front != NULL)
        {
	    return coremmu->loadBIOS(front->loadfile(filename));
	}

	return false;
    }

    bool GBCore::loadROM(string filename)
    {
	cout << "mbGB::Loading ROM " << filename << "..." << endl;
        if (front != NULL)
        {
	    return coremmu->loadROM(front->loadfile(filename));
	}

	return false;
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
	paused = true;

	stringstream savestate;

	savestate << romname << ".mbsave";

	string savename = savestate.str();

	bool ret = false;

	if (!corecpu->loadcpu(savename))
	{
	    cout << "mbGB::Save state could not be loaded." << endl;
	    return false;
	}

	int offs = corecpu->cpusize();

	if (!coremmu->loadmmu(offs, savename))
	{
	    cout << "mbGB::Save state could not be loaded." << endl;
	    return false;
	}

	cout << "mbGB::Save state succesfully loaded." << endl;
	ret = true;
	paused = false;
	return ret;
    }

    bool GBCore::savestate()
    {
	paused = true;
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
	paused = false;
	
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

    void GBCore::setdotrender(bool val)
    {
	coregpu->setdotrender(val);
    }
	
    int GBCore::runinstruction()
    {
	return corecpu->runinstruction();
    }
    
    void GBCore::runapp()
    {
        if (front != NULL)
        {
            front->runapp();
        }
    }

    void GBCore::runcore()
    {
	while (totalcycles < (70224 << coremmu->doublespeed))
	{
	    totalcycles += runinstruction();
	}
	
	totalcycles = 0;
    }

    bool GBCore::initcore()
    {
	preinit();

	if (biosload())
	{
	    if (!loadBIOS(biosname))
	    {
		return false;
	    }
	}
	

	if (!loadROM(romname))
	{
	    return false;
	}

	init();
	
	if (front != NULL)
	{
	    front->init();
	}
	
	return true;
    }

    void GBCore::setsamplerate(int val)
    {
	coreapu->setsamplerate(val);
    }

    void GBCore::setrumblecallback(rumblefunc cb)
    {
	coremmu->setrumblecallback(cb);
    }

    void GBCore::setsensorcallback(sensorfunc cb)
    {
	coremmu->setsensorcallback(cb);
    }

    void GBCore::setaudiocallback(apuoutputfunc cb)
    {
	coreapu->setaudiocallback(cb);
    }
    
    void GBCore::setpixelcallback(pixelfunc cb)
    {
	coregpu->setpixelcallback(cb);
    }
    
    void GBCore::setaudioflags(int val)
    {
        coreapu->setaudioflags(val);
    }

    void GBCore::resetcore()
    {
	shutdown();
	initcore();
    }

    void GBCore::resetcoreretro()
    {
	shutdown();
	preinit();
	init();
    }
};
