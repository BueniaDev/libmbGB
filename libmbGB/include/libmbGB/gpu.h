#ifndef GB_GPU
#define GB_GPU

#include "libmbgb_api.h"
#include "mmu.h"
#include "utils.h"
using namespace gb;

namespace gb
{
    struct RGB
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };
    
    class LIBMBGB_API GPU
    {
	public:
	    GPU();
	    ~GPU();

	    void reset();
	    void clearscreen();

	    MMU *gmem;

	    RGB framebuffer[160 * 144];
	    uint8_t bgscanline[160];
	    uint8_t bgscancolor[160];

	    void updategraphics(int cycles);
        void checklyc();
	    void setlcdstatus();
	    
	    void drawscanline();
	    void rendertiles(uint8_t lcdcontrol);
	    void renderwindow(uint8_t lcdcontrol);
	    void rendersprites(uint8_t lcdcontrol);
	    int getdmgcolor(int id, uint16_t palette);
	    int getgbccolor(int id, int color);
	    int getgbcobjcolor(int id, int color);

	    int scanlinecounter;
        int windowlinecounter;
        uint8_t mode = 0;
        bool newvblank = false;

	uint8_t colorarray[32] = 
	{
	    0x0, 0x8, 0x10, 0x18, 0x20, 0x29, 0x31,
	    0x39, 0x41, 0x4A, 0x52, 0x5A, 0x62, 0x6A,
	    0x73, 0x7B, 0x83, 0x8B, 0x94, 0x9C, 0xA4,
	    0xAC, 0xB4, 0xBD, 0xC5, 0xCD, 0xD5, 0xDE,
	    0xE6, 0xEE, 0xF6, 0xFF
	};
    };
}

#endif // GB_GPU
