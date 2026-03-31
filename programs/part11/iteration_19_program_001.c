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
    return a * b + c * d - e;
}

/* Volatile function pointer to create unpredictable call site */
static int (*volatile volatile_func)(int, int, int, int, int) = helper4;

/* Main test function with register-intensive computations between calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 17;
    int c = seed ^ 0x55AA;
    int d = seed - 42;
    int e = seed * seed;
    int f = seed | 0xFF00;
    
    /* Complex arithmetic to keep values live in registers */
    a = b * c + d;
    b = a ^ f + c;
    c = d * e - a;
    d = b | c ^ f;
    e = a + b + c + d;
    f = (a * b) ^ (c * d);
    
    /* First call - uses some registers */
    int r1 = helper1(a, b);
    
    /* More arithmetic between calls to keep values live */
    a = r1 + b * 3;
    b = c ^ d + r1;
    c = a * d - e;
    
    /* Second call - uses different registers */
    int r2 = helper2(a, b, c);
    
    /* More computations creating dependencies */
    d = r1 ^ r2 + f;
    e = a * b + c * d;
    f = (r1 | r2) ^ (a & b);
    
    /* Third call - uses many registers */
    int r3 = helper3(a, b, c, d);
    
    /* Complex chain of computations */
    a = r3 * e + f;
    b = (r1 + r2) ^ r3;
    c = d * e - a * b;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r4 = volatile_func(a, b, c, d, e);
    
    /* Final computations using all values to prevent dead code elimination */
    int result = (r1 * r2) + (r3 ^ r4) - (a & b) | (c * d) ^ (e + f);
    
    /* Use all variables in final result */
    result += (a >> 2) + (b << 3) - (c & 0xFF) + (d | 0xAA) ^ (e * 2) + (f / 3);
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
        total ^= test_caller_save(i * 2 + 1);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
