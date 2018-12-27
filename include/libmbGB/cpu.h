#ifndef GB_CPU
#define GB_CPU

#include "libmbgb_api.h"
#include "mmu.h"
#include "gpu.h"
#include "timers.h"
#include "utils.h"
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

	        void reset();
	        void resetBIOS();

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

		bool stopped;
		bool halted;

		int m_bytes;
	        int m_cycles;

		bool interruptmaster;
		bool interruptdelay;
		bool skipinstruction;

		void dointerrupts();
		void requestinterrupt(int id);

	        MMU *mem;
		Timers *timers;
		GPU *gpu;

		void advancecycles(int cycles);

	        void executenextopcode();
	        void executeopcode(uint8_t opcode);
	        void executecbopcode();

	        void daa();
		void stop();
	        uint8_t add8bit(uint8_t regone, uint8_t regtwo);
		uint8_t add8bitc(uint8_t regone, uint8_t regtwo);
	        uint8_t sub8bit(uint8_t regone, uint8_t regtwo);
	        uint8_t sub8bitc(uint8_t regone, uint8_t regtwo);
	        uint8_t and8bit(uint8_t regone, uint8_t regtwo);
	        uint8_t or8bit(uint8_t regone, uint8_t regtwo);
	        uint8_t xor8bit(uint8_t regone, uint8_t regtwo);
	        uint8_t inc8bit(uint8_t regone);
	        uint8_t dec8bit(uint8_t regone);
	        uint16_t add16bit(uint16_t regone, uint16_t regtwo);
	        uint16_t adds8bit(uint16_t regone, uint8_t regtwo);
		uint8_t swap(uint8_t regone);
		uint8_t rl(uint8_t regone);
		uint8_t rlc(uint8_t regone);
		uint8_t rr(uint8_t regone);
		uint8_t rrc(uint8_t regone);
		uint8_t sla(uint8_t regone);
		uint8_t sra(uint8_t regone);
		uint8_t srl(uint8_t regone);
		void bit(uint8_t regone, int bit);
		uint8_t set(uint8_t regone, int bit);
		uint8_t res(uint8_t regone, int bit);
		void jr(uint8_t regone);
	};
}

#endif // GB_CPU
