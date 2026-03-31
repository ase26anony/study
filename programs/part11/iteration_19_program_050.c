/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper functions to force call instructions */
__attribute__((noinline)) int helper1(int a, int b) {
    return a * b + 7;
}

__attribute__((noinline)) int helper2(int a, int b, int c) {
    return (a ^ b) | c;
}

__attribute__((noinline)) int helper3(int a, int b, int c, int d) {
    return (a + b) * (c - d);
}

__attribute__((noinline)) int helper4(int a, int b) {
    return (a << 3) | (b & 0xFF);
}

/* Volatile function pointer to create unpredictable call site */
static int (*volatile volatile_func)(int, int, int) = NULL;

/* Main test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3 + 1;
    int b = seed * 5 - 2;
    int c = seed * 7 + 3;
    int d = seed * 11 - 4;
    int e = seed * 13 + 5;
    int f = seed * 17 - 6;
    
    /* First computation using multiple registers */
    a = b * c + d;
    e = a ^ f;
    
    /* First call - clobbers call-clobbered registers */
    int r1 = helper1(a, b);
    
    /* Intervening computation keeping values live in registers */
    b = c * d + e;
    f = a ^ b ^ c;
    
    /* Second call with different arguments */
    int r2 = helper2(b, c, d);
    
    /* More register-intensive computations */
    c = d * e + f;
    a = b ^ c ^ d;
    
    /* Third call - creates another save/restore point */
    int r3 = helper3(c, d, e, f);
    
    /* Complex computation chain */
    d = e * f + a;
    b = c ^ d ^ e;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(a, b, c);
        d = d ^ r4;
    }
    
    /* Final computation using all values */
    e = f * a + b;
    c = d ^ e ^ f;
    
    /* Fourth call near the end of the basic block */
    int r5 = helper4(e, f);
    
    /* Use all computed values in final expression to prevent elimination */
    int result = (a + b) ^ (c - d) | (e * f) & (r1 + r2) ^ (r3 + r5);
    
    /* Additional computation to ensure BB_END isn't at the call */
    result = result * 2 + (seed & 1);
    
    return result;
}

/* Alternate helper to initialize volatile function pointer */
__attribute__((noinline)) int dummy_func(int x, int y, int z) {
    return x * y + z;
}

int main() {
    /* Initialize volatile function pointer sometimes */
    if (rand() % 2) {
        volatile_func = dummy_func;
    }
    
    int total = 0;
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
        total ^= i * 3;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
