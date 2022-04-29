/*
    This file is part of libmbGB.
    Copyright (C) 2022 BueniaDev.

    libmbGB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libmbGB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "libmbgb.h"
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
	coreinfrared = make_unique<Infrared>(*coremmu);
	coreapu = make_unique<APU>(*coremmu);
	corecpu = make_unique<CPU>(*coremmu, *coregpu, *coretimers, *coreserial, *coreapu, *coreinfrared);
    }

    GBCore::~GBCore()
    {

    }

    void GBCore::init()
    {
	if (isxmas())
	{
	    cout << "Happy holidays from libmbGB!" << endl;
	}

	if (dev == NULL)
	{
	    connectserialdevice(new Disconnected());
	}

	if (devir == NULL || !coremmu->isgbcconsole())
	{
	    connectirdevice(new DisconnectedIR());
	}

	loadconfigaddon();

	coremmu->init();
	corecpu->init();
	coregpu->init();
	coreapu->init();
	coretimers->init();
	coreinput->init();
	coreserial->init();
	loadbackup();

	cout << "mbGB::Initialized" << endl;
    }

    void GBCore::shutdown(bool isreset)
    {
	paused = true;
	saveconfigaddon();

	if (!isreset)
	{
	    dev = NULL;
	    system_type = 0;
	}

	coreserial->shutdown();
	coreinput->shutdown();
	coretimers->shutdown();
	coreapu->shutdown();
	coregpu->shutdown();
	corecpu->shutdown();
	savebackup();
	
	coremmu->shutdown();

	if (front != NULL)
	{
	    if (coremmu->isgbcamera())
	    {
		front->camerashutdown();
	    }

	    front->shutdown();
	}
	
	iscorerunning = false;
	cout << "mbGB::Shutting down..." << endl;
    }

    void GBCore::printusage(char *argv)
    {
	cout << "Usage: " << argv << " ROM [options]" << endl;
	cout << endl;
	cout << "Options:" << endl;
	cout << "-b [FILE], --bios [FILE] \t\t Loads and uses a BIOS file." << endl;
	cout << "--sys-dmg \t\t Plays ROMs in DMG mode." << endl;
	cout << "--sys-gbc \t\t Plays ROMs in GBC mode." << endl;
	cout << "--sys-gba \t\t Plays ROMs in GBA's GBC mode (currently WIP)." << endl;
	cout << "--sys-hybrid \t\t Plays ROMs in hybrid DMG/GBC mode." << endl;
	cout << "--dotrender \t\t Enables the more accurate dot-based renderer." << endl;
	cout << "--mbc1m \t\t Enables the MBC1 multicart mode, if applicable." << endl;
	cout << "--serialdebug \t\t Emulates the custom SerialDebug device (useful for reverse-engineering of Link Cable-based addons)." << endl;
	cout << "--printer \t\t Emulates the Game Boy Printer." << endl;
	cout << "--mobile \t\t Emulates the Mobile Adapter GB (currently WIP)." << endl;
	cout << "--power \t\t Emulates the Power Antenna / Bug Sensor (currently non-functional)." << endl;
	cout << "--turbo-file \t\t Emulates the Turbo File GB (currently WIP)." << endl;
	cout << "--bcb \t\t Emulates the Barcode Boy (DMG only)." << endl;
	cout << "--ir [DEVICE NAME] \t\t Emulates one of the infrared devices shown below (GBC only, currently WIP)." << endl;
	cout << "    dev \t\t Emulates the custom IRDebug device (useful for reverse-engineering of infrared protocols)." << endl;
	cout << "    cca \t\t Emulates an artifical light source (used by Chee Chai Alien, currently WIP)." << endl;
	cout << "    zzh \t\t Emulates the Full Changer (used by Zok Zok Heroes, currently non-functional)." << endl;
	cout << "    tvr \t\t Emulates a TV remote (currently non-functional)." << endl;
	cout << "-h, --help \t\t Displays this help message." << endl;
	cout << endl;
    }

    bool GBCore::biosload()
    {
	return coremmu->biosload;
    }

    bool GBCore::isagbmode()
    {
	return coremmu->isagbconsole();
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

	    if ((strcmp(argv[i], "--ir") == 0))
	    {
		if ((i + 1) == argc)
		{
		    connectirdevice(new DisconnectedIR());
		}
		else
		{
		    char *devname = argv[i + 1];

		    if ((strcmp(devname, "dev") == 0))
		    {
			connectirdevice(new InfraredDebug());
		    }
		    else if ((strcmp(devname, "cca") == 0))
		    {
			connectirdevice(new CheeChaiLight());
		    }
		    else if ((strcmp(devname, "zzh") == 0))
		    {
			connectirdevice(new DisconnectedIR());
		    }
		    else if ((strcmp(devname, "tvr") == 0))
		    {
			connectirdevice(new DisconnectedIR());
		    }
		}
	    }

	    if ((strcmp(argv[i], "--sys-dmg") == 0))
	    {
		// Set system type to DMG
		setsystemtype(1);

		if (biosname == "")
		{
		    coremmu->biosload = true;
		    biosname = "dmg_bios.bin";
		}
	    }

	    if ((strcmp(argv[i], "--sys-gbc") == 0))
	    {
		// Set system type to CGB
		setsystemtype(2);

		if (biosname == "")
		{
		    coremmu->biosload = true;
		    biosname = "cgb_bios.bin";
		}
	    }

	    if ((strcmp(argv[i], "--sys-gba") == 0))
	    {
		// Set system type to AGB
		setsystemtype(3);

		if (biosname == "")
		{
		    coremmu->biosload = true;
		    biosname = "agb_bios.bin";
		}
	    }

	    if ((strcmp(argv[i], "--sys-hybrid") == 0))
	    {
		coremmu->hybrid = true;

		if (biosname == "")
		{
		    coremmu->biosload = true;
		    biosname = "cgb_bios.bin";
		}
	    }

	    if ((strcmp(argv[i], "--dotrender") == 0))
	    {
		setdotrender(true);
	    }
	    else
	    {
		setdotrender(false);
	    }

	    if ((strcmp(argv[i], "--mbc1m") == 0))
	    {
		coremmu->ismulticart = true;
	    }
	    else
	    {
		coremmu->ismulticart = false;
	    }

	    if ((strcmp(argv[i], "--serialdebug") == 0))
	    {
		connectserialdevice(new SerialDebug());
	    }

	    if ((strcmp(argv[i], "--printer") == 0))
	    {
		connectserialdevice(new GBPrinter());
	    }
		
	    if ((strcmp(argv[i], "--mobile") == 0))
	    {
		connectserialdevice(new MobileAdapterGB());
	    }
	    
	    if ((strcmp(argv[i], "--power") == 0))
	    {
		connectserialdevice(new Disconnected());
	    }

	    if ((strcmp(argv[i], "--turbo-file") == 0))
	    {
		connectserialdevice(new TurboFileGB());
	    }
	    
	    if ((strcmp(argv[i], "--bcb") == 0))
	    {
		connectserialdevice(new BarcodeBoy());
	    }
	}

	return true;
    }

    void GBCore::setsystemtype(int index)
    {
	system_type = index;
	switch (index)
	{
	    case 0: coremmu->gameboy = Console::Default; break;
	    case 1: coremmu->gameboy = Console::DMG; break;
	    case 2: coremmu->gameboy = Console::CGB; break;
	    case 3: coremmu->gameboy = Console::AGB; break;
	    default: coremmu->gameboy = Console::Default; break;
	}
    }

    void GBCore::connectserialdevice(int index)
    {
	switch (index)
	{
	    case 0: connectserialdevice(new Disconnected()); break;
	    case 1: connectserialdevice(new GBPrinter()); break;
	    case 2: connectserialdevice(new MobileAdapterGB()); break;
	    case 3: connectserialdevice(new BarcodeBoy()); break;
	    case 4: connectserialdevice(new TurboFileGB()); break;
	    default: connectserialdevice(new Disconnected()); break;
	}
    }

    void GBCore::setaddonfilename(string filename)
    {
	if (dev != NULL)
	{
	    dev->setsavefilename(filename);
	}
    }
    
    void GBCore::connectserialdevice(SerialDevice *cb)
    {
	// Sanity check to prevent possible buffer overflow
	// with NULL serial device pointer
	if (cb == NULL)
	{
	    cb = new Disconnected();
	}

    	dev = NULL;
	cout << "Connecting addon of " << cb->getaddonname() << endl;
	auto serialcb = bind(&Serial::recieve, &*coreserial, _1);
	cb->setlinkcallback(serialcb);
	dev = cb;
	setprintercallback();
	coreserial->setserialdevice(dev);
    }

    void GBCore::connectirdevice(InfraredDevice *cb)
    {
	// Sanity check to prevent possible buffer overflow
	// with NULL IR device pointer
	if (cb == NULL)
	{
	    cb = new DisconnectedIR();
	}

    	devir = NULL;
	cout << "Connecting IR device of " << cb->getdevicename() << endl;
	devir = cb;
	coreinfrared->setirdevice(devir);
    }

    void GBCore::setfrontend(mbGBFrontend *cb)
    {
	front = cb;

	if (front != NULL)
	{
	    setaudiocallback(bind(&mbGBFrontend::audiocallback, cb, _1, _2));
	    setrumblecallback(bind(&mbGBFrontend::rumblecallback, cb, _1));
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

    bool GBCore::loadconfigaddon()
    {
	if (dev == NULL)
	{
	    return true;
	}

	if (dev->getsavefilename() != "")
	{
	    return dev->loadfile(front->loadfile(dev->getsavefilename()));
	}
	
	return true;
    }

    bool GBCore::saveconfigaddon()
    {
	if (dev == NULL)
	{
	    return true;
	}

	if (dev->getsavefilename() != "")
	{
	    return front->savefile(dev->getsavefilename(), dev->getsavefiledata());
	}
	
	return true;
    }

    bool GBCore::loadbackup()
    {
	stringstream saveram;

	saveram << romname << "." << (save_extension.empty() ? "sav" : save_extension);

	if (front != NULL)
	{
	    return coremmu->loadbackup(front->loadfile(saveram.str()));
	}
	
	return true;
    }

    bool GBCore::savebackup()
    {
	stringstream saveram;

	saveram << romname << "." << (save_extension.empty() ? "sav" : save_extension);

	if (front != NULL)
	{
	    return front->savefile(saveram.str(), coremmu->savebackup());
	}
	
	return true;
    }

    void GBCore::set_save_extension(string ext)
    {
	save_extension = ext;
    }

    size_t GBCore::getstatesize()
    {
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
	// Sanity check to avoid crash caused by out-of-bounds vector accesses
	if (((x < 0) || (x >= screenwidth)) || ((y < 0) || (y >= screenheight)))
	{
	    return {0, 0, 0};
	}

	return coregpu->framebuffer.at(x + (y * screenwidth));
    }

    vector<gbRGB> GBCore::getframebuffer()
    {
	return coregpu->framebuffer;
    }

    void GBCore::keychanged(gbButton button, bool is_pressed)
    {
	coreinput->keychanged(button, is_pressed);
    }

    void GBCore::keypressed(gbButton button)
    {
	keychanged(button, true);
    }

    void GBCore::keyreleased(gbButton button)
    {
	keychanged(button, false);
    }

    vector<uint8_t> GBCore::dumpvram()
    {
	return coremmu->vram;
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

    int GBCore::get_total_cycles()
    {
	return (70224 << coremmu->doublespeed);
    }

    void GBCore::set_accel_values(float x, float y)
    {
	coremmu->set_accel_values(x, y);
    }

    void GBCore::runcore()
    {
	while (totalcycles < get_total_cycles())
	{
	    totalcycles += runinstruction();
	}

	totalcycles = 0;
    }

    bool GBCore::initcore()
    {
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

	cout << "Screen size: " << dec << int(screenwidth) << "x" << dec << int(screenheight) << endl;

	init();

	if (front != NULL)
	{
	    front->init();

	    if (coremmu->isgbcamera())
	    {
		front->camerainit();
	    }
	}
	
	iscorerunning = true;
	paused = false;
	return true;
    }

    bool GBCore::isrunning()
    {
	return iscorerunning;
    }

    void GBCore::setsamplerate(int val)
    {
	coreapu->setsamplerate(val);
    }

    void GBCore::setrumblecallback(rumblefunc cb)
    {
	coremmu->setrumblecallback(cb);
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

    void GBCore::addcheats(vector<string> cheats)
    {
	coremmu->add_cheats(cheats);
    }

    void GBCore::resetcore()
    {
	cout << "Resetting GBCore..." << endl;

	// Shut down core components without
	// resetting frontend or attached devices
	shutdown(true);
	// Override emulated system if manually set
	setsystemtype(system_type);
	initcore();
    }

    void GBCore::debugoutput()
    {
	corecpu->printregs();
    }
};
