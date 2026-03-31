struct S10 dest;
struct S11 src;
// This would be a type error:
// copy(&dest, (struct S10*)&src);  // Dangerous! Only copies 10 of 11 bytes
