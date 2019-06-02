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
	    uint8_t winscanline[160];
	    uint8_t bgscancolor[160];

	    bool bgprior[160] = { false };
	    bool winprior[160] = { false };

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
    };
}

#endif // GB_GPU
