// Example: Add 9 values with carry chain
uint64_t result = 0;
uint64_t carry = 0;

asm volatile (
    "adds %[result], %[in0], %[in1]\n\t"
    "adcs %[result], %[result], %[in2]\n\t"
    "adcs %[result], %[result], %[in3]\n\t"
    "adcs %[result], %[result], %[in4]\n\t"
    "adcs %[result], %[result], %[in5]\n\t"
    "adcs %[result], %[result], %[in6]\n\t"
    "adcs %[result], %[result], %[in7]\n\t"
    "adcs %[result], %[result], %[in8]\n\t"
    "adc %[carry], xzr, xzr"
    : [result] "=r"(result), [carry] "=r"(carry)
    : [in0] "r"(in0), [in1] "r"(in1), [in2] "r"(in2),
      [in3] "r"(in3), [in4] "r"(in4), [in5] "r"(in5),
      [in6] "r"(in6), [in7] "r"(in7), [in8] "r"(in8)
    : "cc"
);
