// Example for adding two 256-bit numbers (8x32-bit)
uint32_t a[8], b[8], result[8];
uint32_t carry = 0;

for (int i = 0; i < 8; i++) {
    asm volatile ("adds %0, %1, %2"
                  : "=r"(result[i])
                  : "r"(a[i]), "r"(b[i]));
    asm volatile ("adc %0, %0, #0"
                  : "+r"(carry));
}
