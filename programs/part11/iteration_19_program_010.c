/* caller-save-test.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Non-inline helper functions to force call instructions */
__attribute__((noinline)) int helper1(int a, int b) {
    return (a * b) ^ 0x1234;
}

__attribute__((noinline)) int helper2(int a, int b, int c) {
    return (a + b) * c;
}

__attribute__((noinline)) int helper3(int a, int b, int c, int d) {
    return (a ^ b) + (c & d);
}

__attribute__((noinline)) int helper4(int a, int b, int c, int d, int e) {
    return (a * b) + (c - d) ^ e;
}

/* Volatile function pointer to create unpredictable call site */
static int (*volatile volatile_func)(int, int, int, int, int) = helper4;

/* Main test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0xABCD;
    int d = seed - 123;
    int e = seed | 0x5555;
    int f = seed & 0xAAAA;
    
    /* Complex arithmetic creating dependencies between variables */
    a = b * c + d;
    e = a ^ f;
    d = (b << 3) | (c >> 2);
    
    /* First call - uses some registers, clobbers others */
    int r1 = helper1(a, b);
    /* Use result immediately to keep it live */
    c = r1 + d;
    
    /* More arithmetic between calls */
    f = (e * 7) - (d / 3);
    b = a ^ c ^ f;
    
    /* Second call with more arguments */
    int r2 = helper2(a, b, c);
    /* Complex dependency chain */
    e = r2 * d + f;
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    a = r3 ^ e;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r4 = volatile_func(a, b, c, d, e);
    
    /* More arithmetic after the volatile call */
    f = (r4 * a) + (b - c);
    d = e ^ f;
    
    /* Fourth call - result used in final computation */
    int r5 = helper4(f, d, a, b, c);
    
    /* Final complex computation using all values */
    int result = (a * b) + (c * d) - (e * f) ^ r1 ^ r2 ^ r3 ^ r4 ^ r5;
    
    /* Use all variables in final result to prevent dead code elimination */
    result += (a & b) | (c & d) | (e & f);
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call test function multiple times with different inputs
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
        total ^= test_caller_save(i * 3 + 1);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
