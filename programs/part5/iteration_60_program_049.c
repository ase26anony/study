// Case 1: other ⊆ loop
// other blocks: {1, 2, 3}
// loop blocks:  {1, 2, 3, 4, 5}
// Result: other added to loop->loops

// Case 2: loop ⊆ other
// other blocks: {1, 2, 3, 4, 5}
// loop blocks:  {1, 2, 3}
// Result: loop added to other->loops

// Case 3: Partial overlap
// other blocks: {1, 2, 4}
// loop blocks:  {2, 3, 4}
// Result: No relationship established
