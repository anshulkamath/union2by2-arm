#include "union2by2.h"

#include <stdio.h>
#include <time.h>

#include "utils.h"

#define BATCH_SIZE 100

int run_benchmark(uint16_t *set1, uint16_t n1, uint16_t *set2, uint16_t n2)
{
	uint16_t scalar[n1 + n2];
	uint16_t vectorized[n1 + n2];
	uint16_t scalar_ret, vector_ret;
	
	size_t count;
	clock_t start, end;

	count = 0;
	start = clock();
	while (1) {
		for (int batch = 0; batch < BATCH_SIZE; batch++) {
			scalar_ret = union2by2_scalar(scalar, set1, n1, set2, n2);
		}

		count += BATCH_SIZE;

		end = clock();
		if (end - start > CLOCKS_PER_SEC) {
			size_t ops = count * CLOCKS_PER_SEC / (end - start);
			printf("scalar: %zu ops\n", ops);
			break;
		}
	}

	count = 0;
	start = clock();
	while (1) {
		for (int batch = 0; batch < BATCH_SIZE; batch++) {
			vector_ret = union2by2_vectorized(vectorized, set1, n1, set2, n2);
		}

		count += BATCH_SIZE;

		end = clock();
		if (end - start > CLOCKS_PER_SEC) {
			size_t ops = count * CLOCKS_PER_SEC / (end - start);
			printf("vectorized: %zu ops\n", ops);
			break;
		}
	}

	printf("validation check: ");
	if (scalar_ret != vector_ret) {
		printf("[FAIL]\n");
		return 1;
	}

	for (uint16_t i = 0; i < scalar_ret; i++) {
		if (scalar[i] != vectorized[i]) {
			printf("[FAIL]\n");
			return 1;
		}
	}

	printf("[PASS]\n");

	return 0;
}

int compare_union2by2_random(void)
{
	typedef struct benchmark_s {
		char *name;
		uint16_t s1;
		uint16_t s2;
		uint16_t max_val;
	} benchmark_t;

	benchmark_t benchmarks[] = {
		{
			.name = "benchmark_1",
			.s1 = 10000,
			.s2 = 10000,
			.max_val = UINT16_MAX,
		},
	};

	const int N_BENCHMARKS = sizeof(benchmarks) / sizeof(benchmark_t);
	for (int i = 0; i < N_BENCHMARKS; i++) {
		benchmark_t b = benchmarks[i];

		if (b.s1 == 0) {
			b.s1 = generate_random_size(b.max_val);
		}

		if (b.s2 == 0) {
			b.s2 = generate_random_size(b.max_val);
		}

		uint16_t set1[b.s1];
		uint16_t set2[b.s2];

		if (generate_sets(set1, &b.s1, set2, &b.s2, b.max_val)) {
			return 1;
		}

		printf("running benchmark: %s/%hu_by_%hu\n", b.name, b.s1, b.s2);
		if (run_benchmark(set1, b.s1, set2, b.s2)) {
			return 1;
		}
	}

	return 0;
}

int main(void)
{
	int pass = 0;
	pass |= compare_union2by2_random();

	return pass;
}
