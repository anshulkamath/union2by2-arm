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

    
if __name__ == "__main__":
    test_neon_merge()
