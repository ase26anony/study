/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper functions */
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
    return (a + b) * (c - d) + e;
}

/* Volatile function pointer to prevent optimization */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func;

/* Test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Multiple local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55;
    int d = seed - 11;
    int e = seed * 2 + 1;
    int f = seed | 0xFF;
    
    /* Complex arithmetic creating register dependencies */
    a = b * c + d;
    b = a ^ f;
    c = d * e - a;
    d = (b << 3) | (c >> 2);
    e = a + b + c + d;
    f = (e * 3) ^ (a & b);
    
    /* First call - uses some registers */
    int r1 = helper1(a, b);
    
    /* More arithmetic between calls */
    a = r1 * c + d;
    b = helper2(a, c, d) ^ e;
    c = (d << 2) | (e >> 1);
    
    /* Second call with different arguments */
    int r2 = helper2(b, c, d);
    
    /* More computations */
    d = r1 + r2 * 3;
    e = helper3(a, b, c, d) - f;
    f = (e * a) | (b * c);
    
    /* Third call */
    int r3 = helper3(c, d, e, f);
    
    /* Volatile function pointer call - unpredictable for compiler */
    volatile_func = helper1;
    int r4 = volatile_func(r2, r3);
    
    /* More arithmetic after volatile call */
    a = r3 * r4 + a;
    b = helper4(a, b, c, d, e) ^ f;
    c = (r1 << 4) | (r2 >> 2);
    
    /* Fourth call */
    int r5 = helper4(a, b, c, d, e);
    
    /* Final computation using all values */
    int result = (r1 * 2) + (r2 * 3) - (r3 * 4) + (r4 * 5) - (r5 * 6) +
                 (a / 7) + (b / 8) - (c / 9) + (d / 10) - (e / 11) + (f / 12);
    
    return result;
}

int main() {
    volatile_func = helper1;
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    int total = 0;
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
    }
    
    printf("Result: %d\n", total);
    
    /* Additional test with different patterns */
    for (int i = 0; i < 50; i++) {
        total -= test_caller_save(i * 2 + 1);
    }
    
    printf("Final result: %d\n", total);
    return 0;
}
