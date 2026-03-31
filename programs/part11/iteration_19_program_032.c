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
    return (a * b) + (c - d);
}

__attribute__((noinline)) int helper4(int a, int b, int c, int d, int e) {
    return (a + b) * (c - d) ^ e;
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
    int e = seed | 0x1234;
    int f = seed & 0xF0F0;
    
    /* First computation using multiple registers */
    a = b * c + d;
    b = a ^ f;
    
    /* First call - clobbers caller-saved registers */
    int r1 = helper1(a, b);
    
    /* Intervening computations keeping values live in registers */
    c = r1 * d + e;
    d = c ^ a;
    e = b + d * 3;
    
    /* Second call with different arguments */
    int r2 = helper2(c, d, e);
    
    /* More register-intensive computations */
    f = r2 * a + b;
    a = f ^ c;
    b = d * e + f;
    
    /* Third call - creates another save/restore point */
    int r3 = helper3(a, b, c, d);
    
    /* Complex computation chain */
    c = r3 + a * b;
    d = c ^ f;
    e = d * 2 + r2;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        f = volatile_func(e, f);
    } else {
        f = helper1(e, f);  /* Fallback call */
    }
    
    /* Final computation using all values */
    int result = (a + b) ^ (c - d) | (e * f) + r1 - r2 + r3;
    
    return result;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        /* Initialize volatile function pointer occasionally */
        if (i % 23 == 0) {
            volatile_func = helper1;
        } else if (i % 29 == 0) {
            volatile_func = helper2;
        } else {
            volatile_func = NULL;
        }
        
        int result = test_caller_save(i);
        total += result;
        
        /* Use result to prevent dead code elimination */
        if (result % 1000 == 0) {
            printf("Intermediate result at i=%d: %d\n", i, result);
        }
    }
    
    printf("Final total: %d\n", total);
    return 0;
}
