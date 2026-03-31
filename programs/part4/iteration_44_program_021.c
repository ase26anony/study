struct S10 dest;
struct S11 src;
// This would be a type error:
// copy(&dest, (struct S10*)&src); // Dangerous but possible with cast
