#include "../../include/libmbGB/utils.h"


bool TestBit(uint8_t reg, int bit)
{
    return (reg & (1 << bit)) ? true : false;
}

uint8_t BitSet(uint8_t reg, int bit)
{
    return (reg | (1 << bit));
}

uint8_t BitReset(uint8_t reg, int bit)
{
    return (reg & ~(1 << bit));
}

