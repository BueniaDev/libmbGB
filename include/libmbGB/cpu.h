#include "libmbgb_api.h"
#include "mmu.h"
using namespace gb;

namespace gb
{
	class LIBMBGB_API CPU
	{
	    CPU();
	    ~CPU();

	    union Register
	    {
		uint16_t reg;
		struct
		{
		    uint8_t hi;
		    uint8_t lo;
		};
	    };

	    void CPUReset();
	    void CPUResetBIOS();

	    Register af;
	    Register bc;
	    Register de;
	    Register hl;

	    uint16_t pc;
	    uint16_t sp;

	    int zero = 7;
	    int subtract = 6;
	    int half = 5;
    	    int carry = 4;
	};
}
