/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper functions to force call instructions */
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
static int (*volatile volatile_func)(int, int, int, int, int) = helper4;

/* Test function with complex register usage across multiple calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 17;
    int c = seed ^ 0x55AA;
    int d = seed - 42;
    int e = seed * seed;
    int f = seed | 0xFF00;
    
    /* First computation using multiple registers */
    a = b * c + d;
    e = a ^ f;
    
    /* First call - clobbers call-clobbered registers */
    int r1 = helper1(a, b);
    
    /* Inter-call computation keeping values live in registers */
    b = c * d + e;
    d = a ^ b | c;
    
    /* Second call with more arguments */
    int r2 = helper2(a, b, c);
    
    /* More register-intensive computations */
    c = d * e + r1;
    f = (a & b) | (c ^ d);
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* Complex computation using all available values */
    e = (r1 * r2) + (r3 ^ f);
    a = (b << 2) | (c >> 3);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r4 = volatile_func(a, b, c, d, e);
    
    /* Final computation using all results - ensures values stay live */
    int result = (r1 + r2) * (r3 - r4) + (a * b) / (c | 1) + (d ^ e) * f;
    
    /* Use result in a way that prevents dead code elimination */
    return result + (seed & 1);
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
