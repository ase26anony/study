val1 = some_computation()    // in "then" branch
val2 = another_computation() // in "else" branch
val3 = φ(val1, val2)         // at merge point
if (val3 == 0) ...           // uses the phi result
