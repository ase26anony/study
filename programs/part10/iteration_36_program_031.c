if (a_high.sgt (max_r)  // signed greater than comparison
    || (a_high == max_r &&
        a_low.ugt (max_s)))  // unsigned greater than comparison
