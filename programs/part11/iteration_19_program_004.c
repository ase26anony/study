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
    return a + b * c - d / (e + 1);
}

/* Volatile function pointer to create unpredictable call site */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func = NULL;

/* Main test function with register-intensive computations */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55;
    int d = seed - 19;
    int e = seed * seed;
    int f = seed | 0xFF;
    
    /* First computation using multiple registers */
    a = b * c + d;
    e = a ^ f;
    
    /* First call - clobbers caller-saved registers */
    int r1 = helper1(a, b);
    
    /* Intervening computation keeping values live in registers */
    b = r1 * c + d;
    c = a ^ b | e;
    d = helper2(a, b, c);  /* Second call */
    
    /* More register-intensive computations */
    e = d * a + b;
    f = c ^ e;
    a = b + c * d;
    
    /* Third call with more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        /* This creates a call site with unknown clobbering behavior */
        int volatile_result = volatile_func(r3, e);
        f += volatile_result;
    }
    
    /* More computations between calls */
    b = e * f + r3;
    c = a ^ d | b;
    
    /* Fourth call with many arguments */
    int r4 = helper4(a, b, c, d, e);
    
    /* Final complex computation using all values */
    int result = (a * b) + (c ^ d) - (e & f) + (r1 * r3) / (r4 + 1);
    
    /* Use all variables to prevent dead code elimination */
    result += (a < b) ? c : d;
    result ^= (e > f) ? r1 : r3;
    
    return result;
}

int main() {
    volatile_func = helper1;  /* Set to a valid function */
    
    int total = 0;
    
    /* Call test function multiple times with different inputs
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
        total ^= test_caller_save(i * 2);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
