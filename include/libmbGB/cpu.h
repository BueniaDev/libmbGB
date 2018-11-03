#include "libmbgb_api.h"
#include "mmu.h"
using namespace gb;

#if defined(MSB_FIRST) || defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define IS_BIG_ENDIAN
#else
#define IS_LITTLE_ENDIAN
#endif

namespace gb
{
	union Register
	{
	    struct
	    {
		#ifdef IS_LITTLE_ENDIAN
		    uint8_t lo;
		    uint8_t hi;
		#else
		    uint8_t hi;
		    uint8_t lo;
		#endif
	    };
	    uint16_t reg;
	};	

	class LIBMBGB_API CPU
	{
	    public:
		CPU();
		~CPU();

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
	        void pushontostack(uint16_t regone, int cycles);
	        void popontostack(uint16_t regone, int cycles);
	        uint8_t add8bit(uint8_t regone, uint8_t regtwo, bool carry);
	        uint8_t sub8bit(uint8_t regone, uint8_t regtwo, bool carry);
	        uint8_t and8bit(uint8_t regone, uint8_t regtwo);
	        uint8_t or8bit(uint8_t regone, uint8_t regtwo);
	        uint8_t xor8bit(uint8_t regone, uint8_t regtwo);
	        uint8_t inc8bit(uint8_t regone);
	        uint8_t dec8bit(uint8_t regone);
	        uint16_t add16bit(uint16_t regone, uint16_t regtwo);
	        uint16_t adds16bit(uint16_t regone, uint8_t regtwo);
	};
}
