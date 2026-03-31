uint64_t result[2]; // For 128-bit result
uint32_t in0, in1, in2, in3, in4, in5, in6, in7, in8;

// For adding multiple 32-bit values with carry propagation
asm volatile (
    "adds %0, %2, %3\n\t"      // Add first two with flags
    "adcs %0, %0, %4\n\t"      // Add with carry
    "adcs %0, %0, %5\n\t"      // Add with carry
    "adcs %0, %0, %6\n\t"      // Add with carry
    "adcs %0, %0, %7\n\t"      // Add with carry
    "adcs %0, %0, %8\n\t"      // Add with carry
    "adcs %0, %0, %9\n\t"      // Add with carry
    "adcs %0, %0, %10\n\t"     // Add with carry
    "adc %1, xzr, xzr"         // Capture final carry
    : "=r"(result[0]), "=r"(result[1])
    : "r"(in0), "r"(in1), "r"(in2), "r"(in3), "r"(in4),
      "r"(in5), "r"(in6), "r"(in7), "r"(in8)
    : "cc"
);
