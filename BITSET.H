#ifndef BITSET_H_
#define BITSET_H_
extern word bitset[];
#define BITSET_RESET memset(bitset,0,sizeof(bitset));
#define BITSET_INSERT(n) bitset[(n)/16]|=1<<((n)%16);
#define BITSET_CONTAINS(n) (bitset[(n)/16]&(1<<((n)%16)))
#endif
