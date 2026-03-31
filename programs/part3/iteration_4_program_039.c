volatile int v = 0;
if (v) goto L1;
goto L2;
L1: a = b + c;
L2: d = e * f;
