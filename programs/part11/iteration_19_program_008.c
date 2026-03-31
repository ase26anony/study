/* caller-save-test.c
 * Test program to trigger specific instruction reordering logic
 * in GCC's caller-save optimization (lines 905-913 of caller-save.cc)
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inline helper functions to force actual call instructions */
__attribute__((noinline)) int helper1(int a, int b) {
    return a + b * 3;
}

__attribute__((noinline)) int helper2(int a, int b, int c) {
    return (a ^ b) | c;
}

__attribute__((noinline)) int helper3(int a, int b, int c, int d) {
    return (a * b) + (c - d);
}

__attribute__((noinline)) int helper4(int a, int b, int c, int d, int e) {
    return (a << 2) | (b & c) ^ (d * e);
}

/* Volatile function pointer to create unpredictable call site */
typedef int (*func_ptr_t)(int, int, int);
volatile func_ptr_t volatile_func = NULL;

/* Main test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 2;
    int b = seed + 0x1234;
    int c = seed ^ 0xABCD;
    int d = seed - 0x5678;
    int e = seed | 0x9ABC;
    int f = seed & 0xDEF0;
    
    /* First computation creating dependencies */
    a = b * c + d;
    b = a ^ f;
    c = d + e * 2;
    
    /* First call - uses some registers */
    int r1 = helper1(a, b);
    
    /* Inter-call computation keeping values live */
    d = r1 * c + 0x1111;
    e = (d ^ a) | b;
    f = c + d - e;
    
    /* Second call with more arguments */
    int r2 = helper2(a, b, c);
    
    /* More computations between calls */
    a = r2 + d * 3;
    b = (e ^ f) + r1;
    c = a * b - d;
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* Complex computation chain */
    d = (r3 << 3) | (a & b);
    e = c ^ d + f;
    f = r2 * e - a;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(d, e, f);
        a = r4 + b * c;
    }
    
    /* Fourth call with many arguments */
    int r5 = helper4(a, b, c, d, e);
    
    /* Final computation using all values to prevent elimination */
    int result = (a + b) ^ (c - d) | (e * f) & (r1 + r2) - (r3 ^ r5);
    
    /* Additional computation to create more register pressure */
    result = result * 2 + (seed & 0xFF);
    
    return result;
}

/* Another helper to increase call density */
__attribute__((noinline)) int helper5(int x) {
    return x * 0x55 + 0xAA;
}

/* Secondary test function with different pattern */
__attribute__((noinline)) int test_caller_save2(int seed) {
    int v1 = seed;
    int v2 = seed * 3;
    int v3 = seed + 0x1000;
    int v4 = seed ^ 0x2000;
    int v5 = seed | 0x4000;
    
    /* Chain of calls with computations in between */
    v1 = helper1(v1, v2);
    v3 = v1 * v2 + v4;
    v2 = helper2(v3, v4, v5);
    v4 = v2 ^ v3 | v5;
    v5 = helper3(v1, v2, v3, v4);
    v1 = helper4(v2, v3, v4, v5, v1);
    
    /* Force BB_END scenario: call whose result is used later */
    int temp = helper5(v1);
    
    /* Use temp after more computations */
    v2 = v3 * v4 + temp;
    v3 = (v2 ^ temp) + v5;
    
    /* Final call that might trigger BB_END update */
    int result = helper1(v2, v3);
    
    /* Use result in computation that keeps everything live */
    return result + v1 + v4 + v5;
}

int main(int argc, char **argv) {
    /* Initialize volatile function pointer */
    volatile_func = (func_ptr_t)helper2;
    
    int total = 0;
    
    /* Loop with varying inputs to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        int seed = i * 0x12345 + argc;
        
        /* Call test functions with different patterns */
        total += test_caller_save(seed);
        total ^= test_caller_save2(seed + 1);
        
        /* Alternate volatile function pointer */
        if (i % 3 == 0) {
            volatile_func = (func_ptr_t)helper1;
        } else if (i % 3 == 1) {
            volatile_func = (func_ptr_t)helper3;
        } else {
            volatile_func = NULL;
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
