#ifndef GB_MMU
#define GB_MMU

#include "libmbgb_api.h"
#include <cstdint>

namespace gb
{
    class LIBMBGB_API MMU
    {
        public:
            MMU();
            ~MMU();

            uint8_t memorymap[0x10000];
            uint8_t bios[0x100];

            void reset();
            bool biosload;
            bool inbios;

            uint8_t readByte(uint16_t address);
            void writeByte(uint16_t address, uint8_t value);
            uint16_t readWord(uint16_t address);
            void writeWord(uint16_t address, uint16_t value);
	    int8_t readsByte(uint16_t address);
    };
}

#endif // GB_MMU
