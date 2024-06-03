#include "union2by2.h"

#if USE_AVX

#ifndef AVX_UTILS
#define AVX_UTILS

#include <immintrin.h> // for most things (AVX2, AVX512, _popcnt64)
#include <smmintrin.h>
#include <tmmintrin.h>

#include "shuffle_mat.c"

// Assuming that vInput1 and vInput2 are sorted, produces a sorted output going
// from vecMin all the way to vecMax
// developed originally for merge sort using SIMD instructions.
// Standard merge. See, e.g., Inoue and Taura, SIMD- and Cache-Friendly
// Algorithm for Sorting an Array of Structures
static inline void merge(const __m128i vInput1,
			 const __m128i vInput2, // input 1 & 2
			 __m128i *vecMin, __m128i *vecMax)
{ // output
	__m128i vecTmp;
	vecTmp = _mm_min_epu16(vInput1, vInput2);
	*vecMax = _mm_max_epu16(vInput1, vInput2);
	vecTmp = _mm_alignr_epi8(vecTmp, vecTmp, 2);
	*vecMin = _mm_min_epu16(vecTmp, *vecMax);
	*vecMax = _mm_max_epu16(vecTmp, *vecMax);
	vecTmp = _mm_alignr_epi8(*vecMin, *vecMin, 2);
	*vecMin = _mm_min_epu16(vecTmp, *vecMax);
	*vecMax = _mm_max_epu16(vecTmp, *vecMax);
	vecTmp = _mm_alignr_epi8(*vecMin, *vecMin, 2);
	*vecMin = _mm_min_epu16(vecTmp, *vecMax);
	*vecMax = _mm_max_epu16(vecTmp, *vecMax);
	vecTmp = _mm_alignr_epi8(*vecMin, *vecMin, 2);
	*vecMin = _mm_min_epu16(vecTmp, *vecMax);
	*vecMax = _mm_max_epu16(vecTmp, *vecMax);
	vecTmp = _mm_alignr_epi8(*vecMin, *vecMin, 2);
	*vecMin = _mm_min_epu16(vecTmp, *vecMax);
	*vecMax = _mm_max_epu16(vecTmp, *vecMax);
	vecTmp = _mm_alignr_epi8(*vecMin, *vecMin, 2);
	*vecMin = _mm_min_epu16(vecTmp, *vecMax);
	*vecMax = _mm_max_epu16(vecTmp, *vecMax);
	vecTmp = _mm_alignr_epi8(*vecMin, *vecMin, 2);
	*vecMin = _mm_min_epu16(vecTmp, *vecMax);
	*vecMax = _mm_max_epu16(vecTmp, *vecMax);
	*vecMin = _mm_alignr_epi8(*vecMin, *vecMin, 2);
}

static inline int store_unique(__m128i old, __m128i newval, uint16_t *output)
{
	__m128i vecTmp = _mm_alignr_epi8(newval, old, 16 - 2);
	// lots of high latency instructions follow (optimize?)
	int M = _mm_movemask_epi8(_mm_packs_epi16(
		_mm_cmpeq_epi16(vecTmp, newval), _mm_setzero_si128()));
	int numberofnewvalues = 8 - _mm_popcnt_u32(M);
	__m128i key = _mm_lddqu_si128((const __m128i *)uniqshuf + M);
	__m128i val = _mm_shuffle_epi8(newval, key);
	_mm_storeu_si128((__m128i *)output, val);
	return numberofnewvalues;
}

#endif // AVX_UTILS

#endif // USE_AVX
