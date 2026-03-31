struct S10 src = {"Hello"};
struct S10 dest;
copy(&dest, &src);
// Now dest.a contains "Hello"
