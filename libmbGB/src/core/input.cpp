#include "../../include/libmbGB/input.h"
#include <iostream>
using namespace gb;
using namespace std;

namespace gb
{
    Input::Input()
    {
	
    }

    Input::~Input()
    {
	cout << "Input::Shutting down..." << endl;
    }

    void Input::reset()
    {
	cout << "Input::Initialized" << endl;
    }

    void Input::setkey(int key, bool set)
    {
	switch (key)
	{
	    case 0: up = set; break;
	    case 1: down = set; break;
	    case 2: left = set; break;
	    case 3: right = set; break;
	    case 4: start = set; break;
	    case 5: select = set; break;
	    case 6: a = set; break;
	    case 7: b = set; break;
	}
    }

    void Input::keypressed(int key)
    {
	setkey(key, true);
    }

    void Input::keyreleased(int key)
    {
	setkey(key, false);
    }

    void Input::write(uint8_t set)
    {
	p1data = (p1data & 0xF) | (set & 0x30);
    }

    uint8_t Input::getjoypadstate()
    {
	uint8_t controlbits = 0;

	if ((p1data & 0x30) == 0x20)
	{
	    controlbits |= (right ? 0 : 1) << 0;
	    controlbits |= (left ? 0 : 1) << 1;
	    controlbits |= (up ? 0 : 1) << 2;
	    controlbits |= (down ? 0 : 1) << 3;
	}
	else if ((p1data & 0x30) == 0x10)
	{
	    controlbits |= (a ? 0 : 1) << 0;
	    controlbits |= (b ? 0 : 1) << 1;
	    controlbits |= (select ? 0 : 1) << 2;
	    controlbits |= (start ? 0 : 1) << 3;
	}
	else
	{
	    controlbits = 0xF;
	}

	p1data &= 0xF0;
	p1data |= controlbits;
	return (p1data | 0xC0);
    }
}
