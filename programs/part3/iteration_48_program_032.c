   for (int i = 0; i < n; ++i) {
       if (some_condition(i)) {
           val = 1; // Path A
       } else {
           val = 0; // Path B
           // do work (moved inside else branch)
       }
   }
