#ifndef UTILS_H
#define UTILS_H

#include <arm_neon.h>
#include <stdlib.h>

int assert_equivalent(uint16x8_t v1, uint16x8_t v2);
uint16_t get_set_size(char *set_str);
uint16_t parse_set_string(uint16_t *buff, char *set_str);
void print_results(uint16_t *expected, uint16_t *received, uint16_t len);
int validate(uint16_t *expected, uint16_t *received, uint16_t len);
int compare_uints(const void *a, const void *b);
uint16_t unique(uint16_t *dst, uint16_t len);
uint16_t generate_random_size(uint16_t max_size);
int generate_sets(uint16_t *set1, uint16_t *n1, uint16_t *set2, uint16_t *n2,
		  uint16_t max_val);

#endif // UTILS_H
