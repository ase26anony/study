/* caller-save-test.c */
#include <stdio.h>
#include <stdint.h>

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
    return (a * b) + (c * d) - e;
}

/* Volatile function pointer to create unpredictable call site */
typedef int (*func_ptr_t)(int, int, int);
volatile func_ptr_t volatile_func = NULL;

/* Main test function with register-intensive computations */
__attribute__((noinline)) int test_function(int seed) {
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
    d = (b << 2) | (c >> 3);
    
    /* First call - uses some registers, clobbers others */
    int r1 = helper1(a, b);
    c = r1 * d + e;
    
    /* More arithmetic between calls */
    f = (a & b) | (c ^ d);
    b = e * 2 - f;
    
    /* Second call with more arguments */
    int r2 = helper2(a, b, c);
    e = r2 ^ d;
    a = b + c * 2;
    
    /* Third call - uses different registers */
    int r3 = helper3(a, b, c, d);
    f = r3 & 0xFF;
    d = e * 3 + f;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(a, b, c);
        e = r4 ^ f;
    }
    
    /* More arithmetic */
    b = (d << 1) | (e >> 2);
    c = a * 3 - b;
    
    /* Fourth call with many arguments */
    int r5 = helper4(a, b, c, d, e);
    f = r5 & 0xFFFF;
    
    /* Final computation using all variables to prevent dead code elimination */
    int result = (a + b) ^ (c - d) | (e * f);
    result = result ^ (seed & 0xFF);
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_function(i);
        
        /* Occasionally change the volatile function pointer */
        if (i % 37 == 0) {
            volatile_func = (func_ptr_t)helper2;
        } else if (i % 23 == 0) {
            volatile_func = (func_ptr_t)helper1;
        } else {
            volatile_func = NULL;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
