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
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func = NULL;

/* Main test function with register-intensive computations */
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
    
    /* More arithmetic between calls to keep values live */
    c = r1 * d + e;
    f = (a & b) | (c ^ d);
    
    /* Second call with more arguments */
    int r2 = helper2(c, d, e);
    
    /* Complex computation forcing register reuse */
    a = (r1 << 3) ^ (r2 >> 1);
    b = (c * d) + (e * f);
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* More arithmetic creating cross-dependencies */
    e = (r2 & r3) | (a ^ b);
    f = (c * r1) - (d * r2);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(e, f);
        a = r4 + r3;
    }
    
    /* Fourth call with many arguments */
    int r5 = helper4(a, b, c, d, e);
    
    /* Final complex computation using all values */
    int result = (r1 + r2) ^ (r3 * r4) | (r5 & a) + (b - c) * (d ^ e) | f;
    
    /* Use all variables to prevent dead code elimination */
    result += (a & 0xFF) + (b & 0xFF00) + (c & 0xFF0000) + 
              (d & 0xFF000000) + (e & 0xF) + (f & 0xF0);
    
    return result;
}

int main() {
    /* Initialize volatile function pointer */
    volatile_func = helper1;
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    int total = 0;
    for (int i = 0; i < 100; i++) {
        total ^= test_caller_save(i);
        
        /* Change function pointer occasionally */
        if (i % 3 == 0) {
            volatile_func = helper1;
        } else if (i % 3 == 1) {
            volatile_func = helper2;
        } else {
            volatile_func = NULL;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
