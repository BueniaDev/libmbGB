#ifndef GB_MMU
#define GB_MMU

#include "libmbgb_api.h"
#include "input.h"
#include "apu.h"
#include "utils.h"
#include <cstdint>
using namespace std;

namespace gb
{
    class LIBMBGB_API MMU
    {
        public:
            MMU();
            ~MMU();

            uint8_t memorymap[0x10000];
	    uint8_t bios[0x100];
	    uint8_t cartmem[0x200000];
        uint8_t rambanks[0x8000];
        uint8_t currentrombank = 1;
        uint8_t currentrambank = 0;
        uint8_t higherrombankbits = 0;
        uint8_t specialrombanks[4] = {0x00, 0x20, 0x40, 0x60};
        uint8_t specialmbc3banks[4] = {0x00};

	    bool biosload;
        
            bool loadmmu(string filename);
            bool savemmu(string filename);

            void reset();
	    void resetmem();

            uint8_t readByte(uint16_t address);
            void writeByte(uint16_t address, uint8_t value);
            uint16_t readWord(uint16_t address);
            void writeWord(uint16_t address, uint16_t value);
	    int8_t readsByte(uint16_t address);

        int getmbctype(uint8_t mbcval);
        int getramsize(uint8_t ramval);
        int getrombanks(uint8_t romval);
        int getrambanks(int rambankval);

	    bool loadROM(string filename);
	    bool loadBIOS(string filename);

        int mbctype = 0;
        int ramsize = 0;
        int rombanks = 0;
        int rambank = 0;
	    bool notmbc = false;
        
        bool ramenabled = false;
        int rommode = 0;

	    uint8_t mbc1read(uint16_t address);
	    void mbc1write(uint16_t address, uint8_t value);
        uint8_t mbc2read(uint16_t address);
        void mbc2write(uint16_t address, uint8_t value);
        uint8_t mbc3read(uint16_t address);
        void mbc3write(uint16_t address, uint8_t value);
        uint8_t mbc5read(uint16_t address);
        void mbc5write(uint16_t address, uint8_t value);

	    Input *joypad;
        APU *audio;
    };
}

#endif // GB_MMU
