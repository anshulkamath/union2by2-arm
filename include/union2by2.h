#ifndef UNION2BY2_H
#define UNION2BY2_H

#include <stdlib.h>

#if (defined(__x86_64__) || defined(_M_X64))
#define USE_AVX
#define VECTORIZED __m128i
#else
#define VECTORIZED uint16x8_t
#endif

uint16_t union2by2_scalar(uint16_t *dst, uint16_t *src1, uint16_t len1, uint16_t *src2,
		     uint16_t len2);

uint16_t union2by2_vectorized(uint16_t *dst, uint16_t *src1, uint16_t len1, uint16_t *src2,
		     uint16_t len2);

#endif // UNION2BY2_H
