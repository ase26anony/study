val1 = some_computation()    // Branch 1
val2 = another_computation() // Branch 2
val = φ(val1, val2)          // Merge point: phi node
if (val == 0) ...            // Use the merged value
