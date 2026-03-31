// For adding 9 values with carry chain
uint64_t sum = in0;
sum = __builtin_add_overflow(sum, in1, &sum);  // With carry handling
// ... repeat for in2..in8
