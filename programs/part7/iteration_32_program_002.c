unsigned int a, b, carry_in, result, carry_out;

asm volatile ("addc %0, %1, %2"
              : "=r"(result), "=@ccc"(carry_out)
              : "r"(a), "r"(b), "r"(carry_in));
