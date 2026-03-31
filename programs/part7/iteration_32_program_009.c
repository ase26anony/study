asm volatile("adds %0, %1, %2 \n\t"   // add and set flags
             "adc %0, %0, %3"         // add with carry
             : "=r"(out0)
             : "r"(in0), "r"(in1), "r"(carry_in));
