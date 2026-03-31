"movl %1, %0"  // 32-bit move
: "=r"(output)  // Output constraint
: "m"(input_array[complex_index])  // Input constraint
