/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline helper functions to force actual calls */
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
    return (a + b) * (c - d) ^ e;
}

/* Volatile function pointer to create unpredictable call site */
static int (*volatile volatile_func)(int, int, int, int, int) = helper4;

/* Main test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55;
    int d = seed - 19;
    int e = seed * seed;
    int f = seed | 0xFF;
    
    /* First computation keeping values live across calls */
    a = b * c + d;
    b = a ^ f;
    
    /* First call - uses some registers, clobbers others */
    int r1 = helper1(a, b);
    
    /* More computations with dependencies */
    c = r1 * d + e;
    d = c ^ a;
    e = b + d * 2;
    
    /* Second call with more arguments */
    int r2 = helper2(c, d, e);
    
    /* Complex computation chain */
    f = r2 * a + b;
    a = f ^ c;
    b = d * e + r1;
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* More register-intensive operations */
    c = r3 + f * 2;
    d = a ^ b ^ c;
    e = r2 * 3 + d;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r4 = volatile_func(a, b, c, d, e);
    
    /* Final computation using all values to prevent dead code elimination */
    int result = (a * b) + (c ^ d) - (e & f) + (r1 | r2) * (r3 ^ r4);
    
    return result;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Call test function multiple times with different inputs
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
