volatile int v = 0;    // Declare volatile variable v initialized to 0
if (v) goto L1;        // Check if v is non-zero (false since v = 0)
goto L2;               // Jump to L2 (this will execute)
L1: a = b + c;         // Label L1 - won't be reached
L2: d = e * f;         // Label L2 - will execute
