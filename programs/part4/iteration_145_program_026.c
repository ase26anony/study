// Possible compiler-optimized version (if allowed)
int t1 = b + 1;  // a = b + 1
int t2 = t1 * 2; // c = a * 2 (moved up!)
a = t1;
c = t2;
b = c - a;
