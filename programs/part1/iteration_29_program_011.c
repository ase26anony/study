   max_s.high = -1;
   max_s.low = -1;  // This is all 1's in two's complement (maximum unsigned value)
   max_s = max_s.zext(i_f_bits);  // Zero-extend by i_f_bits
