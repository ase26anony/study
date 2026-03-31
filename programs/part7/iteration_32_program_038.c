// Example: Add 9 values with carry propagation
uint64_t result = 0;
uint64_t carry = 0;

// Add first two with carry
asm volatile("addc %0, %1, %2, %3"
             : "=r"(result), "=r"(carry)
             : "r"(in0), "r"(in1), "r"(0));  // initial carry = 0

// Then add the rest in sequence
asm volatile("addc %0, %1, %2, %3"
             : "=r"(result), "=r"(carry)
             : "r"(result), "r"(in2), "r"(carry));
// ... repeat for in3 through in8
