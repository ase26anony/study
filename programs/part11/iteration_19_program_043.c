/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper functions */
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

/* Test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare many local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 17;
    int c = seed ^ 0x55AA;
    int d = seed - 42;
    int e = seed * seed;
    int f = seed | 0xFF00;
    int g = seed & 0x00FF;
    int h = seed + 1000;
    
    /* Complex arithmetic creating dependencies between variables */
    a = b * c + d;          /* Uses b, c, d -> needs multiple registers */
    e = a ^ f ^ g;          /* Uses a, f, g */
    h = (h << 3) | (h >> 5); /* Rotate h */
    
    /* First call - uses some registers, clobbers others */
    int r1 = helper1(a, b);
    
    /* More arithmetic between calls - keeps values live in registers */
    c = r1 + d * 2;
    f = e ^ (g << 1);
    
    /* Second call with more arguments */
    int r2 = helper2(c, d, e);
    
    /* Complex computation using results */
    a = r2 * 3 + f;
    b = (h & 0xF0F0) | (r1 & 0x0F0F);
    g = a ^ b ^ c;
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* More arithmetic */
    e = r3 + g * 7;
    h = (h * 13) ^ (f << 2);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r4 = volatile_func(e, f, g, h, r2);
    
    /* Final complex computation using all values */
    int result = (a + b) * (c - d) + (e ^ f) * (g | h) + r1 * r2 + r3 * r4;
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call test function multiple times with different inputs
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
        total += test_caller_save(i * 2);
        total += test_caller_save(i + 100);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
