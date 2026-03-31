   max_r.high = 0; max_r.low = 0;        // max_r = 0
   max_s.high = -1; max_s.low = -1;      // max_s = -1 (all bits set)
   max_s = max_s.zext(i_f_bits);         // Zero-extend max_s to i_f_bits width
   
   min_r.high = -1; min_r.low = -1;      // min_r = -1 (all bits set)
   min_s.high = 0; min_s.low = 1;        // min_s = 1
   min_s = min_s.alshift(i_f_bits, HOST_BITS_PER_DOUBLE_INT);  // Left shift by i_f_bits
   min_s = min_s.sext(1 + i_f_bits);     // Sign-extend to 1 + i_f_bits width
