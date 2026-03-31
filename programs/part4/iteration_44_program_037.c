// This would compile with a warning but is dangerous:
struct S11 s11;
struct S10 s10;
copy(&s10, (struct S10*)&s11);  // Buffer overflow - copying 11 bytes into 10
