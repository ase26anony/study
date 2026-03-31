val1 = some_computation()    // if branch
val2 = another_computation() // else branch
val3 = φ(val1, val2)         // phi node at merge point
if (val3 == 0) ...           // use of phi result
