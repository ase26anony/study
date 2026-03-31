asm volatile ("custom_op %0, %1, %2, %3, %4, %5, %6, %7, %8, %9"
             : "=r"(out0), "=r"(out1), "=r"(out2)      // Output operands
             : "r"(in0), "r"(in1), "r"(in2), "r"(in3), // Input operands
               "r"(in4), "r"(in5), "r"(in6), "r"(in7));
