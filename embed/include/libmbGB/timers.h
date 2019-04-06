#ifndef GB_TIMERS
#define GB_TIMERS

#include "libmbgb_api.h"
#include "mmu.h"
#include "utils.h"
using namespace gb;

namespace gb
{
    class LIBMBGB_API Timers
    {
	public:
	    Timers();
	    ~Timers();

	    void reset();
	    void updatetimers(int cycles);
	    void dodividerregister(int cycles);
	    int dividerregister;
	    int timercounter;
	    int getclockfreq(uint8_t freq);

	    MMU *tmem;
    };
}

#endif // GB_TIMERS
