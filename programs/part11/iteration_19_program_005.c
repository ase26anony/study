/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper functions to force actual call instructions */
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
    return (a << 3) | (b & 0xFF);
}

/* Volatile function pointer to create unpredictable call site */
static int (*volatile volatile_func)(int, int, int) = NULL;

/* Main test function with register-intensive computations between calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed * 3;
    int b = seed + 17;
    int c = seed ^ 0xABCD;
    int d = seed - 42;
    int e = seed | 0x1234;
    int f = seed & 0xF0F0;
    
    /* First computation and call - creates live values in call-clobbered regs */
    a = b * c + d;
    int r1 = helper1(a, b);
    
    /* Use result and compute more - forcing values to stay in registers */
    e = (r1 ^ f) + c;
    d = a * e - b;
    
    /* Second call with different arguments */
    int r2 = helper2(a, e, d);
    
    /* More register-intensive computations between calls */
    f = (r2 << 2) | (d & 0xFF);
    b = c * f + r1;
    
    /* Third call - more complex */
    int r3 = helper3(a, b, c, d);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(e, f, r3);
        a = r4 ^ b;
    }
    
    /* More computations to keep values live */
    c = (r3 + a) * (b - d);
    e = helper4(c, f);
    
    /* Final computation using all values - ensures no dead code elimination */
    int result = (a ^ b) + (c | d) - (e & f) * r1 / (r2 + r3);
    
    return result;
}

/* Another volatile function for initialization */
static void init_volatile_func(void) {
    volatile_func = helper2;  /* Assign to an existing function */
}

int main(void) {
    int total = 0;
    
    /* Initialize volatile function pointer */
    init_volatile_func();
    
    /* Call test function in a loop with varying arguments
       to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
        total += test_caller_save(i * 2);
        total += test_caller_save(i ^ 0x55);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
