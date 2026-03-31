// Loop A blocks: {1, 2, 3, 4}
// Loop B blocks: {3, 4, 5}

// They intersect (blocks 3,4), so first condition false
// B is NOT subset of A (B has block 5 not in A) → second condition false
// A is NOT subset of B (A has blocks 1,2 not in B) → third condition false
// Result: Neither loop contains the other (improper nesting)
