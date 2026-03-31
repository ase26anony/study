struct S10 src = {"Hello"};
struct S10 dst;
copy(&dst, &src); // dst.a now contains "Hello"
