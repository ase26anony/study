   max_r.high = 0;
   max_r.low = 0;
   max_s.high = -1;  // All bits set to 1 (assuming two's complement)
   max_s.low = -1;
   max_s = max_s.zext(i_f_bits);  // Zero-extend to i_f_bits width
