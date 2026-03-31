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
    return a + (b << 2) - (c >> 1) + d * e;
}

/* Volatile function pointer to create unpredictable call site */
static int (*volatile volatile_func)(int, int, int, int) = NULL;

/* Main test function with register-intensive computations */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3 + 1;
    int b = seed * 5 - 2;
    int c = seed * 7 + 3;
    int d = seed * 11 - 4;
    int e = seed * 13 + 5;
    int f = seed * 17 - 6;
    int g = seed * 19 + 7;
    int h = seed * 23 - 8;
    
    /* Complex computation chain creating dependencies across calls */
    
    /* First computation - uses multiple registers */
    a = b * c + d;
    b = a ^ e;
    c = b + f * g;
    
    /* First call - clobbers call-clobbered registers */
    int r1 = helper1(a, b);
    
    /* More computations reusing variables */
    d = r1 * c + a;
    e = d ^ b;
    f = e + c * 2;
    
    /* Second call with more arguments */
    int r2 = helper2(d, e, f);
    
    /* More register-intensive computations */
    g = r2 * a + b;
    h = g ^ c;
    a = h + d * e;
    
    /* Third call with even more arguments */
    int r3 = helper3(g, h, a, b);
    
    /* More computations */
    b = r3 * c + d;
    c = b ^ e;
    d = c + f * g;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(b, c, d, e);
        e = r4 * a + b;
    }
    
    /* Fourth call - after volatile call */
    int r5 = helper4(c, d, e, f, g);
    
    /* Final computation using all values to prevent dead code elimination */
    int result = (a + b) ^ (c - d) | (e * f) & (g + h) + (r1 ^ r2) - (r3 | r5);
    
    return result;
}

int main(int argc, char **argv) {
    /* Initialize volatile function pointer */
    volatile_func = helper3;  /* Use helper3 as the target */
    
    int total = 0;
    
    /* Call test function multiple times with different seeds
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
    }
    
    printf("Result: %d\n", total);
    
    /* Also test with NULL volatile function pointer */
    volatile_func = NULL;
    total += test_caller_save(1000);
    
    printf("Final result: %d\n", total);
    
    return 0;
}
