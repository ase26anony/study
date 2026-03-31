/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline helper functions to force actual call instructions */
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

/* Test function with complex register usage across multiple calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3 + 1;
    int b = seed * 5 - 2;
    int c = seed * 7 + 3;
    int d = seed * 11 - 5;
    int e = seed * 13 + 7;
    int f = seed * 17 - 11;
    
    /* First computation using multiple registers */
    a = b * c + d;
    e = a ^ f;
    
    /* First call - clobbers call-clobbered registers */
    int r1 = helper1(a, b);
    
    /* Intervening computation keeping values live in registers */
    c = d * e + r1;
    f = a ^ c | d;
    
    /* Second call with different arguments */
    int r2 = helper2(c, d, e);
    
    /* More register-intensive computations */
    b = r1 * r2 + f;
    d = (a << 2) | (b & 0xF);
    
    /* Third call - more register clobbering */
    int r3 = helper3(a, b, c, d);
    
    /* Complex dependency chain */
    e = r1 ^ r2 ^ r3;
    f = (d * e) + (c << 1);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(e, f, a);
        b = r4 * 3;
    }
    
    /* Fourth call after volatile call */
    int r5 = helper4(b, c);
    
    /* Final computation using all values to prevent dead code elimination */
    int result = (a + b) ^ (c - d) | (e * f) & (r1 + r2 - r3) ^ r5;
    
    return result;
}

int main(int argc, char **argv) {
    /* Initialize volatile function pointer (could be NULL) */
    volatile_func = helper2;  /* Use helper2 as the volatile function */
    
    int total = 0;
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        int seed = i * 12345 + argc;
        int result = test_caller_save(seed);
        total += result;
        
        /* Also call with different patterns */
        result = test_caller_save(seed ^ 0x5555);
        total -= result;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
