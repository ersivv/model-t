#ifndef __COMMON_H__
#define __COMMON_H__

#include "types.h"

#define SETBIT(p, b) ((uint8_t*)(p))[(b) / 8] |= (1 << ((b) % 8))
#define CLRBIT(p, b) ((uint8_t*)(p))[(b) / 8] &= ~(1 << ((b) % 8))
#define ASSIGNBIT(p, b, v) if (v) { SETBIT(p, b); } else { CLRBIT(p, b); }
#define TESTBIT(p, b) ((((uint8_t*)(p))[(b) / 8] & (1 << ((b) % 8))) != 0)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define gp_low(bank, bit) palClearPad(bank, bit)
#define gp_high(bank, bit) palSetPad(bank, bit)

#endif
