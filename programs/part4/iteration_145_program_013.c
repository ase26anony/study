// Possible reordering without barrier:
int t1 = b + 1;    // a = b + 1
int t2 = t1 * 2;   // c = a * 2 (moved up!)
a = t1;            // Actually store a
b = t2 - t1;       // b = c - a
c = t2;            // Actually store c
