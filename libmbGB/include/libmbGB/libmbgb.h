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

#ifndef LIBMBGB_CORE_H
#define LIBMBGB_CORE_H

#include "mmu.h"
#include "gpu.h"
#include "apu.h"
#include "input.h"
#include "timers.h"
#include "serial.h"
#include "cpu.h"
using namespace gb;

namespace gb
{
    class LIBMBGB_API GBCore
    {
	public:
	    GBCore();
	    ~GBCore();

	    bool init();
	    void shutdown();

	    void setModel(GBModel model);

	    void connectSerialDevice(GBSerialDevice *device);

	    bool initCore();
	    void runCore();

	    bool loadBIOS(vector<uint8_t> bios);
	    bool loadROM(vector<uint8_t> rom);

	    bool loadBIOS(string filename);
	    bool loadROM(string filename);

	    void keyChanged(GBButton button, bool is_pressed);

	    void keyPressed(GBButton button);
	    void keyReleased(GBButton button);

	    void setFrontend(mbGBFrontend *cb);

	    vector<GBRGB> getFramebuffer()
	    {
		return coregpu->getFramebuffer();
	    }

	    int getDepth()
	    {
		return coregpu->getDepth();
	    }


	private:
	    unique_ptr<GBMMU> coremmu;
	    unique_ptr<GBGPU> coregpu;
	    unique_ptr<GBInput> coreinput;
	    unique_ptr<GBTimers> coretimers;
	    unique_ptr<GBSerial> coreserial;
	    unique_ptr<GBAPU> coreapu;
	    unique_ptr<GBCPU> corecpu;

	    mbGBFrontend *front = NULL;
    };
};

#endif // LIBMBGB_CORE_H