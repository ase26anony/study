unsigned int a, b, carry_in, result, carry_out;

// Add a + b + carry_in
asm volatile ("addc %0, %1, %2"
              : "=r"(result), "=@ccc"(carry_out)
              : "r"(a), "r"(b), "@ccc"(carry_in));
