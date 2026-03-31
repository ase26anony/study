   max_r.high = 0; max_r.low = 0;        // max_r = 0
   max_s.high = -1; max_s.low = -1;      // max_s = -1 (all bits set)
   max_s = max_s.zext(i_f_bits);         // Zero-extend max_s to i_f_bits
