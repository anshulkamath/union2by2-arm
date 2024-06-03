#ifndef UNION2BY2_H
#define UNION2BY2_H

#include <stdlib.h>

uint16_t union2by2_scalar(uint16_t *dst, uint16_t *src1, uint16_t len1, uint16_t *src2,
		     uint16_t len2);

uint16_t union2by2_vectorized(uint16_t *dst, uint16_t *src1, uint16_t len1, uint16_t *src2,
		     uint16_t len2);

#endif // UNION2BY2_H
