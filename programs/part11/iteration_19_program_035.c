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
    return a + (b << 2) - (c >> 1) + (d * e);
}

/* Volatile function pointer to force call instruction */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func = helper1;

/* Main test function with complex register usage */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple variables to create register pressure */
    int a = seed * 3;
    int b = seed + 17;
    int c = seed ^ 0x55AA;
    int d = seed - 42;
    int e = seed | 0xFF00;
    int f = seed & 0x00FF;
    
    /* Complex computations creating register dependencies */
    a = b * c + d;
    e = a ^ f;
    d = (b << 3) | (c >> 2);
    
    /* First call - uses some registers */
    int r1 = helper1(a, b);
    c = r1 + d;
    
    /* More computations between calls */
    f = (e * 2) - (d / 3);
    b = (a & 0xFFFF) | (c << 16);
    
    /* Second call with more arguments */
    int r2 = helper2(a, c, e);
    a = r2 ^ f;
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    e = r3 + (f * 2);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r4 = volatile_func(e, f);
    d = r4 - a;
    
    /* Fourth call after volatile call */
    int r5 = helper4(a, b, c, d, e);
    f = r5 ^ (a + b + c);
    
    /* Final computation using all variables */
    int result = (a * b) + (c * d) - (e * f) + (a ^ b ^ c ^ d ^ e ^ f);
    
    /* Use all variables to prevent dead code elimination */
    result += (a < b) ? c : d;
    result *= (e > f) ? 2 : 3;
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call test function multiple times with different inputs
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
        total += test_caller_save(i * 2);
        total += test_caller_save(i + 1000);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
