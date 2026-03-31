// Before optimization
int *ptr = ...;
int value = *ptr;
ptr = ptr + 0;  // Redundant, might get optimized

// Or looking for patterns that can use post-increment addressing
