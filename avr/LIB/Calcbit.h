#ifndef CALCBIT_H
#define CALCBIT_H

#define setbit(reg, bit_no) ((reg) |= (1 << bit_no))
#define clearbit(reg, bit_no) ((reg) &= ~(1 << bit_no))
#define getbit(reg, bit_no) (((reg) & (1 << bit_no)) >> bit_no)
// #define getbit(reg, bit_no) ((reg >> bit_no) & 1)
#define togglebit(reg, bit_no) ((reg) ^= (1 << bit_no))

#endif