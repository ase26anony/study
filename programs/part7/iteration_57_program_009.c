"movl %1, %0"  // 32-bit move
: "=r"(output)  // output in any general register
: "m"(input_array[complex_index])  // memory operand
