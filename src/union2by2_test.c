// need to include C file here because a lot of the helper functions are
// defined with internal linkage, so we can't test them otherwise.

#include "union2by2.c"

#include <arm_neon.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

int neon_merge_test(void)
{
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
			.arr1 = { 0, 1, 2, 3, 4, 5, 6, 7 },
			.arr2 = { 8, 9, 10, 11, 12, 13, 14, 15 },
			.e_min = { 0, 1, 2, 3, 4, 5, 6, 7 },
			.e_max = { 8, 9, 10, 11, 12, 13, 14, 15 },
		},
		{
			.name = "reversed",
			.arr1 = { 8, 9, 10, 11, 12, 13, 14, 15 },
			.arr2 = { 0, 1, 2, 3, 4, 5, 6, 7 },
			.e_min = { 0, 1, 2, 3, 4, 5, 6, 7 },
			.e_max = { 8, 9, 10, 11, 12, 13, 14, 15 },
		},
		{
			.name = "interleaved",
			.arr1 = { 0, 2, 4, 6, 8, 10, 12, 14 },
			.arr2 = { 1, 3, 5, 7, 9, 11, 13, 15 },
			.e_min = { 0, 1, 2, 3, 4, 5, 6, 7 },
			.e_max = { 8, 9, 10, 11, 12, 13, 14, 15 },
		},
		{
			.name = "same",
			.arr1 = { 0, 1, 2, 3, 4, 5, 6, 7 },
			.arr2 = { 0, 1, 2, 3, 4, 5, 6, 7 },
			.e_min = { 0, 0, 1, 1, 2, 2, 3, 3 },
			.e_max = { 4, 4, 5, 5, 6, 6, 7, 7 },
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

		merge(v1, v2, &v_min, &v_max);
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

int store_unique_test(void)
{
	typedef struct test_case_s {
		char *name;
		uint16_t last_store[8];
		uint16_t vec_min[8];
		uint16_t expected[8];
		int number_new_values;
	} test_case_t;

	int rc = 0;

	test_case_t tests[] = {
		{
			.name = "cold_start_no_copies",
			.last_store = { -1, -1, -1, -1, -1, -1, -1, -1 },
			.vec_min = { 0, 1, 2, 3, 4, 5, 6, 7 },
			.expected = { 0, 1, 2, 3, 4, 5, 6, 7 },
			.number_new_values = 8,
		},
		{
			.name = "cold_start_some_copies",
			.last_store = { -1, -1, -1, -1, -1, -1, -1, -1 },
			.vec_min = { 0, 0, 1, 1, 2, 2, 3, 3 },
			.expected = { 0, 1, 2, 3, 0, 0, 0, 0 },
			.number_new_values = 4,
		},
		{
			.name = "warm_start_no_copies",
			.last_store = { 0, 1, 2, 3, 4, 5, 6, 7 },
			.vec_min = { 8, 9, 10, 11, 12, 13, 14, 15 },
			.expected = { 8, 9, 10, 11, 12, 13, 14, 15 },
			.number_new_values = 8,
		},
		{
			.name = "warm_start_some_copies",
			.last_store = { 0, 0, 1, 1, 2, 2, 3, 3 },
			.vec_min = { 3, 3, 4, 4, 5, 5, 6, 6 },
			.expected = { 4, 5, 6, 0, 0, 0, 0, 0 },
			.number_new_values = 3,
		},
		{
			.name = "warm_start_all_copies",
			.last_store = { 0, 0, 1, 1, 2, 2, 3, 3 },
			.vec_min = { 4, 4, 5, 5, 6, 6, 7, 7 },
			.expected = { 4, 5, 6, 7, 0, 0, 0, 0 },
			.number_new_values = 4,
		},
	};

	printf("testing %s:\n", __FUNCTION__);
	int N = sizeof(tests) / sizeof(test_case_t);
	for (int i = 0; i < N; i++) {
		test_case_t t = tests[i];
		printf("%s/%s: ", __FUNCTION__, t.name);

		uint16x8_t last_store, vec_min;
		uint16_t result[8];
		int result_count;
		memset(result, -1, 8);

		last_store = vld1q_u16(t.last_store);
		vec_min = vld1q_u16(t.vec_min);
		result_count = store_unique(last_store, vec_min, result);

		if ((rc = t.number_new_values != result_count)) {
			printf("incorrect number of elements: expected: %d, received: %d\n",
			       t.number_new_values, result_count);
			break;
		}

		if ((rc = validate(t.expected, result, 8))) {
			break;
		}
	}

	printf("\n");
	return rc;
}

int union2by2_test(void)
{
	typedef struct test_case_s {
		char *name;
		char *set1;
		char *set2;
		char *expected;
	} test_case_t;

	typedef struct {
		char *name;
		int (*u2b2)(uint16_t *dst, uint16_t *src1, uint16_t len1,
			    uint16_t *src2, uint16_t len2);
	} union_func_t;

	int rc = 0;

	test_case_t tests[] = {
		{
			.name = "less_than_vector_size",
			.set1 = "0 1 2 3",
			.set2 = "4 5 6 7 8 9 10 11",
			.expected = "0 1 2 3 4 5 6 7 8 9 10 11",
		},
		{
			.name = "in_order",
			.set1 = "0 1 2 3 4 5 6 7",
			.set2 = "8 9 10 11 12 13 14 15",
			.expected = "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15",
		},
		{
			.name = "in_order",
			.set1 = "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15",
			.set2 = "16 17 18 19 20 21 22 23",
			.expected =
				"0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23",
		},
		{
			.name = "alternating",
			.set1 = "0 2 4 6 8 10 12 14",
			.set2 = "1 3 5 7 9 11 13 15",
			.expected = "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15",
		},
		{
			.name = "alternating",
			.set1 = "0 2 4 6 8 10 12 14 16 18 20 22",
			.set2 = "1 3 5 7 9 11 13 15 17 19 21 23",
			.expected =
				"0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23",
		},
		{
			.name = "alternating_step_2",
			.set1 = "0 1 4 5 8 9 12 13",
			.set2 = "2 3 6 7 10 11 14 15",
			.expected = "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15",
		},
		{
			.name = "alternating_step_2",
			.set1 = "0 1 4 5 8 9 12 13 16 17 20 21",
			.set2 = "2 3 6 7 10 11 14 15 18 19 22 23",
			.expected =
				"0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23",
		},
		{
			.name = "all_same",
			.set1 = "0 1 2 3 4 5 6 7",
			.set2 = "0 1 2 3 4 5 6 7",
			.expected = "0 1 2 3 4 5 6 7",
		},
		{
			.name = "overlap",
			.set1 = "0 1 2 3 4 5 6 7",
			.set2 = "4 5 6 7 8 9 10 11",
			.expected = "0 1 2 3 4 5 6 7 8 9 10 11",
		},
		{
			.name = "overlap",
			.set1 = "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15",
			.set2 = "12 13 14 15 16 17 18 19",
			.expected =
				"0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19",
		},
		{
			.name = "subset",
			.set1 = "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15",
			.set2 = "4 5 6 7 8 9 10 11 12 13",
			.expected = "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15",
		},
	};

	union_func_t funcs[] = {
		{ .name = "scalar", .u2b2 = union2by2_scalar },
		{ .name = "vectorized", .u2b2 = union2by2_vectorized },
	};

	int N_FUNCS = sizeof(funcs) / sizeof(union_func_t);
	int N_TESTS = sizeof(tests) / sizeof(test_case_t);

	printf("testing %s:\n", __FUNCTION__);
	for (int i = 0; i < N_FUNCS; i++) {
		union_func_t f = funcs[i];
		printf("Function: %s\n", f.name);

		for (int j = 0; j < N_TESTS; j++) {
			test_case_t t = tests[j];

			uint16_t n1 = get_set_size(t.set1);
			uint16_t n2 = get_set_size(t.set2);
			uint16_t expected_size = get_set_size(t.expected);

			uint16_t set1[n1];
			parse_set_string(set1, t.set1);

			uint16_t set2[n2];
			parse_set_string(set2, t.set2);

			uint16_t expected[expected_size];
			parse_set_string(expected, t.expected);

			uint16_t result[n1 + n2];
			uint16_t result_size;
			memset(result, -1, n1 + n2);

			result_size = f.u2b2(result, set1, n1, set2, n2);

			printf("%s/%s/%u_by_%u/set_len: ", __FUNCTION__,
			       t.name, n1, n2);
			if ((rc = expected_size != result_size)) {
				printf("[FAIL]\n");
				printf("incorrect number of elements: expected: %u, received: %u\n",
				       expected_size, result_size);
				break;
			} else {
				printf("[PASS]\n");
			}

			printf("%s/%s/%u_by_%u/forward: ", __FUNCTION__,
			       t.name, n1, n2);
			if ((rc = validate(expected, result, result_size))) {
				break;
			}

			printf("%s/%s/%u_by_%u/reverse: ", __FUNCTION__,
			       t.name, n1, n2);
			f.u2b2(result, set2, n2, set1, n1);
			if ((rc = validate(expected, result, result_size))) {
				break;
			}
			printf("\n");
		}
	}

	return rc;
}

int main(void)
{
	int pass = 0;

	pass |= neon_merge_test();
	pass |= store_unique_test();
	pass |= union2by2_test();

	return pass;
}
