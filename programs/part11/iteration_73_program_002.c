(!MEM_P (target)
 || count <= 2
 || (tree_fits_uhwi_p (TYPE_SIZE (elttype))
     && (tree_to_uhwi (TYPE_SIZE (elttype)) * count
