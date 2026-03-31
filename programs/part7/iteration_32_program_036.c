uint32_t out0 = in0 + in1 + in2 + in3 + in4 + in5 + in6 + in7 + in8;
// Or with carry handling:
uint64_t sum64 = (uint64_t)in0 + in1 + in2 + in3 + in4 + in5 + in6 + in7 + in8;
uint32_t out0 = (uint32_t)sum64;
uint32_t carry = (uint32_t)(sum64 >> 32);
