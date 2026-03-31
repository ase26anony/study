// For x86 (adding 2 numbers with carry)
uint64_t result, carry;
asm volatile ("add %2, %0\n"
              "adc $0, %1"
              : "+r"(result), "+r"(carry)
              : "r"(value));

// For ARM (adding 2 numbers with carry)
uint32_t result, carry;
asm volatile ("adds %0, %1, %2\n"
              "adc %3, %3, #0"
              : "=r"(result), "=r"(carry)
              : "r"(a), "r"(b));
