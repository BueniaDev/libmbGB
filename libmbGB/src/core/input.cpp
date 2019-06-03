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
	directionswitch = !TestBit(set, 4);
	buttonswitch = !TestBit(set, 5);
    }

    uint8_t Input::getjoypadstate()
    {
	uint8_t buttons = 0b1111;

	if (directionswitch)
	{
	    buttons = BitSetTo(buttons, 0, !right);
	    buttons = BitSetTo(buttons, 1, !left);
	    buttons = BitSetTo(buttons, 2, !up);
	    buttons = BitSetTo(buttons, 3, !down);
	}

	if (buttonswitch)
	{
	    buttons = BitSetTo(buttons, 0, !a);
	    buttons = BitSetTo(buttons, 1, !b);
	    buttons = BitSetTo(buttons, 2, !select);
	    buttons = BitSetTo(buttons, 3, !start);
	}


	buttons = BitSetTo(buttons, 4, !directionswitch);
	buttons = BitSetTo(buttons, 5, !buttonswitch);

	return buttons;
    }
}
