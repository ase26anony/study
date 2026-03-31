if (a_high.sgt (max_r)  // if a_high is signed greater than max_r (0)
    || (a_high == max_r &&  // OR if a_high equals 0
        a_low.ugt (max_s)))  // AND a_low is unsigned greater than max_s
