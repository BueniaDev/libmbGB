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
	    uint8_t memdump[0x100000];
	    uint8_t bios[0x900];
	    uint8_t cartmem[0x800000];
            uint8_t rambanks[0x8000];
            uint8_t currentrombank = 1;
            uint8_t currentrambank = 0;
            uint8_t higherrombankbits = 0;
            uint8_t specialrombanks[4] = {0x00, 0x20, 0x40, 0x60};
            uint8_t specialmbc3banks[4] = {0x00};
	    uint8_t vram[0x4000] = {0};
	    uint8_t wram[0x8000] = {0};
	    int wrambank = 1;
	    int vrambank1 = 0;
	    uint8_t gbcbgpallete[0x40];
	    uint8_t gbcobjpallete[0x40];
	    int gbcbgpalleteindex = 0;
	    int gbcobjpalleteindex = 0;
	    bool gbcbgpallinc = false;
	    bool gbcobjpallinc = false;
	    bool doublespeed = false;

	    bool biosload = false;
	    bool isgbcenabled = false;
	    int gbtype = 0;
	    bool ismanual = false;
	    bool isgbcbios = false;
	    int biossize;
        
            bool loadmmu(string filename);
            bool savemmu(string filename);
	    bool dumpmem();

            void reset();
	    void resetmem();

            uint8_t readDirectly(uint16_t address);
            void writeDirectly(uint16_t address, uint8_t value);
            uint8_t readVram(uint16_t address);
            void writeVram(uint16_t address, uint8_t value);
	    uint8_t readWram(uint16_t address);
	    void writeWram(uint16_t address, uint8_t value);
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

	    void hdmatransfer();
	    bool hdmaactive = false;
	    uint16_t hdmadest = 0;
	    uint16_t hdmasource = 0;
	    uint16_t hdmalength = 0;

        int mbctype = 0;
        int ramsize = 0;
        int rombanks = 0;
        int rambank = 0;
	    bool notmbc = false;
        
        bool ramenabled = false;
	bool rtc = false;
        int rommode = 0;
	uint8_t ramrtcselect = 0;
	uint8_t realsecs = 0;
	uint8_t realmins = 0;
	uint8_t realhours = 0;
	uint8_t realdays = 0;
	uint8_t realdayshi = 0;
	uint8_t latchsecs = 0;
	uint8_t latchmins = 0;
	uint8_t latchhours = 0;
	uint8_t latchdays = 0;
	uint8_t latchdayshi = 0;
	bool latch = false;

	void updatetimer();
	void latchtimer();
	unsigned int currenttime;

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
