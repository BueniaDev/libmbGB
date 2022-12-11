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

#include <libmbgb.h>
using namespace gb;

namespace gb
{
    GBCore::GBCore()
    {
	coregpu = make_unique<GBGPU>();
	coreinput = make_unique<GBInput>();
	coretimers = make_unique<GBTimers>();
	coremmu = make_unique<GBMMU>(*coregpu, *coreinput, *coretimers);
	corecpu = make_unique<GBCPU>(*coremmu);
    }

    GBCore::~GBCore()
    {

    }

    bool GBCore::init()
    {
	if (!initCore())
	{
	    return false;
	}

	cout << "mbGB::Initialized" << endl;
	return true;
    }

    void GBCore::shutdown()
    {
	if (front != NULL)
	{
	    front->shutdown();
	}

	coregpu->shutdown();
	coreinput->shutdown();
	coretimers->shutdown();
	coremmu->shutdown();
	corecpu->shutdown();
	cout << "mbGB::Shutting down..." << endl;
    }

    void GBCore::setFrontend(mbGBFrontend *cb)
    {
	front = cb;
    }

    bool GBCore::initCore()
    {
	if (is_xmas())
	{
	    cout << "Happy holidays from libmbGB!" << endl;
	}

	coregpu->init();
	coreinput->init();
	coretimers->init();
	coremmu->init();
	corecpu->init();

	if (front != NULL)
	{
	    if (!front->init())
	    {
		return false;
	    }
	}

	cout << "mbGB::Initialized" << endl;
	return true;
    }

    void GBCore::runCore()
    {
	while (coremmu->isFrame())
	{
	    corecpu->runInstruction();
	}

	coremmu->resetFrame();
    }

    void GBCore::keyChanged(GBButton button, bool is_pressed)
    {
	coreinput->keyChanged(button, is_pressed);
    }

    void GBCore::keyPressed(GBButton button)
    {
	keyChanged(button, true);
    }

    void GBCore::keyReleased(GBButton button)
    {
	keyChanged(button, false);
    }

    bool GBCore::loadBIOS(vector<uint8_t> bios)
    {
	return coremmu->loadBIOS(bios);
    }

    bool GBCore::loadROM(vector<uint8_t> rom)
    {
	return coremmu->loadROM(rom);
    }

    bool GBCore::loadBIOS(string filename)
    {
	if (front == NULL)
	{
	    return false;
	}

	return loadBIOS(front->loadFile(filename));
    }

    bool GBCore::loadROM(string filename)
    {
	if (front == NULL)
	{
	    return false;
	}

	return loadROM(front->loadFile(filename));
    }
};