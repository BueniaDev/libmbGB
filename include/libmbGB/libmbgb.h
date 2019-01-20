#ifndef GB_CORE
#define GB_CORE

#include "mmu.h"
#include "cpu.h"
#include "gpu.h"
#include "apu.h"
#include "input.h"
#include "timers.h"
#include <string>
using namespace gb;
using namespace std;

namespace gb
{
    class LIBMBGB_API DMGCore
    {
	public:
	    DMGCore();
	    ~DMGCore();

	    void reset();
	    bool loadROM(string filename);
	    bool loadBIOS(string filename);
	    bool getoptions(int argc, char* argv[]);
	    void runcore();

	    CPU corecpu;
	    MMU coremmu;
	    GPU coregpu;
        APU coreapu;
	    Input coreinput;
	    Timers coretimers;
    };
}

#endif // GB_CORE
