asm volatile ("adc %0, %1, %2"
              : "=r"(out0)
              : "r"(in0), "r"(in1), "r"(in2)  // Actually needs carry flag setup
              : "cc");
