/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper functions to force actual call instructions */
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
    return a * 3 - b * 2;
}

/* Volatile function pointer to create unpredictable call site */
static int (*volatile volatile_func)(int, int, int) = NULL;

/* Main test function with register-intensive computations between calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Multiple local variables to create register pressure */
    int a = seed * 3 + 1;
    int b = seed * 5 - 2;
    int c = seed * 7 + 3;
    int d = seed * 11 - 4;
    int e = seed * 13 + 5;
    int f = seed * 17 - 6;
    
    /* Complex arithmetic creating dependencies, forcing values to stay in registers */
    a = b * c + d;
    b = a ^ f + e;
    c = d * 3 - a / 2;
    d = (e << 2) | (f & 0xFF);
    e = a + b + c + d;
    f = (a * b) ^ (c * d);
    
    /* First call - uses multiple registers */
    int r1 = helper1(a, b);
    
    /* More register-intensive computations between calls */
    a = r1 + c * 2;
    b = d ^ e;
    c = f - a + b;
    d = (r1 << 3) | (a & 0xF);
    
    /* Second call with different arguments */
    int r2 = helper2(b, c, d);
    
    /* More computations creating live values across calls */
    e = a * r2 + 1;
    f = b ^ r2 | c;
    a = (d + e) * f;
    b = r1 - r2 + a;
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(e, f, r3);
        c = r4 * 2 + a;
    }
    
    /* More computations using all previous results */
    d = r1 * r2 + r3;
    e = (a ^ b) | (c & d);
    f = helper4(d, e);  /* Fourth call */
    
    /* Final complex expression using all variables to prevent dead code elimination */
    return (a + b) * (c - d) + (e ^ f) * r1 - r2 / (r3 + 1);
}

int main(int argc, char **argv) {
    /* Initialize volatile function pointer */
    volatile_func = helper2;  /* Use helper2 as the volatile function */
    
    int total = 0;
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
    }
    
    /* Also test with NULL volatile function pointer */
    volatile_func = NULL;
    total += test_caller_save(999);
    
    printf("Result: %d\n", total);
    return 0;
}
