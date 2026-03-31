volatile int v = 0;      // v is declared as volatile
if (v) goto L1;          // Check if v is non-zero
goto L2;                 // If v is 0, jump to L2
L1: a = b + c;           // Label L1: execute addition
L2: d = e * f;           // Label L2: execute multiplication
