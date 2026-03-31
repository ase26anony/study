// Loop A: blocks {1, 2, 3, 4}
// Loop B: blocks {2, 3}

// bitmap_intersect_p returns true (they share blocks 2,3)
// bitmap_intersect_compl_p(other=B, loop=A): B has no blocks outside A → true
// Result: B added to A->loops (B is nested in A)
