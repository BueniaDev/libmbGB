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

#ifndef LIBMBGB_IRDEVICES
#define LIBMBGB_IRDEVICES

#include "libmbgb_api.h"
#include "utils.h"
#include <iostream>
#include <functional>
using namespace gb;
using namespace std;

namespace gb
{

    // Interface for addons that communicate via the GBC's IR port
    class LIBMBGB_API InfraredDevice
    {
	public:
	    InfraredDevice();
	    ~InfraredDevice();

	    virtual string getdevicename() = 0; // Fetches the name of the connected addon
	    virtual bool getirstatus() = 0; // Fetches the IR status (true = light, false = no light)
	    virtual void updateir(bool rp_val) = 0; // Updates the IR light (must be called once per instruction)
    };

    // Emulates a disconnected IR device
    class LIBMBGB_API DisconnectedIR : public InfraredDevice
    {
	public:
	    DisconnectedIR();
	    ~DisconnectedIR();

	    string getdevicename()
	    {
		return "Disconnected";
	    }

	    bool getirstatus()
	    {
		return false;
	    }

	    void updateir(bool rp_val)
	    {
		return;
	    }
    };

    // Emulates a disconnected IR device
    class LIBMBGB_API InfraredDebug : public InfraredDevice
    {
	public:
	    InfraredDebug();
	    ~InfraredDebug();

	    bool prev_rp_val = false;
	    bool is_sending_signal = false;
	    int rp_counter = 0;

	    string getdevicename()
	    {
		return "InfraredDebug";
	    }

	    bool getirstatus()
	    {
		return false;
	    }

	    void updateir(bool rp_val)
	    {
		if (rp_val && !prev_rp_val)
		{
		    cout << "[InfraredDebug] Turning on IR..." << endl;
		    is_sending_signal = true;
		}
		else if (!rp_val && prev_rp_val)
		{
		    cout << "[InfraredDebug] Turning off IR..." << endl;
		    cout << "[InfraredDebug] IR signal lasted for " << dec << (int)(rp_counter) << " cycles" << endl;
		    cout << endl;
		    rp_counter = 0;
		}
		else if (rp_val && is_sending_signal)
		{
		    rp_counter += 4;

		    if (rp_counter == (4194304 * 10))
		    {
			cout << "[InfraredDebug] IR signal timed out due to inactivity..." << endl;
			is_sending_signal = false;
			rp_counter = 0;
		    }
		}

		prev_rp_val = rp_val;
		return;
	    }
    };

    // Emulates an artifical light source (i.e. an ordinary household lamp, used by Chee Chai Alien)
    class LIBMBGB_API CheeChaiLight : public InfraredDevice
    {
	public:
	    CheeChaiLight();
	    ~CheeChaiLight();

	    string getdevicename()
	    {
		return "CheeChaiLight";
	    }

	    bool getirstatus()
	    {
		// TODO: Add interactive mode (like with GBE+)
		return true;
	    }

	    void updateir(bool rp_val)
	    {
		return;
	    }
    };
};

#endif // LIBMBGB_IRDEVICES