#ifndef GB_GPU
#define GB_GPU

#include "libmbgb_api.h"
#include "cpu.h"
#include "mmu.h"
#include "utils.h"
using namespace gb;

namespace gb
{
    class LIBMBGB_API GPU
    {
	public:
	    GPU();
	    ~GPU();

	    void reset();
	    void clearscreen();

	    CPU *gcpu;
	    MMU *gmem;

	    uint8_t framebuffer[160 * 144][3];

	    void updategraphics(int cycles);
	    void setlcdstatus();
	    
	    void drawscanline();
	    void rendertiles(uint8_t lcdcontrol);
	    int getcolor(int id, uint16_t palette);

	    int scanlinecounter;
    };
}


#endif // GB_GPU
