/* caller-save-test.c
 * Designed to trigger GCC's caller-save instruction reordering logic
 * Specifically targets lines 905-913 in caller-save.cc
 */

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

/* Main test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x55AA;
    int d = seed - 100;
    int e = seed << 3;
    int f = seed >> 2;
    
    /* Complex arithmetic creating dependencies, forcing values to stay in registers */
    a = b * c + d;
    e = a ^ f;
    d = (b << 2) | (c >> 1);
    
    /* First call - uses some registers */
    int r1 = helper1(a, b);
    
    /* More arithmetic between calls */
    c = r1 * d + e;
    f = (a & b) | (c ^ d);
    
    /* Second call with more arguments */
    int r2 = helper2(c, d, e);
    
    /* Complex computation creating more register dependencies */
    a = (r1 << 3) + (r2 >> 1);
    b = (c ^ d) * (e & f);
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* More arithmetic */
    e = (r2 * r3) + (a ^ b);
    f = (c | d) & (r1 ^ r2);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(e, f, r3);
        a = r4 + a;
    }
    
    /* Fourth call with many arguments */
    int r5 = helper4(a, b, c, d, e);
    
    /* Final complex computation using all values */
    int result = (r1 ^ r2) + (r3 & r5) * (a | b) - (c ^ d) + (e & f);
    
    /* Ensure all computed values are used to prevent dead code elimination */
    return result + (r1 + r2 + r3 + r5);
}

/* Another helper to prevent constant propagation */
__attribute__((noinline)) int unpredictable(int x) {
    static int counter = 0;
    return x ^ (counter++);
}

int main(void) {
    int total = 0;
    
    /* Initialize volatile function pointer */
    volatile_func = helper2;
    
    /* Call test function multiple times with different inputs
     * to prevent constant propagation and create varied register usage */
    for (int i = 0; i < 100; i++) {
        int seed = unpredictable(i);
        int result = test_caller_save(seed);
        total += result;
        
        /* Change volatile function pointer occasionally */
        if (i % 3 == 0) {
            volatile_func = helper2;
        } else if (i % 3 == 1) {
            volatile_func = helper1;
        } else {
            volatile_func = NULL;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
