__asm__ volatile (
    "movl %1, %0"
    : "=r"(output)
    : "m"(input_array[complex_index])
);
