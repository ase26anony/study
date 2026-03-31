/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline helper functions to force actual call instructions */
__attribute__((noinline)) int helper1(int a, int b) {
    return a * b + 7;
}

__attribute__((noinline)) int helper2(int a, int b, int c) {
    return (a ^ b) | c;
}

__attribute__((noinline)) int helper3(int a, int b, int c, int d) {
    return (a * b) + (c - d);
}

__attribute__((noinline)) int helper4(int a, int b) {
    return (a << 3) | (b & 0xFF);
}

/* Volatile function pointer to create unpredictable call site */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func = helper1;

/* Main test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 17;
    int c = seed ^ 0x55AA;
    int d = seed - 42;
    int e = seed * seed;
    int f = seed | 0x1234;
    
    /* First computation keeping values live across calls */
    a = b * c + d;
    b = a ^ f;
    
    /* First call - clobbers call-clobbered registers */
    int r1 = helper1(a, b);
    
    /* Interleaved computations using multiple registers */
    c = d * e + r1;
    d = (a ^ c) | b;
    e = r1 * 3 - d;
    
    /* Second call with more arguments */
    int r2 = helper2(c, d, e);
    
    /* More register-intensive computations */
    f = (a + b) * (c - d);
    a = (r1 ^ r2) + f;
    b = e * 2 + r1;
    
    /* Third call - different number of arguments */
    int r3 = helper3(a, b, c, d);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r4 = volatile_func(r2, r3);
    
    /* More computations after volatile call */
    c = (r3 << 2) | (r4 & 0xF);
    d = r1 + r2 + r3 + r4;
    
    /* Fourth call */
    int r5 = helper4(c, d);
    
    /* Final computation using all values to prevent elimination */
    int result = (a * b) + (c ^ d) - (e * f) + (r1 - r2) * (r3 + r4) / (r5 | 1);
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call test function multiple times with different inputs
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
