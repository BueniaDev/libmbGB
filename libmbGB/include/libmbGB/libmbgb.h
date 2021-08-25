/*
    This file is part of libmbGB.
    Copyright (C) 2021 BueniaDev.

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

#ifndef LIBMBGB_CORE
#define LIBMBGB_CORE

#include <cstdint>
#include <memory>
#include <vector>
#include <utility>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <array>
#include <mutex>
#include <thread>
#include <ctime>
#include "utils.h"
#include "enums.h"
#include "mmu.h"
#include "serial.h"
#include "infrared.h"
#include "cpu.h"
#include "gpu.h"
#include "addons.h"
#include "input.h"
#include "timers.h"
#include "libmbgb_api.h"
using namespace gb;
using namespace std;
using namespace std::placeholders;

namespace gb
{

    class LIBMBGB_API GBCore
    {
	public:
	    GBCore();
	    ~GBCore();

	    unique_ptr<MMU> coremmu;
	    unique_ptr<GPU> coregpu;
	    unique_ptr<Timers> coretimers;
	    unique_ptr<Input> coreinput;
	    unique_ptr<CPU> corecpu;
	    unique_ptr<Serial> coreserial;
	    unique_ptr<Infrared> coreinfrared;
	    unique_ptr<APU> coreapu;

	    vector<GBCore*> gbcores;
	    
	    mbGBFrontend *front = NULL;

	    void preinit();
	    void init();
	    void shutdown(bool isreset = false);

	    void addcore(string romfile);

	    bool getoptions(int argc, char* argv[]);
	    bool loadBIOS(string filename);
	    bool loadROM(string filename);
	    size_t getstatesize();
	    bool loadstate();
	    bool savestate();
	    void dosavestate(mbGBSavestate &file);
	    gbRGB getpixel(int x, int y);
	    vector<gbRGB> getframebuffer();
	    void printusage(char *argv);
	    void keypressed(gbButton button);
	    void keyreleased(gbButton button);
	    void sensorpressed(gbGyro pos);
	    void sensorreleased(gbGyro pos);
	    vector<uint8_t> dumpvram();
	    void setaddonfilename(string filename);
	    void setdotrender(bool val);

	    void setfrontend(mbGBFrontend *front);

	    bool loadbackup();
	    bool savebackup();

	    bool loadconfigaddon();
	    bool saveconfigaddon();

	    bool isrunning();
	    
	    bool paused = false;

	    int overspentcycles = 0;
	    int runinstruction();
	    void update(int steps);
	    void runcore();
	    void runapp();
	    bool islinkactive();
	    bool initcore();
	    void resetcore();
	    void setsamplerate(int val);
	    void setaudiocallback(apuoutputfunc cb);
	    void setrumblecallback(rumblefunc cb);
	    void setpixelcallback(pixelfunc cb);
	    void setcamcallbacks(caminitfunc icb, camstopfunc scb, camframefunc fcb);
	    void setprintercallback();
	    void setprintcallback(printfunc cb);
	    bool isprinterenabled = false;
	    bool ismobileenabled = false;
	    bool ispowerenabled = false;
	    bool isbcbenabled = false;

	    inline bool isxmas()
	    {
		time_t t = time(NULL);
		tm* timeptr = localtime(&t);

		return (timeptr->tm_mon == 11);
	    }

	    string romname;
	    string biosname;

	    bool biosload();
	    bool isagbmode();

	    int screenwidth = 0;
	    int screenheight = 0;

	    int totalcycles = 0;
	    
	    void swipebarcode()
	    {
		if (dev != NULL)
		{
	    	    dev->swipebarcode();
		}
	    }
	    
	    SerialDevice *dev = NULL;
	    InfraredDevice *devir = NULL;
	    
	    void setsystemtype(int index);

	    void connectserialdevice(int index);

	    void connectserialdevice(SerialDevice *cb);
	    void connectirdevice(InfraredDevice *cb);
	private:
	    bool iscorerunning = false;
    };
};

#endif // LIBMBGB_CORE
