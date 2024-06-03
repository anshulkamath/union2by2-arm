#include <arm_neon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int assert_equivalent(uint16x8_t v1, uint16x8_t v2)
{
	// xors all 8 lanes for v1 and v2, giving us 0 (if equal) or 0xFFFF
	// (if unequal). then performs saturating narrow, giving us a 64-bit
	// values, saturating for under/over flow. hence, zero stays 0, and
	// 0xFFFF becomes 0xFF. finally, we get the one (and only) lane of the
	// 64x1 vector and assert that it is 0, implying equality.
	uint16x4_t t = vqmovn_u16(veorq_u16(v1, v2));
	return vget_lane_u64(vreinterpret_u64_u16(t), 0) == 0;
}

uint16_t get_set_size(char *set_str)
{
	if (strlen(set_str) == 0) {
		return 0;
	}

	char *str_cpy[strlen(set_str) + 1];
	strcpy(str_cpy, set_str);

	strtok(str_cpy, " ");
	uint16_t counter = 0;
	do {
		counter++;
	} while (strtok(NULL, " "));

	return counter;
}

uint16_t parse_set_string(uint16_t *buff, char *set_str)
{
	if (strlen(set_str) == 0) {
		return 0;
	}

	char *str_cpy[strlen(set_str) + 1];
	strcpy(str_cpy, set_str);

	char *val = strtok(str_cpy, " ");
	uint16_t pos = 0;
	do {
		buff[pos++] = (uint16_t)atoi(val);
	} while ((val = strtok(NULL, " ")));

	return pos;
}

void print_results(uint16_t *expected, uint16_t *received, uint16_t len)
{
	printf("expected: {");
	for (uint16_t i = 0; i < len; i++) {
		printf("%x, ", expected[i]);
	}
	printf("}\n");

	printf("received: {");
	for (uint16_t i = 0; i < len; i++) {
		printf("%x, ", received[i]);
	}
	printf("}\n");
}

int validate(uint16_t *expected, uint16_t *received, uint16_t len)
{
	for (uint16_t i = 0; i < len; i++) {
		if (expected[i] != received[i]) {
			printf("[FAIL]\n");
			print_results(expected, received, len);
			return 1;
		}
	}
	printf("[PASS]\n");
	return 0;
}

uint16_t unique(uint16_t *dst, uint16_t len)
{
	uint16_t pos = 1;
	for (uint16_t i = 1; i < len; i++) {
		if (dst[i - 1] == dst[i]) {
			continue;
		}
		dst[pos++] = dst[i];
	}
	return pos;
}

int compare_uints(const void *a, const void *b)
{
	return (*(uint16_t *)a - *(uint16_t *)b);
}
