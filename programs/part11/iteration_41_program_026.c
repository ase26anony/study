   val_0 = 0
   for (i_0 = 0; i_0 < n; i_1 = i_0 + 1) {
     val_1 = φ(val_0, val_3)  // Phi node: from initial value or previous iteration
     if (some_condition(i_1)) {
       val_2 = 1;
     } else {
       val_3 = 0;
     }
     val_4 = φ(val_2, val_3)  // Phi node merging both branches
     if (val_4 == 1) {
       // do work
     }
     val_0 = val_4  // For next iteration
   }
