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

__attribute__((noinline)) int helper4(int a, int b) {
    return a * 3 + b * 5;
}

/* Volatile function pointer to prevent optimization */
typedef int (*func_ptr)(int, int);
volatile func_ptr volatile_func = helper4;

/* Test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Multiple local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 17;
    int c = seed ^ 0xABCD;
    int d = seed - 42;
    int e = seed * seed;
    int f = seed | 0x1234;
    
    /* First computation keeping values live across call */
    a = b * c + d;          /* Uses b, c, d -> likely in registers */
    e = a ^ f;              /* Uses a, f -> more register pressure */
    
    /* First call - clobbers caller-saved registers */
    int r1 = helper1(a, b); /* a, b used as args, but e, c, d, f still live */
    
    /* Intervening computations to keep values in registers */
    c = d * e + r1;         /* Uses d, e, r1 */
    f = c ^ a;              /* Uses c, a */
    
    /* Second call with more arguments */
    int r2 = helper2(c, d, e); /* c, d, e as args, but f, a, b, r1 still live */
    
    /* More computations creating complex live ranges */
    b = a * r1 + r2;        /* Uses a, r1, r2 */
    d = f ^ b;              /* Uses f, b */
    
    /* Third call with many arguments */
    int r3 = helper3(a, b, c, d); /* a, b, c, d as args, e, f, r1, r2 live */
    
    /* Volatile function pointer call - compiler can't analyze */
    int r4 = volatile_func(e, f); /* e, f as args, others still live */
    
    /* Final computation using all values to prevent elimination */
    int result = (a + b) ^ (c - d) | (e * f) & (r1 + r2) - (r3 ^ r4);
    
    return result;
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
