/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper functions */
__attribute__((noinline)) int helper1(int a, int b) {
    return a * b + 7;
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

/* Volatile function pointer to prevent optimization */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func;

/* Test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Multiple local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 11;
    int c = seed ^ 0x55;
    int d = seed - 7;
    int e = seed * seed;
    int f = seed | 0xFF;
    int g = seed << 2;
    int h = seed >> 1;
    
    /* First computation using multiple registers */
    a = b * c + d;
    b = a ^ f;
    c = d + e * 3;
    
    /* First call - clobbers call-clobbered registers */
    int r1 = helper1(a, b);
    
    /* Intervening computation keeping values live */
    d = r1 * c + g;
    e = d ^ h;
    f = a + b - c;
    
    /* Second call with more arguments */
    int r2 = helper2(d, e, f);
    
    /* More computations creating dependencies */
    g = r2 * a + b;
    h = g ^ c ^ d;
    a = h + e * f;
    
    /* Third call - different number of arguments */
    int r3 = helper3(g, h, a, r2);
    
    /* Complex computation chain */
    b = r3 * 2 + g;
    c = b ^ (h << 1);
    d = c + a * 3;
    
    /* Volatile function pointer call - unpredictable for compiler */
    if (volatile_func) {
        int r4 = volatile_func(d, e);
        f = r4 + g * h;
    }
    
    /* Final call with many arguments */
    int r5 = helper4(a, b, c, d, f);
    
    /* Use all computed values in final expression */
    return (r1 ^ r2) + (r3 * r5) - (a + b + c + d + e + f + g + h);
}

int main(int argc, char **argv) {
    /* Initialize volatile function pointer */
    volatile_func = helper1;
    
    /* Call test function multiple times with different inputs
       to prevent constant propagation */
    int total = 0;
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
