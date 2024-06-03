#include "union2by2.h"

#ifndef USE_AVX

#ifndef NEON_UTILS
#define NEON_UTILS

#include <arm_neon.h>
#include <stdlib.h>

#include "shuffle_mat.c"

/* Example:
 *  a = { 0, 2, 4, 6 }
 *  b = { 1, 3, 5, 7 }
 *
 *  tmp = { 0, 2, 4, 6 }
 *  max = { 1, 3, 5, 7 }
 *
 *  tmp = { 2, 4, 6, 0 }
 *
 *  min = { 1, 3, 5, 0 }
 *  max = { 2, 4, 6, 7 }
 *
 *  tmp = { 3, 5, 0, 1 }
 *
 *  min = { 2, 4, 0, 1 }
 *  max = { 3, 5, 6, 7 }
 *
 *  tmp = { 4, 0, 1, 2 }
 *
 *  min = { 3, 0, 1, 2 }
 *  max = { 4, 5, 6, 7 }
 *
 *  min = { 0, 1, 2, 3 }
 *
 */
static inline void merge(const uint16x8_t v_input1,
			      const uint16x8_t v_input2, uint16x8_t *vec_min,
			      uint16x8_t *vec_max)
{
	uint16x8_t vec_tmp;
	vec_tmp = vminq_u16(v_input1, v_input2);
	*vec_max = vmaxq_u16(v_input1, v_input2);
	vec_tmp = vextq_u8(vec_tmp, vec_tmp, 2);
	*vec_min = vminq_u16(vec_tmp, *vec_max);
	*vec_max = vmaxq_u16(vec_tmp, *vec_max);
	vec_tmp = vextq_u8(*vec_min, *vec_min, 2);
	*vec_min = vminq_u16(vec_tmp, *vec_max);
	*vec_max = vmaxq_u16(vec_tmp, *vec_max);
	vec_tmp = vextq_u8(*vec_min, *vec_min, 2);
	*vec_min = vminq_u16(vec_tmp, *vec_max);
	*vec_max = vmaxq_u16(vec_tmp, *vec_max);
	vec_tmp = vextq_u8(*vec_min, *vec_min, 2);
	*vec_min = vminq_u16(vec_tmp, *vec_max);
	*vec_max = vmaxq_u16(vec_tmp, *vec_max);
	vec_tmp = vextq_u8(*vec_min, *vec_min, 2);
	*vec_min = vminq_u16(vec_tmp, *vec_max);
	*vec_max = vmaxq_u16(vec_tmp, *vec_max);
	vec_tmp = vextq_u8(*vec_min, *vec_min, 2);
	*vec_min = vminq_u16(vec_tmp, *vec_max);
	*vec_max = vmaxq_u16(vec_tmp, *vec_max);
	vec_tmp = vextq_u8(*vec_min, *vec_min, 2);
	*vec_min = vminq_u16(vec_tmp, *vec_max);
	*vec_max = vmaxq_u16(vec_tmp, *vec_max);
	*vec_min = vextq_u8(*vec_min, *vec_min, 2);
}

static uint8x8_t vmovmaskq_u8(uint8x8_t input)
{
	// Example input (half scale):
	// 0x89 FF 1D C0 00 10 99 33

	// Shift out everything but the sign bits
	// 0x01 01 00 01 00 00 01 00
	uint16x4_t high_bits = vreinterpret_u16_u8(vshr_n_u8(input, 7));

	// Merge the even lanes together with vsra. The '??' bytes are garbage.
	// vsri could also be used, but it is slightly slower on aarch64.
	// 0x??03 ??02 ??00 ??01
	uint32x2_t paired16 =
		vreinterpret_u32_u16(vsra_n_u16(high_bits, high_bits, 7));
	// Repeat with wider lanes.
	// 0x??????0B ??????04
	uint64x1_t paired32 =
		vreinterpret_u64_u32(vsra_n_u32(paired16, paired16, 14));
	// 0x??????????????4B
	uint8x8_t paired64 =
		vreinterpret_u8_u64(vsra_n_u64(paired32, paired32, 28));
	// Extract the low 8 bits from each lane and join.
	// 0x4B
	return paired64;
}

/* Example:
 *  old = { 0, 1, 1, 2 }
 *  new = { 2, 3, 3, 4 }
 *
 *  vecTmp = { 2, 2, 3, 3 }
 *  M = new & vecTmp = { 1, 0, 1, 0 } = 0b1010
 *  arr = uniqshuf + M = { 0x1, 0x3, 0xFF, 0xFF } (choose index i iff M[i] == 0,
 *  ensuring uniqueness) output += [arr[i] if i != 0xFF]
 */
static inline int store_unique(uint16x8_t old, uint16x8_t newval,
			       uint16_t *output)
{
	// rotate the concat of newval and old by 1
	uint8x16_t vec_tmp = vreinterpretq_u8_u16(vextq_u16(old, newval, 7));

	// we want an 8-bitmask where the ith bit denotes if the ith lane of
	// 16x8 register is 1 (representing whether or not we have a match)
	uint8x8_t cmp = vqmovn_u16(vceqq_u16(vec_tmp, newval));
	uint8x8_t tM = vmovmaskq_u8(cmp);

	uint8_t M = vget_lane_u8(tM, 0);
	int number_of_new_values = 8 - vget_lane_u8(vcnt_u8(tM), 0);

	uint8x16_t key = vld1q_u8((const uint8x16_t *)uniqshuf + M);
	uint8x16_t val = vqtbl1q_u8(vreinterpretq_u8_u16(newval), key);
	vst1q_u8((uint8x16_t *)output, val);
	return number_of_new_values;
}

#endif // NEON_UTILS

#endif
