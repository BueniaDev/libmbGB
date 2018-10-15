#ifndef UTIL_H
#define UTIL_H

#define Bit(b) (1 << b)
#define Test(x, y) (x & y)
#define Set(x, y) ( (x) |= (y) )
#define Reset(x, y) ( (x) &= (~y) )

#define TestBit(x, y) ( Test(x, Bit(y)) != 0 )
#define BitSet(x, y) ( Set(x, Bit(y)) )
#define BitReset(x, y) ( Reset(x, Bit(y)) )



#endif // UTIL_H
