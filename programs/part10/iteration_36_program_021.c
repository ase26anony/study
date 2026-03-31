if (a_high.sgt (max_r)           // Case 1: a_high > max_r (signed comparison)
    || (a_high == max_r &&       // Case 2: a_high equals max_r AND
        a_low.ugt (max_s)))      //         a_low > max_s (unsigned comparison)
