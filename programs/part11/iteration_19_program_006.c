/* caller-save-test.c */
#include <stdio.h>
#include <stdint.h>

/* Non-inline helper functions to force call instructions */
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
volatile func_ptr_t volatile_func = helper1;

/* Main test function with register-intensive computations */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x55AA;
    int d = seed - 100;
    int e = seed << 3;
    int f = seed | 0xFF00;
    
    /* Complex arithmetic creating dependencies between variables */
    a = b * c + d;
    e = a ^ f;
    d = (b << 2) | (c >> 1);
    
    /* First call - uses some registers */
    int r1 = helper1(a, b);
    
    /* More arithmetic between calls */
    c = r1 * d + e;
    f = (a & b) | (c ^ d);
    
    /* Second call with more arguments */
    int r2 = helper2(a, b, c);
    
    /* Complex computation using results */
    e = r1 * r2 + a - b;
    d = (e << 3) ^ (f >> 1);
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    
    /* More arithmetic */
    f = r2 * r3 - e;
    a = (b ^ c) & (d | e);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r4 = volatile_func(f, a);
    
    /* More computations after volatile call */
    b = r3 + r4 * 2;
    c = (a << 1) | (b >> 1);
    
    /* Fourth call with many arguments */
    int r5 = helper4(a, b, c, d, e);
    
    /* Final complex computation using all values */
    int result = (r1 ^ r2) + (r3 & r4) * r5 - (a | b) + (c ^ d) - (e & f);
    
    /* Ensure all values are used to prevent dead code elimination */
    result += (a + b + c + d + e + f + r1 + r2 + r3 + r4 + r5);
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
        total ^= test_caller_save(i * 3 + 1);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
