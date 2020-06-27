/*  Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

#ifndef XOSHIRO_H
#define XOSHIRO_H


#include <stdint.h>
#include <stdio.h>
static uint64_t s[4];

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}


void create(const uint64_t s1,const uint64_t s2,const uint64_t s3,const uint64_t s4){
    s[0]=s1;
    s[1]=s2;
    s[2]=s3;
    s[3]=s3;
}



double next(void) {
	const uint64_t result_plus = s[0] + s[3];

	const uint64_t t = s[1] << 17;
	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = rotl(s[3], 45);

	//return (result_plus >> 11) * 0x1.0p-53;
	return (result_plus >> 11) * (1. / (UINT64_C(1) << 53));
}
 

#endif
