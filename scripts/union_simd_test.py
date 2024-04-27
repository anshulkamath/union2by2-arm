import union_simd
import sys

def test_neon_merge():
    print("test_sse_merge:")
    tests = [
        {
            'name': 'in_order',
            'arr1': [0, 1, 2, 3, 4, 5, 6, 7],
            'arr2': [8, 9, 10, 11, 12, 13, 14, 15],
            'expected_max': [8, 9, 10, 11, 12, 13, 14, 15],
            'expected_min': [0, 1, 2, 3, 4, 5, 6, 7],
        },
        {
            'name': 'reversed',
            'arr1': [8, 9, 10, 11, 12, 13, 14, 15],
            'arr2': [0, 1, 2, 3, 4, 5, 6, 7],
            'expected_max': [8, 9, 10, 11, 12, 13, 14, 15],
            'expected_min': [0, 1, 2, 3, 4, 5, 6, 7],
        },
        {
            'name': 'interwoven',
            'arr1': [0, 2, 4, 6, 8, 10, 12, 14],
            'arr2': [1, 3, 5, 7, 9, 11, 13, 15],
            'expected_max': [8, 9, 10, 11, 12, 13, 14, 15],
            'expected_min': [0, 1, 2, 3, 4, 5, 6, 7],
        },
        {
            'name': 'split',
            'arr1': [0, 1, 2, 3, 8, 9, 10, 11],
            'arr2': [4, 5, 6, 7, 12, 13, 14, 15],
            'expected_max': [8, 9, 10, 11, 12, 13, 14, 15],
            'expected_min': [0, 1, 2, 3, 4, 5, 6, 7],
        },
    ]

    for test in tests:
        print(f"testing {test['name']}:", end="")
        res_max, res_min = union_simd.neon_merge(test['arr1'], test['arr2'])
        if res_max != test['expected_max']:
            print(f"\t[FAIL].\n\texpected: {test['expected_max']}\n\tgot: {res_max}")
            sys.exit(1)
            
        if res_min != test['expected_min']:
            print(f"\t[FAIL].\n\texpected: {test['expected_min']}\n\tgot: {res_min}")
            sys.exit(1)
        print("\t[PASS].")
    print()

def test_store_unique():
    print("test_store_unique:")
    tests = [
        {
            'name': 'no_duplicates',
            'last': [-1, -1, -1, -1, -1, -1, -1, -1],
            'curr': [0, 1, 2, 3, 4, 5, 6, 7],
            'expected': [0, 1, 2, 3, 4, 5, 6, 7],
            'expected_n': 8,
        },
        {
            'name': 'one_duplicate_without_carry',
            'last': [-1, -1, -1, -1, -1, -1, -1, -1],
            'curr': [0, 0, 1, 2, 3, 4, 5, 6],
            'expected': [0, 1, 2, 3, 4, 5, 6],
            'expected_n': 7,
        },
        {
            'name': 'one_duplicate_with_carry',
            'last': [-1, -1, -1, -1, -1, -1, -1, 0],
            'curr': [0, 1, 2, 3, 4, 5, 6, 7],
            'expected': [1, 2, 3, 4, 5, 6, 7],
            'expected_n': 7,
        },
        {
            'name': 'some_duplicates_without_carry',
            'last': [-1, -1, -1, -1, -1, -1, -1, -1],
            'curr': [0, 1, 2, 2, 3, 3, 4, 5],
            'expected': [0, 1, 2, 3, 4, 5],
            'expected_n': 6,
        },
        {
            'name': 'some_duplicates_with_carry',
            'last': [-1, -1, -1, -1, -1, -1, -1, 0],
            'curr': [0, 1, 2, 2, 3, 3, 4, 5],
            'expected': [1, 2, 3, 4, 5],
            'expected_n': 5,
        },
        {
            'name': 'max_duplicates',
            'last': [-1, -1, -1, -1, -1, -1, -1, 0],
            'curr': [0, 1, 1, 2, 2, 3, 3, 4],
            'expected': [1, 2, 3, 4],
            'expected_n': 4,
        },
    ]

    for test in tests:
        print(f"testing {test['name']}:", end="")
        out = []
        num_elems = union_simd.store_unique(out, test['curr'], test['last'])

        if out != test['expected']:
            print(f"\t[FAIL].\n\texpected: {test['expected']}\n\tgot: {out}")
            sys.exit(1)

        if num_elems != test['expected_n']:
            print(f"\t[FAIL].\n\texpected: {test['expected_n']} elems\n\tgot: {num_elems} elems")
            sys.exit(1)

        print("\t[PASS].")
    print()

    
if __name__ == "__main__":
    test_neon_merge()
    test_store_unique()
