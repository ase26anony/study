if (a_high.sgt(max_r) ||  // Signed greater than comparison
    (a_high == max_r &&   // If high parts are equal
     a_low.ugt(max_s)))   // Check if low part is unsigned greater than
