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

__attribute__((noinline)) int helper4(int a, int b) {
    return (a << 3) | (b & 0xFF);
}

/* Volatile function pointer to prevent optimization */
static int (*volatile volatile_func)(int, int, int, int);

/* Test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Multiple local variables to create register pressure */
    int a = seed * 3 + 1;
    int b = seed * 5 - 2;
    int c = seed * 7 + 3;
    int d = seed * 11 - 5;
    int e = seed * 13 + 7;
    int f = seed * 17 - 11;
    
    /* Complex arithmetic creating register dependencies */
    a = b * c + d;
    b = a ^ f + e;
    c = (d << 2) | (e & 0xF);
    d = a * b - c;
    
    /* First call - uses multiple registers */
    int r1 = helper1(a, b);
    e = r1 + c * 2;
    
    /* More arithmetic between calls */
    f = (d ^ e) + (a << 1);
    a = b * 3 + c / 2;
    
    /* Second call with different arguments */
    int r2 = helper2(c, d, e);
    b = r2 - f;
    
    /* More computations */
    c = a * b + d;
    d = (e ^ f) | (a & b);
    
    /* Third call */
    int r3 = helper3(a, b, c, d);
    e = r3 + f;
    
    /* Volatile function pointer call - unpredictable for compiler */
    volatile_func = helper3;  /* Use helper3 as target */
    int r4 = volatile_func(e, f, a, b);
    f = r4 - d;
    
    /* More arithmetic */
    a = b + c * 3;
    b = (d ^ e) + (f << 2);
    
    /* Fourth call */
    int r5 = helper4(a, b);
    c = r5 + e;
    
    /* Final complex computation using all variables */
    int result = (a * b) + (c ^ d) - (e | f) + (r1 & r2) * (r3 ^ r4) / (r5 + 1);
    
    return result;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
