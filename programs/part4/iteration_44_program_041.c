struct S10 dest;
struct S11 src;
// This won't compile directly:
// copy(&dest, (struct S10*)&src);  // Dangerous cast!
