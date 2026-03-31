// Pattern being detected:
x_1 = phi(...);  // phi_stmt
x_2 = x_1;       // copy chain (traversed by while loop)
if (x_2 == 0)    // cmp_stmt with constant 0/1
  // ...
