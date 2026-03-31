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

__attribute__((noinline)) int helper4(int a, int b, int c, int d, int e) {
    return (a * b) + (c * d) - e;
}

/* Volatile function pointer to create unpredictable call site */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func = NULL;

/* Main test function with register-intensive computations between calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 17;
    int c = seed ^ 0xABCD;
    int d = seed - 42;
    int e = seed * seed;
    int f = seed | 0x1234;
    
    /* First computation using multiple registers */
    a = b * c + d;
    e = a ^ f;
    
    /* First call - clobbers call-clobbered registers */
    int r1 = helper1(a, b);
    
    /* Intervening computation keeping values live in registers */
    c = r1 * d + e;
    f = a ^ b ^ c;
    
    /* Second call with different arguments */
    int r2 = helper2(c, d, e);
    
    /* More register-intensive computations */
    a = r2 * 3 + f;
    b = (c << 2) | (d >> 1);
    
    /* Third call with more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* Complex computation chain */
    e = (r3 * a) / (b + 1);
    f = (c ^ d) & (a | b);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(e, f);
        a = r4 * 2;
    }
    
    /* Fourth call with many arguments */
    int r5 = helper4(a, b, c, d, e);
    
    /* Final computation using all values to prevent dead code elimination */
    int result = (r1 + r2) * (r3 - r5) + (a * b) / (c + 1) ^ (d & e) | f;
    
    return result;
}

int main(int argc, char **argv) {
    /* Initialize volatile function pointer */
    volatile_func = helper1;
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    int total = 0;
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
    }
    
    printf("Result: %d\n", total);
    
    /* Also test with NULL function pointer */
    volatile_func = NULL;
    total += test_caller_save(999);
    
    printf("Final result: %d\n", total);
    
    return 0;
}
