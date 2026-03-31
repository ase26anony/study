struct S10 x;
struct S11 y;
// copy(&x, (struct S10*)&y); // Dangerous! Would copy only first 10 bytes of y.a
