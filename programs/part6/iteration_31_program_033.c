max_r.high = 0;
max_r.low = 0;          // max_r = 0
max_s.high = -1;
max_s.low = -1;         // max_s = all 1's (maximum unsigned value)
max_s = max_s.zext(i_f_bits);  // Zero-extend to i_f_bits width
