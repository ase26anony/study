// For adding two 64-bit numbers on 32-bit ARM with carry
uint32_t a_lo, a_hi, b_lo, b_hi;
uint32_t result_lo, result_hi;
uint32_t carry;

asm volatile (
    "adds %0, %2, %4\n\t"    // Add low words, set flags
    "adc %1, %3, %5"         // Add high words with carry
    : "=r"(result_lo), "=r"(result_hi)
    : "r"(a_lo), "r"(a_hi), "r"(b_lo), "r"(b_hi)
    : "cc"  // Clobber condition codes
);
