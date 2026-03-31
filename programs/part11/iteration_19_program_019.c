/* caller-save-test.c */
#include <stdio.h>
#include <stdint.h>

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
    return (a * b) + (c * d) - e;
}

/* Volatile function pointer to create unpredictable call site */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func;

/* Test function with complex register usage across multiple calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x55AA55AA;
    int d = seed - 100;
    int e = seed * seed;
    int f = seed | 0x12345678;
    
    /* First computation using multiple registers */
    a = b * c + d;
    e = a ^ f;
    
    /* First call - clobbers caller-saved registers */
    int r1 = helper1(a, b);
    
    /* Intervening computation keeping values live in registers */
    c = r1 * d + e;
    f = a ^ c;
    
    /* Second call with different arguments */
    int r2 = helper2(c, d, e);
    
    /* More register-intensive computations */
    b = r2 * f + a;
    d = c ^ b;
    
    /* Third call with more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* Complex computation chain */
    e = r3 * a + b * c;
    f = d ^ e;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(e, f);
        a = r4 * b;
    }
    
    /* Fourth call with many arguments */
    int r5 = helper4(a, b, c, d, e);
    
    /* Final computation using all values to prevent dead code elimination */
    int result = (a + b) ^ (c - d) | (e * f) & r1 ^ r2 | r3 + r5;
    
    return result;
}

/* Another volatile function for the function pointer */
__attribute__((noinline)) int volatile_helper(int x, int y) {
    return x * y + 42;
}

int main(void) {
    volatile_func = volatile_helper;
    
    int total = 0;
    
    /* Call test function multiple times with different inputs
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
        total ^= test_caller_save(i * 2);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
