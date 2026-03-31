/* caller-save-test.c
 * Designed to trigger GCC's caller-save instruction reordering logic
 * Specifically targets lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Non-inline helper functions to force actual call instructions */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a + b;
    return result ^ 0x1234;
}

__attribute__((noinline)) int helper2(int a, int b, int c) {
    volatile int result = a * b - c;
    return result | 0x5678;
}

__attribute__((noinline)) int helper3(int a, int b, int c, int d) {
    volatile int result = (a ^ b) + (c & d);
    return result * 3;
}

__attribute__((noinline)) int helper4(int a, int b) {
    volatile int result = a << (b & 3);
    return result - 1;
}

/* Volatile function pointer to create unpredictable call site */
static int (*volatile volatile_func)(int, int, int);

/* Test function with complex register usage across multiple calls */
__attribute__((noinline, optimize("no-crossjumping", "no-sibling-calls")))
int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0xABCD;
    int d = seed - 100;
    int e = seed << 2;
    int f = seed | 0x1234;
    
    /* First computation using multiple registers */
    a = b * c + d;
    e = a ^ f;
    
    /* First call - clobbers call-clobbered registers */
    int r1 = helper1(a, b);
    
    /* Inter-call computation keeping values live in registers */
    c = r1 + d * 2;
    f = e ^ c;
    b = a << 3;
    
    /* Second call with different arguments */
    int r2 = helper2(c, d, e);
    
    /* More register-intensive computations */
    a = r2 * f + 0x55;
    d = (b ^ c) & 0xFF;
    e = a - d;
    
    /* Third call - creates another save/restore point */
    int r3 = helper3(a, b, c, d);
    
    /* Complex computation using all live values */
    f = (r3 + e) * 2;
    b = f ^ a;
    c = d << 2;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    volatile_func = helper2;  /* Different function than expected */
    int r4 = volatile_func(b, c, f);
    
    /* Final computation using all values - ensures they stay live */
    int result = (a + b) ^ (c - d) + (e * f) + (r1 ^ r2) + (r3 & r4);
    
    /* Use result in a way that prevents dead code elimination */
    return result + seed;
}

/* Additional helper to ensure multiple basic blocks */
__attribute__((noinline)) int complex_helper(int x) {
    if (x & 1) {
        return helper1(x, x * 2);
    } else {
        return helper2(x, x / 2, x ^ 0xFF);
    }
}

int main(void) {
    volatile_func = helper3;  /* Initialize volatile function pointer */
    
    int total = 0;
    
    /* Loop with varying arguments to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        /* Mix of different seeds to create varying execution paths */
        int seed = i * 37 + 12345;
        
        /* Call test function */
        int result = test_caller_save(seed);
        
        /* Also call complex helper to create more call sites */
        int extra = complex_helper(seed ^ i);
        
        /* Use results to prevent optimization */
        total += result ^ extra;
        
        /* Occasionally change the volatile function pointer */
        if (i % 7 == 0) {
            volatile_func = (i % 14 == 0) ? (int (*)(int, int, int))helper2 
                                          : (int (*)(int, int, int))helper3;
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
