/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper functions to force call instructions */
__attribute__((noinline)) int helper1(int a, int b) {
    return a * b + 123;
}

__attribute__((noinline)) int helper2(int a, int b, int c) {
    return (a ^ b) | c;
}

__attribute__((noinline)) int helper3(int a, int b, int c, int d) {
    return (a + b) * (c - d);
}

__attribute__((noinline)) int helper4(int a, int b, int c, int d, int e) {
    return (a * b) + (c * d) - e;
}

/* Volatile function pointer to create unpredictable call site */
static int (*volatile volatile_func)(int, int, int, int, int) = helper4;

/* Test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x55AA;
    int d = seed - 100;
    int e = seed << 3;
    int f = seed >> 2;
    
    /* First computation using multiple registers */
    a = b * c + d;
    e = a ^ f;
    
    /* First call - clobbers caller-saved registers */
    int r1 = helper1(a, b);
    
    /* Intervening computation keeping values live in registers */
    c = r1 * d + e;
    f = a + b + c;
    
    /* Second call with different arguments */
    int r2 = helper2(c, d, e);
    
    /* More computations creating dependencies */
    a = r2 ^ f;
    b = c * d - e;
    
    /* Third call with more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* Complex computation chain */
    d = r3 + a * b;
    e = (c ^ d) | f;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r4 = volatile_func(a, b, c, d, e);
    
    /* Final computation using all values to prevent dead code elimination */
    int result = (r1 ^ r2) + (r3 * r4) - (a & b) | (c ^ d) & e;
    
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
