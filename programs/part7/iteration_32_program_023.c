// For adding two 64-bit numbers on a 32-bit system
uint32_t a_lo, a_hi, b_lo, b_hi;
uint32_t result_lo, result_hi;
unsigned char carry;

asm volatile (
    "addc %0, %2, %4\n\t"    // Add low words with carry-in (0 initially)
    "addc %1, %3, %5"        // Add high words with carry from previous
    : "=r"(result_lo), "=r"(result_hi), "=@ccc"(carry)
    : "0"(a_lo), "1"(a_hi), "r"(b_lo), "r"(b_hi)
    : "cc"
);
