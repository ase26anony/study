/* caller-save-test.c */
#include <stdio.h>
#include <stdint.h>
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
    return (a * b) + (c * d) - e;
}

/* Volatile function pointer to create unpredictable call site */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func_ptr;

/* Test function with complex register usage across multiple calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55;
    int d = seed - 19;
    int e = seed * seed;
    int f = seed | 0xFF;
    int g = seed << 2;
    int h = seed >> 1;
    
    /* Complex arithmetic to keep values live in registers */
    a = b * c + d;
    e = a ^ f;
    g = (h << 3) | (e & 0xF);
    
    /* First call - uses some registers */
    int r1 = helper1(a, b);
    
    /* More arithmetic between calls */
    c = d * e - f;
    h = g ^ a;
    b = r1 + c;
    
    /* Second call with different arguments */
    int r2 = helper2(b, c, d);
    
    /* More computations creating dependencies */
    f = e * 3 + h;
    a = r2 ^ f;
    d = c + g;
    
    /* Third call with more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* Complex chain of computations */
    e = f * g;
    h = a | b;
    c = d ^ r3;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func_ptr) {
        int r4 = volatile_func_ptr(e, h);
        c = c + r4;
    }
    
    /* Fourth call with many arguments */
    int r5 = helper4(a, b, c, d, e);
    
    /* Final complex computation using all values */
    int result = (a * b) + (c * d) - (e * f) + (g * h) + r1 - r2 + r3 * r5;
    
    /* Use all variables to prevent dead code elimination */
    result += (a & 1) | (b & 2) | (c & 4) | (d & 8) | 
              (e & 16) | (f & 32) | (g & 64) | (h & 128);
    
    return result;
}

int main() {
    volatile_func_ptr = helper1;  /* Set to a valid function */
    
    int total = 0;
    
    /* Call test function multiple times with different inputs
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
        total ^= test_caller_save(i * 2 + 1);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
