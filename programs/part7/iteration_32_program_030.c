asm volatile ("addc %0, %1, %2\n\t"
              "addc %0, %0, %3\n\t"
              "addc %0, %0, %4\n\t"
              "addc %0, %0, %5\n\t"
              "addc %0, %0, %6\n\t"
              "addc %0, %0, %7\n\t"
              "addc %0, %0, %8\n\t"
              "addc %0, %0, %9"
              : "=r"(out0)
              : "r"(in0), "r"(in1), "r"(in2), "r"(in3), "r"(in4),
                "r"(in5), "r"(in6), "r"(in7), "r"(in8)
              : "cc");
