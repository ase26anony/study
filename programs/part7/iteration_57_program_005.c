__asm__ (
    "movl %1, %0"
    : "=r"(output)
    : "m"(input_array[complex_index])
    // No clobbers needed for simple move
);
