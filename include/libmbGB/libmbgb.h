#ifndef GB_CORE
#define GB_CORE

#include "mmu.h"
#include "cpu.h"
#include "gpu.h"
#include "input.h"
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

	    void loadROM(string filename);
	    void loadBIOS(string filename);
	    bool getoptions(int argc, char* argv[]);
	    void runcore();

	    CPU corecpu;
	    MMU coremmu;
	    GPU coregpu;
	    Input coreinput;
    };
}

#endif // GB_CORE
