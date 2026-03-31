/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper functions */
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
    return a + (b << 2) - (c >> 1) + d * e;
}

/* Volatile function pointer to prevent optimization */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func = NULL;

/* Test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Multiple local variables to create register pressure */
    int a = seed * 3 + 1;
    int b = seed * 5 - 2;
    int c = seed * 7 + 3;
    int d = seed * 11 - 4;
    int e = seed * 13 + 5;
    int f = seed * 17 - 6;
    int g = seed * 19 + 7;
    int h = seed * 23 - 8;
    
    /* Complex arithmetic creating dependencies */
    a = b * c + d - e;
    b = (a ^ f) | (g & h);
    c = d * e - f * g;
    d = (a << 2) | (b >> 3);
    
    /* First call - uses multiple registers */
    int r1 = helper1(a, b);
    
    /* More arithmetic between calls */
    e = r1 * c + d;
    f = (e ^ a) & (b | c);
    
    /* Second call with more arguments */
    int r2 = helper2(c, d, e);
    
    /* Complex computation using results */
    g = (r1 + r2) * (a - b);
    h = (g << 3) ^ (f >> 2);
    
    /* Third call with even more arguments */
    int r3 = helper3(e, f, g, h);
    
    /* More arithmetic */
    a = b + c * d - e / (f + 1);
    b = (r2 ^ r3) & (g | h);
    
    /* Volatile function pointer call - unpredictable */
    if (volatile_func) {
        int r4 = volatile_func(a, b);
        c = r4 * 3 - 7;
    }
    
    /* Fourth call with many arguments */
    int r5 = helper4(r1, r2, r3, a, b);
    
    /* Final complex computation using all values */
    int result = (r1 * 2) + (r2 * 3) - (r3 * 4) + 
                 (a * 5) - (b * 6) + (c * 7) +
                 (d * 8) | (e * 9) ^ (f * 10) &
                 (g * 11) + (h * 12) - (r5 * 13);
    
    return result;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Initialize volatile function pointer */
    volatile_func = helper1;
    
    /* Call test function multiple times with different inputs
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
        
        /* Change function pointer occasionally */
        if (i % 3 == 0) {
            volatile_func = helper2;
        } else if (i % 5 == 0) {
            volatile_func = NULL;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
