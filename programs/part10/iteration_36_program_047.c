if (a_high.sgt (max_r)           // If high part is signed greater than max_r.high
    || (a_high == max_r &&       // OR high parts are equal
        a_low.ugt (max_s)))      // AND low part is unsigned greater than max_s.low
