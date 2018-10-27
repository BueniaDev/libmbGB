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

	    int m_cycles;

	    MMU *mem;

	    void executenextopcode();
	    void executeopcode(uint8_t opcode);
	    void executecbopcode(uint8_t opcode);

	    void daa();
	    void load8bit(uint8_t regone, uint8_t regtwo, int cycles);
	    void load16bit(uint16_t regone, uint16_t regtwo, int cycles);
	    void pushontostack(uint16_t regone, int cycles);
	    void popontostack(uint16_t regone, int cycles);
	    void add8bit(uint8_t regone, uint8_t regtwo, int cycles, bool carry);
	    void sub8bit(uint8_t regone, uint8_t regtwo, int cycles, bool carry);
	    void and8bit(uint8_t regone, uint8_t regtwo, int cycles);
	    void or8bit(uint8_t regone, uint8_t regtwo, int cycles);
	    void xor8bit(uint8_t regone, uint8_t regtwo, int cycles);
	    void inc8bit(uint8_t regone, int cycles);
	    void dec8bit(uint8_t regone, int cycles);
	};
}
