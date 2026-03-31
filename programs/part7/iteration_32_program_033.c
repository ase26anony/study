// C version - much clearer
out0 = in0 + in1 + in2 + in3 + in4 + in5 + in6 + in7 + in8;

// Or if you need carry from previous operation:
uint64_t carry = 0;
uint64_t sum = 0;
sum = in0 + in1; carry = sum < in0;
sum = sum + in2; carry |= sum < in2;
// ... continue for all inputs
out0 = sum;
