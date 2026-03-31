// Case 1: other ⊆ loop
// other blocks: {2, 3, 4}
// loop blocks:  {1, 2, 3, 4, 5}
// → other is nested in loop

// Case 2: loop ⊆ other
// loop blocks:  {2, 3, 4}
// other blocks: {1, 2, 3, 4, 5}
// → loop is nested in other

// Case 3: Partial overlap or disjoint
// loop blocks:  {1, 2, 3}
// other blocks: {3, 4, 5}
// → No nesting relationship
