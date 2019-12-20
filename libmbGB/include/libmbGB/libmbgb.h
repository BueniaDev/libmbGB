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
#include "enums.h"
#include "mmu.h"
#include "serial.h"
#include "cpu.h"
#include "gpu.h"
#include "addons.h"
#include "input.h"
#include "timers.h"
#include "libmbgb_api.h"
using namespace std;

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
	    unique_ptr<APU> coreapu;

	    void preinit();
	    void init();
	    void shutdown();

	    bool getoptions(int argc, char* argv[]);
	    bool loadBIOS(string filename);
	    bool loadROM(string filename);
	    bool loadROM(const char *filename, const uint8_t* buffer, int size);
	    bool loadstate();
	    bool savestate();
	    RGB getpixel(int x, int y);
	    void printusage(char *argv);
	    void keypressed(Button button);
	    void keyreleased(Button button);
	    bool dumpvram(string filename);
	    bool dumpmemory(string filename);
	    void setdotrender(bool val);

	    bool loadbackup();
	    bool savebackup();
	    bool paused = false;

	    int overspentcycles = 0;
		int runinstruction();
	    void runcore();
	    bool initcore();
	    bool initcore(const char *filename, const uint8_t* buffer, int size);
	    void resetcore();
	    void resetcoreretro();
	    void setsamplerate(int val);
	    void setaudiocallback(apuoutputfunc cb);
	    void setrumblecallback(rumblefunc cb);
	    bool isprinterenabled = false;
		bool ismobileenabled = false;

	    MobileAdapterGB *mobilegb = NULL;

	    void setmobileadapter(MobileAdapterGB *gb)
	    {
		mobilegb = gb;
	    }

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
    };
};

#endif // LIBMBGB_CORE
