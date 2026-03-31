if (const_bounds_p  // 1. Bounds are constant
    && tree_fits_shwi_p (lo_index)  // 2. Lower bound fits in signed HWI
    && tree_fits_shwi_p (hi_index)  // 3. Upper bound fits in signed HWI
    && (lo = tree_to_shwi (lo_index),  // 4. Convert bounds to integers
        hi = tree_to_shwi (hi_index),
        count = hi - lo + 1,  // 5. Calculate element count
        (!MEM_P (target)  // 6. Either target is not a memory reference
         || count <= 2  // OR count is small (≤2 elements)
         || (tree_fits_uhwi_p (TYPE_SIZE (elttype))  // OR element size fits in unsigned HWI
             && (tree_to_uhwi (TYPE_SIZE (elttype)) * count  // AND total size calculation
                 ...))))  // Additional condition likely follows
