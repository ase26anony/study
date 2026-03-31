if (const_bounds_p
    && tree_fits_shwi_p (lo_index)
    && tree_fits_shwi_p (hi_index)
    && (lo = tree_to_shwi (lo_index),
        hi = tree_to_shwi (hi_index),
        count = hi - lo + 1,
        (!MEM_P (target)
         || count <= 2
         || (tree_fits_uhwi_p (TYPE_SIZE (elttype))
             && (tree_to_uhwi (TYPE_SIZE (elttype)) * count
