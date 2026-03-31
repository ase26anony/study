uint64_t sum_with_carry(uint64_t in0, uint64_t in1, uint64_t in2, uint64_t in3,
                        uint64_t in4, uint64_t in5, uint64_t in6, uint64_t in7,
                        uint64_t in8) {
    uint64_t result;
    uint64_t carry = 0;
    
    // Add pairs with carry propagation
    asm volatile (
        "adds %[result], %[in0], %[in1]\n\t"
        "adcs %[result], %[result], %[in2]\n\t"
        "adcs %[result], %[result], %[in3]\n\t"
        "adcs %[result], %[result], %[in4]\n\t"
        "adcs %[result], %[result], %[in5]\n\t"
        "adcs %[result], %[result], %[in6]\n\t"
        "adcs %[result], %[result], %[in7]\n\t"
        "adcs %[result], %[result], %[in8]"
        : [result] "=r" (result)
        : [in0] "r" (in0), [in1] "r" (in1), [in2] "r" (in2),
          [in3] "r" (in3), [in4] "r" (in4), [in5] "r" (in5),
          [in6] "r" (in6), [in7] "r" (in7), [in8] "r" (in8)
        : "cc"  // Condition codes clobbered
    );
    
    return result;
}
