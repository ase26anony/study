uint32_t out0, out1, out2, out3;
uint32_t in0, in1, in2, in3, in4, in5, in6, in7, in8;

// Add first pair
asm volatile("add %0, %1, %2" : "=r"(out0) : "r"(in0), "r"(in1));

// Add remaining with carry
asm volatile("adc %0, %1, %2" : "=r"(out1) : "r"(out0), "r"(in2));
asm volatile("adc %0, %1, %2" : "=r"(out2) : "r"(out1), "r"(in3));
// ... and so on
