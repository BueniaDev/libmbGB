#ifndef GB_JOYPAD
#define GB_JOYPAD

#include "libmbgb_api.h"
#include "utils.h"

namespace gb
{
    class LIBMBGB_API Input
    {
	public:
	    Input();
	    ~Input();

	    void reset();
	    void keypressed(int key);
	    void keyreleased(int key);
	    void write(uint8_t set);
	    uint8_t getjoypadstate();

	    void setkey(int key, bool set);

	    bool up = false;
	    bool down = false;
	    bool left = false;
	    bool right = false;
	    bool a = false;
	    bool b = false;
	    bool select = false;
	    bool start = false;

	    bool directionswitch = false;
	    bool buttonswitch = false;

	    uint8_t p1data = 0xFF;
    };
}



#endif // GB_JOYPAD
