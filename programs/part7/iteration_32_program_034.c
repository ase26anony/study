// Example: 64-bit addition with carry (if adding very large numbers)
uint64_t a, b, c, result;
unsigned char carry_in, carry_out;

asm volatile (
    "add %[result], %[a], %[b]\n\t"
    "adc %[result], %[result], %[c]"
    : [result] "=r" (result), "=@ccc" (carry_out)
    : [a] "r" (a), [b] "r" (b), [c] "r" (c), "@ccc" (carry_in)
    :
);
