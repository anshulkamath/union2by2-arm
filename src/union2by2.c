#include "union2by2.h"

#include <arm_neon.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#include "avx_utils.c"
#include "neon_utils.c"

uint16_t union2by2_scalar(uint16_t *dst, uint16_t *src1, uint16_t len1,
			  uint16_t *src2, uint16_t len2)
{
	uint16_t *original_dst = dst;

	uint16_t pos1 = 0;
	uint16_t pos2 = 0;

	uint16_t s1, s2;
	while (pos1 < len1 && pos2 < len2) {
		s1 = src1[pos1];
		s2 = src2[pos2];

		*(dst++) = s1 <= s2 ? s1 : s2;
		pos1 += s1 <= s2;
		pos2 += s2 <= s1;
	}

	// flush rest remaining set
	if (pos1 == len1) {
		src1 = src2;
		pos1 = pos2;
		len1 = len2;
	}

	while (pos1 < len1) {
		*(dst++) = src1[pos1++];
	}

	return dst - original_dst;
}

uint16_t union2by2_vectorized(uint16_t *dst, uint16_t *src1, uint16_t len1,
			      uint16_t *src2, uint16_t len2)
{
	if ((len1 < 8) || (len2 < 8)) {
		return union2by2_scalar(dst, src1, len1, src2, len2);
	}

	VECTORIZED vA, vB, v_min, v_max, last_store;
	uint16_t n_vecs1, n_vecs2;
	uint16_t pos1, pos2;
	uint16_t *original_dst = dst;

	n_vecs1 = len1 / 8;
	n_vecs2 = len2 / 8;

	pos1 = 0;
	pos2 = 0;

	vA = vld1q_u16((VECTORIZED *)src1 + pos1);
	pos1++;

	vB = vld1q_u16((VECTORIZED *)src2 + pos2);
	pos2++;

	merge(vA, vB, &v_min, &v_max);
	last_store = vmovq_n_u16(-1);

	dst += store_unique(last_store, v_min, dst);
	last_store = v_min;

	VECTORIZED V;
	uint16_t curr_a, curr_b, *src;
	uint16_t pos;
	int cmp;
	while ((pos1 < n_vecs1) && (pos2 < n_vecs2)) {
		curr_a = src1[pos1 * 8];
		curr_b = src2[pos2 * 8];

		// prefer cmov to branch prediction
		// apparently the general rule of thumb is to use cmovs if the
		// predictability is under 75%, which should be the case here (we
		// expect branch predictability to be ~50%)
		cmp = curr_a <= curr_b;
		src = cmp ? src1 : src2;
		pos = cmp ? pos1 : pos2;

		V = vld1q_u16((VECTORIZED *)src + pos);

		pos1 += cmp;
		pos2 += !cmp;

		merge(V, v_max, &v_min, &v_max);
		dst += store_unique(last_store, v_min, dst);
		last_store = v_min;
	}

	// we exhausted the smaller of the two sets of 8-tuples, so we must
	// manually finish off the algorithm. we have vec_max elements left
	// from the algorithm and up to 7 other elements from the exhausted
	// set. we toss those elements into a buffer, sort the buffer, and
	// then store the uniques onto the destination. then, we run a scalar
	// union2by2 on the remaining set.
	uint16_t buffer[16];
	uint16_t buffer_size = store_unique(last_store, v_max, buffer);

	uint16_t *smaller, *bigger;
	uint16_t smaller_size, bigger_size;

	if (pos1 == n_vecs1) {
		smaller = src1 + 8 * pos1;
		bigger = src2 + 8 * pos2;

		smaller_size = len1 - (pos1 * 8);
		bigger_size = len2 - (pos2 * 8);
	} else {
		smaller = src2 + 8 * pos2;
		bigger = src1 + 8 * pos1;

		smaller_size = len2 - (pos2 * 8);
		bigger_size = len1 - (pos1 * 8);
	}

	memcpy(buffer + buffer_size, smaller, smaller_size * sizeof(uint16_t));
	buffer_size += smaller_size;
	qsort(buffer, buffer_size, sizeof(uint16_t), compare_uints);
	buffer_size = unique(buffer, buffer_size);

	dst += union2by2_scalar(dst, buffer, buffer_size, bigger, bigger_size);
	return dst - original_dst;
}
