if (a_high.sgt(max_r)  // Signed greater than comparison
    || (a_high == max_r &&
        a_low.ugt(max_s)))  // Unsigned greater than comparison
