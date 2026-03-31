if (a_high.sgt(max_r) ||  // Signed greater-than comparison of high parts
    (a_high == max_r &&   // If high parts are equal
     a_low.ugt(max_s)))   // Unsigned greater-than comparison of low parts
