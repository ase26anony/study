/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline helper functions to force actual call instructions */
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
    return (a + b) * (c - d) + e;
}

/* Volatile function pointer to create unpredictable call site */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func = NULL;

/* Main test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x1234;
    int d = seed - 100;
    int e = seed << 3;
    int f = seed | 0xABCD;
    
    /* Complex arithmetic creating dependencies between variables */
    a = b * c + d;
    e = a ^ f;
    d = (b << 2) | (c >> 1);
    
    /* First call - uses some registers, clobbers others */
    int r1 = helper1(a, b);
    c = r1 + d;  /* Use result immediately, forcing save/restore */
    
    /* More arithmetic between calls */
    f = (e * 3) + (d / 2);
    b = a ^ c ^ f;
    
    /* Second call with more arguments */
    int r2 = helper2(a, b, c);
    e = r2 * d;  /* Another dependency chain */
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    f = r3 + e + a;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(f, e);
        a = r4 * b;  /* Use result, forcing save after volatile call */
    }
    
    /* More arithmetic creating cross-register dependencies */
    c = (a << 1) + (b >> 2);
    d = e * f - c;
    
    /* Fourth call with maximum arguments */
    int r5 = helper4(a, b, c, d, e);
    f = r5 ^ a ^ b;
    
    /* Final complex computation using all variables */
    int result = (a + b) * (c - d) + (e ^ f);
    result = result * 2 - (a & b & c & d & e & f);
    
    return result;
}

int main(void) {
    /* Initialize volatile function pointer */
    volatile_func = helper1;
    
    int total = 0;
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        int result = test_caller_save(i);
        total += result;
        
        /* Change function pointer occasionally */
        if (i % 3 == 0) {
            volatile_func = helper2;
        } else if (i % 3 == 1) {
            volatile_func = helper1;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
