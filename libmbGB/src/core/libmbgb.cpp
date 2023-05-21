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
	coreapu = make_unique<GBAPU>();
	coreinput = make_unique<GBInput>();
	coretimers = make_unique<GBTimers>();
	coreserial = make_unique<GBSerial>();
	coremmu = make_unique<GBMMU>(*coregpu, *coreinput, *coretimers, *coreserial, *coreapu);
	corecpu = make_unique<GBCPU>(*coremmu);
    }

    GBCore::~GBCore()
    {

    }

    void GBCore::setModel(GBModel model)
    {
	coremmu->setModel(model);
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
	coreapu->shutdown();
	coreinput->shutdown();
	coretimers->shutdown();
	coreserial->shutdown();
	coremmu->shutdown();
	corecpu->shutdown();
	cout << "mbGB::Shutting down..." << endl;
    }

    void GBCore::setFrontend(mbGBFrontend *cb)
    {
	front = cb;

	coreapu->setOutputCallback([&](int16_t left, int16_t right) -> void
	{
	    if (front != NULL)
	    {
		front->audioCallback(left, right);
	    }
	});

	coremmu->setRumbleCallback([&](double strength) -> void
	{
	    if (front != NULL)
	    {
		front->rumbleCallback(strength);
	    }
	});
    }

    bool GBCore::initCore()
    {
	if (is_xmas())
	{
	    cout << "Happy holidays from libmbGB!" << endl;
	}
	else if (is_halloween())
	{
	    cout << "Happy Halloween from libmbGB!" << endl;
	}

	if (front != NULL)
	{
	    if (!front->init())
	    {
		return false;
	    }
	}

	coregpu->init(coremmu->getModel(), coremmu->isBIOSLoad());
	coreapu->init();
	coreinput->init();
	coretimers->init();
	coreserial->init();
	coremmu->init();
	corecpu->init();

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

    void GBCore::contextKeyChanged(GBContextButton button, bool is_pressed)
    {
	coremmu->contextKeyChanged(button, is_pressed);
    }

    void GBCore::contextKeyPressed(GBContextButton button)
    {
	contextKeyChanged(button, true);
    }

    void GBCore::contextKeyReleased(GBContextButton button)
    {
	contextKeyChanged(button, false);
    }

    void GBCore::updateAccel(float xpos, float ypos)
    {
	coremmu->updateAccel(xpos, ypos);
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

    void GBCore::connectSerialDevice(GBSerialDevice *device)
    {
	coreserial->connectSerialDevice(device);
    }

    void GBCore::saveBackup(string filename)
    {
	if (front == NULL)
	{
	    return;
	}

	front->saveFile(filename, saveBackup());
    }

    void GBCore::loadBackup(string filename)
    {
	if (front == NULL)
	{
	    return;
	}

	loadBackup(front->loadFile(filename));
    }
};