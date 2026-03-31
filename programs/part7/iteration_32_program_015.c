asm volatile ("addc %0, %1, %2"
              : "=r"(out0)
              : "r"(in0), "r"(in1)
              : "cc");  // clobbers condition codes
