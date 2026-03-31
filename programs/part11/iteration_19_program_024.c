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
volatile func_ptr_t volatile_func = helper1;

/* Main test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 2;
    int b = seed + 5;
    int c = seed ^ 0x1234;
    int d = seed - 7;
    int e = seed * 3 + 1;
    int f = seed | 0xABCD;
    
    /* Complex arithmetic creating dependencies, forcing values to stay in registers */
    a = b * c + d;          /* Uses b, c, d -> result in a */
    e = a ^ f;              /* Uses a, f -> result in e */
    d = (b << 3) | (c >> 2); /* Uses b, c -> result in d */
    f = e * d - a;          /* Uses e, d, a -> result in f */
    
    /* First call - uses multiple registers */
    int r1 = helper1(a, b);
    /* Use result immediately to create live value across next call */
    c = r1 * 2 + d;
    
    /* Second call with different arguments */
    int r2 = helper2(b, c, d);
    /* Complex computation between calls */
    e = (r2 ^ a) | (c & d);
    
    /* Third call with many arguments */
    int r3 = helper3(a, b, c, d);
    /* More register-intensive computation */
    f = (r3 * e) + (a << 2) - (b >> 1);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r4 = volatile_func(e, f);
    /* Use volatile call result in computation */
    a = r4 * 3 + b;
    
    /* Fourth call with many arguments, placed strategically */
    int r5 = helper4(a, b, c, d, e);
    
    /* Final computation using all values to prevent dead code elimination */
    int result = (a ^ b) + (c & d) - (e | f) * r1 / (r2 + r3) ^ r4 + r5;
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call test function with varying arguments to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
