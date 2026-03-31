/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline helper functions to force actual calls */
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
    int c = seed ^ 0x55AA;
    int d = seed - 42;
    int e = seed * seed;
    int f = seed | 0xFF00;
    int g = seed & 0x00FF;
    int h = seed << 3;
    
    /* First computation using multiple registers */
    a = b * c + d;
    b = a ^ f;
    c = d + e * g;
    
    /* First call - clobbers call-clobbered registers */
    int r1 = helper1(a, b);
    
    /* More computations keeping values live across calls */
    d = r1 * c + h;
    e = (a ^ b) | (c & d);
    f = helper2(d, e, f);  /* Second call */
    
    /* Complex computation chain */
    g = (f * a) >> 2;
    h = (b + c) * (d - e);
    int i = g ^ h;
    
    /* Third call with more arguments */
    int r3 = helper3(a, b, g, h);
    
    /* More register-intensive operations */
    a = r3 * i + f;
    b = (a << 4) | (r3 >> 4);
    c = helper4(a, b, c, d, e);  /* Fourth call */
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        d = volatile_func(c, a);  /* Unpredictable call site */
    } else {
        d = c + a;
    }
    
    /* Final computations using all values */
    e = (a * b) + (c * d) - (e * f) + (g * h) - i;
    f = (a ^ b) | (c & d) ^ (e << f) | (g >> h);
    
    /* One more call after complex computations */
    int final = helper1(e, f);
    
    /* Use all computed values in return to prevent dead code elimination */
    return final + a + b + c + d + e + f + g + h + i + r1 + r3;
}

int main() {
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    int total = 0;
    
    /* Initialize volatile function pointer once */
    volatile_func = helper1;
    
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
        
        /* Change function pointer occasionally to maintain volatility */
        if (i % 30 == 0) {
            volatile_func = (i % 60 == 0) ? helper2 : helper1;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
