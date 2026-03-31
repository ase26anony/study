volatile int v = 0;    // Declares a volatile integer v initialized to 0
if (v) goto L1;        // If v is non-zero, jump to label L1
goto L2;               // Otherwise, jump to label L2
L1: a = b + c;         // Label L1: add b and c, store in a
L2: d = e * f;         // Label L2: multiply e and f, store in d
