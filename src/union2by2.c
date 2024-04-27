#include <arm_neon.h>

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
static inline void neon_merge(const uint16x8_t vInput1, const uint16x8_t vInput2,
                             uint16x8_t *vecMin, uint16x8_t *vecMax) {
  uint16x8_t vecTmp;
  vecTmp = vminq_u16(vInput1, vInput2);
  *vecMax = vmaxq_u16(vInput1, vInput2);
  vecTmp = vextq_u8(vecTmp, vecTmp, 2);
  *vecMin = vminq_u16(vecTmp, *vecMax);
  *vecMax = vmaxq_u16(vecTmp, *vecMax);
  vecTmp = vextq_u8(*vecMin, *vecMin, 2);
  *vecMin = vminq_u16(vecTmp, *vecMax);
  *vecMax = vmaxq_u16(vecTmp, *vecMax);
  vecTmp = vextq_u8(*vecMin, *vecMin, 2);
  *vecMin = vminq_u16(vecTmp, *vecMax);
  *vecMax = vmaxq_u16(vecTmp, *vecMax);
  vecTmp = vextq_u8(*vecMin, *vecMin, 2);
  *vecMin = vminq_u16(vecTmp, *vecMax);
  *vecMax = vmaxq_u16(vecTmp, *vecMax);
  vecTmp = vextq_u8(*vecMin, *vecMin, 2);
  *vecMin = vminq_u16(vecTmp, *vecMax);
  *vecMax = vmaxq_u16(vecTmp, *vecMax);
  vecTmp = vextq_u8(*vecMin, *vecMin, 2);
  *vecMin = vminq_u16(vecTmp, *vecMax);
  *vecMax = vmaxq_u16(vecTmp, *vecMax);
  vecTmp = vextq_u8(*vecMin, *vecMin, 2);
  *vecMin = vminq_u16(vecTmp, *vecMax);
  *vecMax = vmaxq_u16(vecTmp, *vecMax);
  *vecMin = vextq_u8(*vecMin, *vecMin, 2);
}

