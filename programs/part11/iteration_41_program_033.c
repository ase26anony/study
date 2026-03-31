   val_0 = 0  // Initial value
   for (i = 0; i < n; ++i) {
       val_phi = φ(val_0, val_2)  // Phi node at loop header
       if (some_condition(i)) {
           val_1 = 1;
       } else {
           val_2 = 0;
       }
       val_2 = φ(val_1, val_2)  // Another phi after the if-else
       if (val_2 == 1) {
           // do work
       }
   }
