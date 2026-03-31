uint64_t result = 0;
uint64_t carry = 0;

// Add pairs with carry
asm volatile("addc %0, %1, %2" : "=r"(result) : "r"(in0), "r"(in1));
asm volatile("addc %0, %0, %2" : "+r"(result) : "r"(in2));
// ... continue for all inputs
