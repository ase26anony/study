(!MEM_P (target)  // If target is NOT a memory location
 || count <= 2    // OR we're accessing 2 or fewer elements
 || (tree_fits_uhwi_p (TYPE_SIZE (elttype))  // OR element size fits in unsigned HWI
     && (tree_to_uhwi (TYPE_SIZE (elttype)) * count  // AND total size (element_size * count)
         ...  // Some threshold check continues here
