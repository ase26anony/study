/* caller-save-test.c
 * Designed to trigger GCC's caller-save instruction reordering logic
 * Specifically targets lines 905-913 in caller-save.cc
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
    return a * b - c * d;
}

__attribute__((noinline)) int helper4(int a, int b) {
    return (a << 3) | (b & 0xFF);
}

/* Volatile function pointer to create unpredictable call site */
static int (*volatile volatile_func)(int, int, int) = NULL;

/* Main test function with complex register usage pattern */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3 + 1;
    int b = seed * 5 - 2;
    int c = seed * 7 + 3;
    int d = seed * 11 - 4;
    int e = seed * 13 + 5;
    int f = seed * 17 - 6;
    
    /* First computation using call-clobbered registers */
    a = b * c + d;
    b = a ^ f;
    
    /* First call - clobbers registers */
    int r1 = helper1(a, b);
    
    /* Intervening computation keeping values live in registers */
    c = d * e + r1;
    d = c ^ a;
    
    /* Second call with different arguments */
    int r2 = helper2(b, c, d);
    
    /* More computations creating dependencies */
    e = f * r1 + r2;
    f = e ^ d;
    
    /* Third call with more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* Critical: Create a scenario where save/restore might be inserted
     * after a call that's at the end of a basic block */
    int temp = e * f + r3;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        /* This creates an unpredictable call site that may need
         * special handling in caller-save */
        int r4 = volatile_func(temp, r2, r3);
        temp = temp ^ r4;
    }
    
    /* Fourth call - result used immediately after */
    int r4 = helper4(temp, f);
    
    /* Final computation using all values to prevent dead code elimination */
    int result = (a + b) ^ (c - d) | (e * f) & (r1 + r2) - (r3 ^ r4);
    
    return result;
}

/* Another volatile function to initialize the function pointer */
__attribute__((noinline)) int dummy_func(int x, int y, int z) {
    return x * y + z;
}

int main(void) {
    /* Initialize volatile function pointer */
    volatile_func = dummy_func;
    
    int total = 0;
    
    /* Loop with varying arguments to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        int result = test_caller_save(i);
        total += result;
        
        /* Occasionally change the function pointer to create
         * different call patterns */
        if (i % 23 == 0) {
            volatile_func = (i % 2) ? dummy_func : NULL;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
