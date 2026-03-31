uint64_t sum_with_carry(uint64_t in0, uint64_t in1, uint64_t in2, uint64_t in3,
                        uint64_t in4, uint64_t in5, uint64_t in6, uint64_t in7,
                        uint64_t in8) {
    uint64_t result;
    uint64_t carry = 0;
    
    // Add pairs with carry
    asm volatile("add %0, %1, %2\n\t"
                 "addc %0, %0, %3\n\t"
                 "addc %0, %0, %4\n\t"
                 // ... more adds as needed
                 : "=r"(result)
                 : "r"(in0), "r"(in1), "r"(in2), "r"(in3)
                 : "cc");
    return result;
}
