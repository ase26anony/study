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
    int f = seed | 0xFF00;
    
    /* Complex arithmetic creating dependencies between variables */
    a = b * c + d;
    e = a ^ f;
    d = (b << 2) | (c >> 1);
    
    /* First call - uses some registers */
    int r1 = helper1(a, b);
    
    /* More arithmetic between calls to keep values live */
    c = r1 * d + e;
    f = (a & b) | (c ^ d);
    
    /* Second call with more arguments */
    int r2 = helper2(a, b, c);
    
    /* Complex computation using results */
    e = r1 * r2 + f;
    a = (e << 3) ^ (d >> 2);
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* More register-intensive operations */
    b = r2 * r3 - e;
    f = (a & c) | (b ^ d);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(e, f, r1);
        c = r4 * a + b;
    }
    
    /* Fourth call with maximum arguments to use more registers */
    int r5 = helper4(a, b, c, d, e);
    
    /* Final complex computation using all values */
    int result = (r1 + r2) * (r3 ^ r5) - (a & b) | (c ^ d) + (e * f);
    
    /* Ensure all variables are used to prevent elimination */
    result += (a + b + c + d + e + f + r1 + r2 + r3 + r5);
    
    return result;
}

/* Another volatile function for the function pointer */
__attribute__((noinline)) int volatile_helper(int x, int y, int z) {
    return (x * y) + z;
}

int main() {
    volatile_func = volatile_helper;
    
    int total = 0;
    
    /* Loop with varying seeds to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        int seed = i * 37 + 12345;
        int result = test_caller_save(seed);
        total += result;
        
        /* Occasionally change the function pointer */
        if (i % 7 == 0) {
            volatile_func = (i % 14 == 0) ? NULL : volatile_helper;
        }
    }
    
    printf("Total result: %d\n", total);
    return 0;
}
