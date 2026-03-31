struct S10 src = {"Hello"};
struct S10 dest;
copy(&dest, &src);  // dest.a now contains "Hello"
