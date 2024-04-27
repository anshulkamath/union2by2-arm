// need to include C file here because a lot of the helper functions are
// defined with internal linkage, so we can't test them otherwise.

#include "union2by2.c"

#include <arm_neon.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int assert_equivalent(uint16x8_t v1, uint16x8_t v2) {
  // xors all 8 lanes for v1 and v2, giving us 0 (if equal) or 0xFFFF
  // (if unequal). then performs saturating narrow, giving us a 64-bit
  // values, saturating for under/over flow. hence, zero stays 0, and
  // 0xFFFF becomes 0xFF. finally, we get the one (and only) lane of the
  // 64x1 vector and assert that it is 0, implying equality.
  uint16x4_t t = vqmovn_u16(veorq_u16(v1, v2));
  return vget_lane_u64(vreinterpret_u64_u16(t), 0) == 0;
}

int neon_merge_test(void) {
  typedef struct test_case_s {
    char *name;
    uint16_t arr1[8];
    uint16_t arr2[8];
    uint16_t e_min[8];
    uint16_t e_max[8];
  } test_case_t;

  test_case_t tests[] = {
      {
          .name = "straight",
          .arr1 = {0, 1, 2, 3, 4, 5, 6, 7},
          .arr2 = {8, 9, 10, 11, 12, 13, 14, 15},
          .e_min = {0, 1, 2, 3, 4, 5, 6, 7},
          .e_max = {8, 9, 10, 11, 12, 13, 14, 15},
      },
      {
          .name = "reversed",
          .arr1 = {8, 9, 10, 11, 12, 13, 14, 15},
          .arr2 = {0, 1, 2, 3, 4, 5, 6, 7},
          .e_min = {0, 1, 2, 3, 4, 5, 6, 7},
          .e_max = {8, 9, 10, 11, 12, 13, 14, 15},
      },
      {
          .name = "interleaved",
          .arr1 = {0, 2, 4, 6, 8, 10, 12, 14},
          .arr2 = {1, 3, 5, 7, 9, 11, 13, 15},
          .e_min = {0, 1, 2, 3, 4, 5, 6, 7},
          .e_max = {8, 9, 10, 11, 12, 13, 14, 15},
      },
  };

  printf("testing %s:\n", __FUNCTION__);
  int N = sizeof(tests) / sizeof(test_case_t);
  for (int i = 0; i < N; i++) {
    test_case_t t = tests[i];
    printf("%s/%s: ", __FUNCTION__, t.name);

    uint16x8_t v1, v2, v_min, v_max, ve_min, ve_max;

    v1 = vld1q_u16(t.arr1);
    v2 = vld1q_u16(t.arr2);
    ve_min = vld1q_u16(t.e_min);
    ve_max = vld1q_u16(t.e_max);

    neon_merge(v1, v2, &v_min, &v_max);
    if (!assert_equivalent(ve_min, v_min) ||
        !assert_equivalent(ve_max, v_max)) {
      printf("[FAIL]\n\n");
      return 1;
    }
    printf("[PASS]\n");
  }
  printf("\n");
  return 0;
}

int store_unique_test(void) {
  typedef struct test_case_s {
    char *name;
    uint16_t last_store[8];
    uint16_t vec_min[8];
    uint16_t expected[8];
  } test_case_t;

  test_case_t tests[] = {
      {
          .name = "cold_start_no_copies",
          .last_store = {-1, -1, -1, -1, -1, -1, -1, -1},
          .vec_min = {0, 1, 2, 3, 4, 5, 6, 7},
          .expected = {0, 1, 2, 3, 4, 5, 6, 7},
      },
      {
          .name = "cold_start_some_copies",
          .last_store = {-1, -1, -1, -1, -1, -1, -1, -1},
          .vec_min = {0, 0, 1, 1, 2, 2, 3, 3},
          .expected = {0, 1, 2, 3, 0, 0, 0, 0},
      },
      {
          .name = "warm_start_no_copies",
          .last_store = {0, 1, 2, 3, 4, 5, 6, 7},
          .vec_min = {8, 9, 10, 11, 12, 13, 14, 15},
          .expected = {8, 9, 10, 11, 12, 13, 14, 15},
      },
      {
          .name = "warm_start_some_copies",
          .last_store = {0, 0, 1, 1, 2, 2, 3, 3},
          .vec_min = {3, 3, 4, 4, 5, 5, 6, 6},
          .expected = {4, 5, 6, 0, 0, 0, 0, 0},
      },
      {
          .name = "warm_start_all_copies",
          .last_store = {0, 0, 1, 1, 2, 2, 3, 3},
          .vec_min = {4, 4, 5, 5, 6, 6, 7, 7},
          .expected = {4, 5, 6, 7, 0, 0, 0, 0},
      },
  };

  printf("testing %s:\n", __FUNCTION__);
  int N = sizeof(tests) / sizeof(test_case_t);
  for (int i = 0; i < N; i++) {
    test_case_t t = tests[i];
    printf("%s/%s: ", __FUNCTION__, t.name);

    uint16x8_t last_store, vec_min;
    uint16_t result[8];
    memset(result, -1, 8);

    last_store = vld1q_u16(t.last_store);
    vec_min = vld1q_u16(t.vec_min);
    store_unique(last_store, vec_min, result);

    for (size_t i = 0; i < 8; i++) {
      if (t.expected[i] != result[i]) {
        printf("[FAIL]\n\n");
        return 1;
      }
    }
    printf("[PASS]\n");
  }
  printf("\n");
  return 0;
}

int main(void) {
  int pass = 0;

  pass |= neon_merge_test();
  pass |= store_unique_test();

  return pass;
}
