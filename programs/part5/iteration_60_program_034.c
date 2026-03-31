// Loop A blocks: {1, 2, 3, 4}
// Loop B blocks: {3, 4, 5, 6}

// They intersect (share blocks 3,4) → pass first check
// B has blocks {5,6} not in A → second condition FALSE
// A has blocks {1,2} not in B → third condition FALSE
// Result: No nesting relationship established
