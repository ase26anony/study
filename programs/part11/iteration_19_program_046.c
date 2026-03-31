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
    return (a * b) + (c ^ d);
}

__attribute__((noinline)) int helper4(int a, int b, int c, int d, int e) {
    return (a + b) * (c - d) ^ e;
}

/* Volatile function pointer to create unpredictable call site */
static int (*volatile volatile_func)(int, int, int, int, int) = helper4;

/* Test function with complex register usage across multiple calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3 + 1;
    int b = seed * 5 - 2;
    int c = seed * 7 + 3;
    int d = seed * 11 - 4;
    int e = seed * 13 + 5;
    int f = seed * 17 - 6;
    
    /* Complex arithmetic creating dependencies between variables */
    a = b * c + d;
    b = a ^ f + e;
    c = d * e - a;
    d = b ^ c * f;
    
    /* First call - uses some registers, clobbers others */
    int r1 = helper1(a, b);
    
    /* More arithmetic between calls to keep values live */
    e = r1 * a + b;
    f = c ^ d + e;
    
    /* Second call with different arguments */
    int r2 = helper2(b, c, d);
    
    /* Complex computation using results */
    a = r1 * r2 + e;
    b = a ^ f - d;
    
    /* Third call with more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* More arithmetic to maintain live values */
    c = r2 * r3 + a;
    d = b ^ c * r1;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r4 = volatile_func(a, b, c, d, e);
    
    /* Final computation using all values - ensures nothing is dead */
    int result = (r1 * r2) ^ (r3 + r4) | (a & b) ^ (c | d) + (e * f);
    
    /* Additional arithmetic to create more register pressure */
    result = result + (a << 2) - (b >> 1) + (c & 0xFF) ^ (d | 0xAA);
    
    return result;
}

int main() {
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
