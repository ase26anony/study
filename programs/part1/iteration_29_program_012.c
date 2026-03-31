if (a_high.sgt (max_r)  // If a_high > max_r (signed comparison)
    || (a_high == max_r &&  // OR if high parts equal
        a_low.ugt (max_s)))  // AND a_low > max_s (unsigned comparison)
