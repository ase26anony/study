// Possible optimization without barrier:
int t1 = b + 1;    // b=2 → t1=3
int t2 = t1 * 2;   // t1=3 → t2=6  
int t3 = t2 - t1;  // t2=6, t1=3 → t3=3
a = t1;            // a=3
c = t2;            // c=6
b = t3;            // b=3
