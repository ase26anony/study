   if (a_high.sgt (max_r)  // signed greater than max_r
       || (a_high == max_r &&
           a_low.ugt (max_s)))  // unsigned greater than max_s
