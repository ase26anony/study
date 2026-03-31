uint32_t result, carry_in, carry_out;
uint32_t a, b;

// Add a + b + carry_in, result in result, carry out in carry_out
asm volatile("adds %0, %2, %3\n\t"   // add with flags set
             "adc %1, %1, #0"        // add carry to something (example)
             : "=r"(result), "=r"(carry_out)
             : "r"(a), "r"(b), "1"(carry_in)
             : "cc");
