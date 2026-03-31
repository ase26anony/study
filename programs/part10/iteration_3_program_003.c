// Loop A blocks: {1, 2, 3, 4}
// Loop B blocks: {3, 4, 5}

// They intersect (blocks 3,4), so first condition is false
// Check if B has blocks outside A: block 5 is outside A → second condition false
// Check if A has blocks outside B: blocks 1,2 are outside B → third condition false
// Result: No nesting relationship (overlapping but not nested)
