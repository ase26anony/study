"movl %1, %0"  // 32-bit move instruction
: "=r"(output)  // Output operand - expects a register
: "m"(input_array[complex_index])  // Input operand - memory address
