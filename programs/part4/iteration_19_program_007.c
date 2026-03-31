// Loop A blocks: {1, 2, 3, 4, 5}
// Loop B blocks: {3, 4}

// bitmap_intersect_compl_p(B, A) = false 
// (B has no blocks outside A)
// Result: A.loops.push(B) - B is nested in A
