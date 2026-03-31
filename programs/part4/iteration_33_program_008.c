// Loop A: blocks {1, 2, 3, 4}
// Loop B: blocks {2, 3}

// Condition 1: They intersect (share blocks 2,3) → don't continue
// Condition 2: All of B's blocks {2,3} are in A's blocks {1,2,3,4} → true
// Result: B is added as nested loop inside A
