/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline helper functions to force actual call instructions */
__attribute__((noinline)) int helper1(int a, int b) {
    return a + b;
}

__attribute__((noinline)) int helper2(int a, int b, int c) {
    return a * b - c;
}

__attribute__((noinline)) int helper3(int a, int b, int c, int d) {
    return (a ^ b) | (c & d);
}

__attribute__((noinline)) int helper4(int a, int b, int c, int d, int e) {
    return (a * b) + (c * d) - e;
}

/* Volatile function pointer to create unpredictable call site */
static int (*volatile volatile_func)(int, int, int, int, int) = helper4;

/* Main test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55;
    int d = seed - 23;
    int e = seed * seed;
    int f = seed | 0xFF;
    
    /* Complex arithmetic creating dependencies, forcing values to stay in registers */
    a = b * c + d;          /* Uses b, c, d -> result in a */
    e = a ^ f;              /* Uses a, f -> result in e */
    b = d << 2;             /* Uses d -> result in b */
    c = e + (a >> 1);       /* Uses e, a -> result in c */
    
    /* First call - clobbers call-clobbered registers */
    int r1 = helper1(a, b);
    
    /* More arithmetic between calls, keeping values live */
    d = r1 * c;             /* Uses r1, c -> result in d */
    f = e ^ d;              /* Uses e, d -> result in f */
    a = b + (c << 1);       /* Uses b, c -> result in a */
    
    /* Second call with more arguments */
    int r2 = helper2(a, d, f);
    
    /* More computations */
    b = r2 ^ e;             /* Uses r2, e -> result in b */
    c = d * 3;              /* Uses d -> result in c */
    e = f + (a >> 2);       /* Uses f, a -> result in e */
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* Complex computation before volatile call */
    f = r3 * e;             /* Uses r3, e -> result in f */
    a = (b << 3) | (c & 0xF); /* Uses b, c -> result in a */
    d = e ^ f;              /* Uses e, f -> result in d */
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r4 = volatile_func(a, b, c, d, e);
    
    /* Final computation using all values to prevent elimination */
    int result = (r1 + r2) ^ (r3 * r4) | (a & b) + (c ^ d) - (e | f);
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
