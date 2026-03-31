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

__attribute__((noinline)) int helper4(int a, int b, int c, int d, int e) {
    return (a * b) + (c * d) - e;
}

/* Volatile function pointer to create unpredictable call site */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func = NULL;

/* Main test function with register-intensive computations */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 17;
    int c = seed ^ 0xABCD;
    int d = seed - 42;
    int e = seed | 0x1234;
    int f = seed & 0xF0F0;
    
    /* First computation creating dependencies */
    a = b * c + d;
    b = a ^ f;
    
    /* First call - clobbers call-clobbered registers */
    int r1 = helper1(a, b);
    
    /* More computations keeping values live across calls */
    c = r1 * d + e;
    d = c ^ a;
    e = b + d * 3;
    
    /* Second call with different arguments */
    int r2 = helper2(c, d, e);
    
    /* Complex computation chain */
    f = r1 + r2;
    a = f * b - c;
    b = a | d;
    c = b ^ e;
    
    /* Third call with more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* More register-intensive operations */
    d = r3 * 2;
    e = f + d;
    a = e ^ r2;
    b = a * c;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(b, d);
        e = r4 + a;
    }
    
    /* Fourth call with many arguments */
    int r5 = helper4(a, b, c, d, e);
    
    /* Final computation using all values to prevent elimination */
    int result = (r1 ^ r2) + (r3 * r5) - (a & b) | (c ^ d) + (e * f);
    
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
