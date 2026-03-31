   max_s.high = -1;
   max_s.low = -1;  // All bits set to 1 (maximum unsigned value)
   max_s = max_s.zext(i_f_bits);  // Zero-extend to i_f_bits width
