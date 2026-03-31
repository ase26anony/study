volatile int v = 0;      // Declares v as volatile and initializes to 0
if (v) goto L1;          // Checks if v is non-zero, if true jumps to L1
goto L2;                 // Unconditional jump to L2 (executed when v == 0)
L1: a = b + c;           // Label L1: performs addition
L2: d = e * f;           // Label L2: performs multiplication
