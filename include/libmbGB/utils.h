#ifndef UTIL_H
#define UTIL_H

#include <cstdint>

bool TestBit(uint8_t reg, int bit);

uint8_t BitSet(uint8_t reg, int bit);

uint8_t BitReset(uint8_t reg, int bit);

int BitGetVal(uint8_t reg, int bit);


#endif // UTIL_H
