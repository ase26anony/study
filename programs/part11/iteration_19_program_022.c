/* caller-save-test.c */
#include <stdio.h>
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
volatile func_ptr_t volatile_func = NULL;

/* Main test function with complex register usage */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x55AA;
    int d = seed - 100;
    int e = seed * seed;
    int f = seed | 0xFF00;
    
    /* Complex arithmetic creating dependencies, forcing values to stay in registers */
    a = b * c + d;
    e = a ^ f;
    d = (b << 3) | (c >> 2);
    
    /* First call - uses some registers */
    int r1 = helper1(a, b);
    
    /* More arithmetic between calls */
    c = r1 * d + e;
    f = (a & b) | (c ^ d);
    
    /* Second call with more arguments */
    int r2 = helper2(a, b, c);
    
    /* Complex computation using all variables */
    e = (r1 * r2) + (a << 2) - (b >> 1);
    d = (c ^ f) | (e & 0xFFFF);
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* More register-intensive operations */
    a = (b * c * d) / (seed + 1);
    f = (e ^ r3) & (r1 | r2);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(r2, r3);
        a = a + r4;
    }
    
    /* Fourth call with maximum arguments */
    int r4 = helper4(a, b, c, d, e);
    
    /* Final complex computation using all values */
    int result = (r1 * r2) + (r3 ^ r4) - (a & b) | (c ^ d) & (e | f);
    
    /* Ensure all values are used to prevent dead code elimination */
    result += (a + b + c + d + e + f + r1 + r2 + r3 + r4);
    
    return result;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Initialize volatile function pointer */
    volatile_func = helper1;
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
    }
    
    /* Also test with NULL function pointer */
    volatile_func = NULL;
    for (int i = 100; i < 200; i++) {
        total += test_caller_save(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
