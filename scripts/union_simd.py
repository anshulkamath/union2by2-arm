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

