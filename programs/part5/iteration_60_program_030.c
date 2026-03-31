// Loop A: blocks {1, 2, 3, 4}
// Loop B: blocks {2, 3}

// First check: They intersect (blocks 2,3) → don't skip
// Second check: B has no blocks outside A → B is nested in A
// Result: A.loops.push(B)
