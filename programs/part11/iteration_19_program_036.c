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

/* Volatile function pointer to prevent optimization */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func;

/* Test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 17;
    int c = seed ^ 0x55AA;
    int d = seed - 42;
    int e = seed * seed;
    int f = seed | 0xFF00;
    
    /* Complex arithmetic creating register dependencies */
    a = b * c + d;
    b = a ^ f + e;
    c = d * e - a;
    d = (b << 3) | (c >> 2);
    e = a + b + c + d;
    f = (e * f) ^ 0x1234;
    
    /* First call - uses some registers */
    int r1 = helper1(a, b);
    a = r1 + c;  /* Use result, keeping 'a' live across next call */
    
    /* More arithmetic between calls */
    b = b * 2 + d;
    c = c ^ e;
    d = d + f - a;
    
    /* Second call with more arguments */
    int r2 = helper2(a, b, c);
    e = r2 * d;  /* Use result */
    
    /* Complex computation creating more live values */
    f = (a << 4) | (b >> 2);
    int g = c * d + e;
    int h = f ^ g;
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    g = r3 + h;
    
    /* More register-intensive operations */
    a = a + g * 2;
    b = b ^ h;
    c = c * 3 - g;
    d = d | (h << 1);
    
    /* Fourth call with maximum arguments */
    int r4 = helper4(a, b, c, d, e);
    h = r4 * f;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    volatile_func = helper1;
    int r5 = volatile_func(g, h);
    
    /* Final computation using all values to prevent dead code elimination */
    int result = a + b - c + d - e + f + g + h + r1 + r2 + r3 + r4 + r5;
    
    /* Additional arithmetic to ensure values stay in registers */
    result = result ^ (a * b);
    result = result | (c & d);
    result = result + (e << 2);
    result = result - (f >> 1);
    result = result * (g % 7);
    result = result ^ (h * 3);
    
    return result;
}

int main() {
    volatile_func = helper1;  /* Initialize volatile function pointer */
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    int total = 0;
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
        total ^= test_caller_save(i * 2 + 1);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
