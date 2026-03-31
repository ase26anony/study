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
static int (*volatile volatile_func)(int, int, int, int);

/* Test function with complex register usage across multiple calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55;
    int d = seed - 19;
    int e = seed * seed;
    int f = seed | 0xFF;
    
    /* Perform arithmetic creating dependencies, keeping values live across calls */
    a = b * c + d;          /* Uses b, c, d -> result in a */
    e = a ^ f;              /* Uses a, f -> result in e */
    
    /* First call - clobbers call-clobbered registers */
    int r1 = helper1(a, b); /* Uses a, b */
    
    /* More arithmetic between calls */
    c = d * e + r1;         /* Uses d, e, r1 -> result in c */
    f = a ^ c | b;          /* Uses a, c, b -> result in f */
    
    /* Second call with more arguments */
    int r2 = helper2(c, d, e); /* Uses c, d, e */
    
    /* Complex computation creating more register dependencies */
    b = (r1 * r2) ^ (a << 2);  /* Uses r1, r2, a -> result in b */
    d = (f + e) * c - a;       /* Uses f, e, c, a -> result in d */
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d); /* Uses a, b, c, d */
    
    /* More arithmetic */
    e = (r2 ^ r3) + (b & d);     /* Uses r2, r3, b, d -> result in e */
    f = r1 * 3 - c / 2;          /* Uses r1, c -> result in f */
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(a, b, c, d); /* Uses a, b, c, d */
        e = e ^ r4;                         /* Result used after call */
    }
    
    /* Final arithmetic using all computed values */
    int result = (a * b) + (c * d) - (e * f) + (r1 ^ r2) | (r3 & 0xFF);
    
    /* Fourth call whose result is only used after more computation */
    int r5 = helper4(result, a, b, c, d); /* Uses result, a, b, c, d */
    
    /* Critical: Use r5 in computation AFTER the call */
    /* This may force save code to be inserted after the helper4 call */
    result = result + r5 * 2;
    
    /* More arithmetic after the last call */
    result = result ^ (e << 1);
    result = result | (f & 0xAA);
    
    /* Final dependency chain ensures all values are live */
    return result;
}

int main(void) {
    /* Initialize volatile function pointer */
    volatile_func = helper3;
    
    int total = 0;
    
    /* Call test function in a loop with varying arguments
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
        total ^= i * 3;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Additional loop with different seeds to increase coverage chance */
    for (int i = 1000; i < 1100; i++) {
        volatile_func = (i % 2) ? helper1 : helper2;
        total -= test_caller_save(i);
    }
    
    printf("Final result: %d\n", total);
    return 0;
}
