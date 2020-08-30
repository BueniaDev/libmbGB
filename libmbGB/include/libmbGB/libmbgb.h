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
#include <ctime>
#include "utils.h"
#include "enums.h"
#include "mmu.h"
#include "serial.h"
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
    class LIBMBGB_API mbGBFrontend
    {
	public:
    	    mbGBFrontend()
    	    {
    	
    	    }
    	
    	    ~mbGBFrontend()
    	    {
    	
    	    }
    	
    	    virtual bool init() = 0;
    	    virtual void shutdown() = 0;
    	    virtual void runapp() = 0;
    	    virtual void audiocallback(audiotype left, audiotype right) = 0;
    	    virtual void rumblecallback(bool enabled) = 0;
    	    virtual void sensorcallback(uint16_t& sensorx, uint16_t& sensory) = 0;
    	    virtual void pixelcallback() = 0;
            virtual vector<uint8_t> loadfile(string filename, const void *data=NULL, int size=0) = 0;
    };

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
	    unique_ptr<APU> coreapu;
	    
	    mbGBFrontend *front = NULL;

	    void preinit();
	    void init();
	    void shutdown();
	    void shutdown(bool frontend);

	    bool getoptions(int argc, char* argv[]);
	    bool loadBIOS(string filename);
	    bool loadROM(string filename);
	    bool loadstate();
	    bool savestate();
	    RGB getpixel(int x, int y);
	    void printusage(char *argv);
	    void keypressed(Button button);
	    void keyreleased(Button button);
	    bool dumpvram(string filename);
	    bool dumpmemory(string filename);
	    void setdotrender(bool val);

	    void setfrontend(mbGBFrontend *front);

	    bool loadbackup();
	    bool savebackup();
	    bool paused = false;

	    int overspentcycles = 0;
	    int runinstruction();
	    void runcore();
	    void runapp();
	    bool initcore();
	    void resetcore();
	    void resetcoreretro();
	    void setsamplerate(int val);
	    void setaudiocallback(apuoutputfunc cb);
	    void setrumblecallback(rumblefunc cb);
	    void setsensorcallback(sensorfunc cb);
	    void setpixelcallback(pixelfunc cb);
	    void setaudioflags(int val);
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

	    int screenwidth = 0;
	    int screenheight = 0;

	    int totalcycles = 0;

	    inline bool isagbmode()
	    {
		return (coremmu->agbmode);
	    }
	    
	    void swipebarcode()
	    {
	    	dev->swipebarcode();
	    }
	    
	    SerialDevice *dev = NULL;
	    
	    void connectserialdevice(SerialDevice *cb);
    };
};

#endif // LIBMBGB_CORE
