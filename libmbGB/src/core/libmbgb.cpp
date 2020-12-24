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
	if (dev == NULL)
	{
	    connectserialdevice(new Disconnected());
	}

	coremmu->resetio();

	if (!coremmu->biosload)
	{
	    coremmu->initio();
	}

	corecpu->init();
	coregpu->init();
	coreapu->init();
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
	dev = NULL;

	coreserial->shutdown();
	coreinput->shutdown();
	coretimers->shutdown();
	coreapu->shutdown();
	coregpu->shutdown();
	corecpu->shutdown();
	savebackup();
	
	coremmu->shutdown();
	
	if (frontend && front != NULL)
	{
	    if (coremmu->isgbcamera())
	    {
		front->camerashutdown();
	    }

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
	cout << "--sys-gba \t\t Plays ROMs in GBA mode. (GB/GBC ROMs only, WIP)." << endl;
	cout << "--sys-hybrid \t\t Plays ROMs in hybrid DMG/GBC mode. (GB/GBC ROMs only)." << endl;
	cout << "--dotrender \t\t Enables the more accurate dot-based renderer." << endl;
	cout << "--accurate-colors \t\t Improves the accuracy of the displayed colors (GBC only)." << endl;
	cout << "--mbc1m \t\t Enables the MBC1 multicart mode, if applicable." << endl;
	cout << "--printer \t\t Emulates the Game Boy Printer (currently WIP)." << endl;
	cout << "--mobile \t\t Emulates the Mobile Adapter GB (currently non-functional)." << endl;
	cout << "--power \t\t Emulates the Power Antenna / Bug Sensor (currently non-functional)." << endl;
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
		connectserialdevice(new GBPrinter());
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

	return true;
    }
    
    void GBCore::connectserialdevice(SerialDevice *cb)
    {
    	dev = NULL;
	cout << "Connecting addon of " << cb->getaddonname() << endl;
	auto serialcb = bind(&Serial::recieve, &*coreserial, _1);
	cb->setlinkcallback(serialcb);
	dev = cb;
	setprintercallback();
	coreserial->setserialdevice(dev);
    }
    
    void GBCore::setfrontend(mbGBFrontend *cb)
    {
        front = cb;
        
	cout << "Setting frontend..." << endl;
        if (front != NULL)
        {
            setaudiocallback(bind(&mbGBFrontend::audiocallback, cb, _1, _2));
            setrumblecallback(bind(&mbGBFrontend::rumblecallback, cb, _1));
            setsensorcallback(bind(&mbGBFrontend::sensorcallback, cb, _1, _2));
            setpixelcallback(bind(&mbGBFrontend::pixelcallback, cb));
	    auto icb = bind(&mbGBFrontend::camerainit, cb);
	    auto scb = bind(&mbGBFrontend::camerashutdown, cb);
	    auto fcb = bind(&mbGBFrontend::cameraframe, cb, _1);
	    setcamcallbacks(icb, scb, fcb);
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

	if (front != NULL)
	{
	    return coremmu->loadbackup(front->loadfile(saveram.str()));
	}
	
	return true;
    }

    bool GBCore::savebackup()
    {
	stringstream saveram;

	saveram << romname << ".sav";

	if (front != NULL)
	{
	    return front->savefile(saveram.str(), coremmu->savebackup());
	}
	
	return true;
    }

    size_t GBCore::getstatesize()
    {
	cout << "Fetching savestate size..." << endl;
	void *data = malloc((16 * 1024 * 1024));
	VecFile file = vfopen(data, (16 * 1024 * 1024));

	mbGBSavestate savestate(file, true);
	dosavestate(savestate);

	size_t size = savestate.state_file.loc_pos;

	free(data);
	return size;
    }

    void GBCore::dosavestate(mbGBSavestate &file)
    {
	corecpu->dosavestate(file);
	coremmu->dosavestate(file);
	coregpu->dosavestate(file);	
	coreapu->dosavestate(file);
	coretimers->dosavestate(file);
    }

    bool GBCore::loadstate()
    {
	cout << "Loading savestate..." << endl;
	if (front != NULL)
	{
	    stringstream str_name;
	    str_name << romname << ".mbsave";

	    vector<uint8_t> temp = front->loadfile(str_name.str());

	    if (temp.empty())
	    {
		cout << "Error loading savestate." << endl;
		return false;
	    }

	    VecFile file = vfopen(temp.data(), temp.size());

	    mbGBSavestate savestate(file, false);
	    dosavestate(savestate);
	    vfclose(file);
	}

	return true;
    }

    bool GBCore::savestate()
    {
	cout << "Saving savestate..." << endl;
	if (front != NULL)
	{
	    vector<uint8_t> temp(getstatesize(), 0);
	    VecFile file = vfopen(temp.data(), temp.size());

	    mbGBSavestate savestate(file, true);

	    stringstream str_name;
	    str_name << romname << ".mbsave";

	    dosavestate(savestate);

	    bool ret = front->savefile(str_name.str(), savestate.get_savestate_file().data);
	    vfclose(file);
	    return ret;
	}

	return true;
    }

    gbRGB GBCore::getpixel(int x, int y)
    {
	return coregpu->framebuffer[x + (y * 160)];
    }

    array<gbRGB, (160 * 144)> GBCore::getframebuffer()
    {
	return coregpu->framebuffer;
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

    bool GBCore::islinkactive()
    {
	return coreserial->pendingrecieve;
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

    void GBCore::update(int steps)
    {
	while (steps--)
	{
	    runinstruction();
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

	    if (coremmu->isgbcamera())
	    {
		return front->camerainit();
	    }
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

    void GBCore::setcamcallbacks(caminitfunc icb, camstopfunc scb, camframefunc fcb)
    {
	coremmu->setcamcallbacks(icb, scb, fcb);
    }

    void GBCore::setprintercallback()
    {
	auto printcb = bind(&mbGBFrontend::printerframe, front, _1, _2);
	setprintcallback(printcb);
    }

    void GBCore::setprintcallback(printfunc cb)
    {
	if (dev != NULL)
	{
	    dev->setprintcallback(cb);
	}
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
