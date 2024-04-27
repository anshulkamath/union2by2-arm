import union_scalar

def neon_merge(vec_1: list, vec_2: list) -> tuple[list, list]:
    """
    performs a merge given two vectors of uint16s.
    we do this by creating a temp vector which maintains intermediate state.
    since SIMD only acts on lanes, we need to rotate temp vector for each lane
    to make sure that we get the smallest value in each lane, and split the vector
    properly.

    returns a tuple of (max, min)
    """
    def get_minmax_vecs(v1: list, v2: list) -> tuple[list, list]:
        l, g = [], []
        for x, y in zip(v1, v2):
            l.append(min(x, y))
            g.append(max(x, y))
        return l, g

    tmp, vmax = get_minmax_vecs(vec_1, vec_2)
    tmp = tmp[1:] + [tmp[0]]
    vmin, vmax = get_minmax_vecs(tmp, vmax)
    tmp = vmin[1:] + [vmin[0]]
    vmin, vmax = get_minmax_vecs(tmp, vmax)
    tmp = vmin[1:] + [vmin[0]]
    vmin, vmax = get_minmax_vecs(tmp, vmax)
    tmp = vmin[1:] + [vmin[0]]
    vmin, vmax = get_minmax_vecs(tmp, vmax)
    tmp = vmin[1:] + [vmin[0]]
    vmin, vmax = get_minmax_vecs(tmp, vmax)
    tmp = vmin[1:] + [vmin[0]]
    vmin, vmax = get_minmax_vecs(tmp, vmax)
    tmp = vmin[1:] + [vmin[0]]
    vmin, vmax = get_minmax_vecs(tmp, vmax)
    vmin = vmin[1:] + [vmin[0]]
    return vmax, vmin


def create_shuffle_matrix(debug=None):
    r = range(256)
    if debug:
        r = [debug]
    mat = []
    for i in r:
        skipped = 0
        for j in range(8):
            if (i >> j) & 1 == 1:
                skipped += 1
            else:
                mat.append(j)
                if debug: print(j)
        mat += [0xff] * skipped
    return mat

shuffle_mat = create_shuffle_matrix()

def store_unique(dst, curr, last):
    """
    writes the unique elements of `curr` into `dst` using `last` as
    reference. first, note that we can have *at most* 2 of the same
    element, since we are operating on the output of a 2-set merge.

    the algorithm is simple in concept: we shift the `curr` to the
    right by 1, and fill the LSB with the last element of `last`.
    this allows us to detect duplicates across boundaries. then, we
    do a bytewise comparison, and create a bitmask where 1 denotes a
    duplicated value. hence, we have a unique `M` value (the bitmask)
    that represents a duplication state. we precompute the sequence of
    shifts needed to generate a unique set using this `M`, and index
    into `new` based on this shuffle sequence. we append the elements
    to the `dst` and return the new number of elements.

    example:
    last = { -1, -1, -1, -1, -1, -1, -1, 3 } (only the last value matters)
    curr =    { 3, 4, 5, 6, 6, 7, 8, 9 }
    shifted = { 3, 3, 4, 5, 6, 6, 7, 8 }
    M = 0b10001000
    """
    shifted = [last[-1]]
    shifted.extend(curr[:-1])

    cmp_vec = "".join(['1' if x == y else '0' for (x, y) in zip(curr, shifted)][::-1])
    cmp_vec = f'0b{cmp_vec}'
    M = int(cmp_vec, 2)

    indexes = shuffle_mat[M*8: (M+1)*8]
    for idx in indexes:
        # if this is ever the case, we are done shifting
        if idx == 0xff:
            break
        dst.append(curr[idx])

    num_elems = 8
    while M:
        if M & 1 == 1:
            num_elems -= 1
        M >>= 1
    return num_elems

