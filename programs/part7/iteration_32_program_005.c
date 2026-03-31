unsigned int a, b, carry_in, result, carry_out;
asm volatile ("addl %2, %0\n\t"
              "adcl $0, %1"
              : "=r"(result), "=r"(carry_out)
              : "r"(a), "0"(b), "1"(carry_in));
