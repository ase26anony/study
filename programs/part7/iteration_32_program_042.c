// For x86_64 (adding 2 values with carry)
asm volatile ("addc %1, %0"
              : "+r"(out0)
              : "r"(in0));

// Or for multiple additions
uint64_t result = in0 + in1 + in2 + in3 + in4 + in5 + in6 + in7 + in8;
