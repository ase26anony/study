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
static int (*volatile volatile_func)(int, int, int, int, int) = helper4;

/* Test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple variables to create register pressure */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x55AA;
    int d = seed - 100;
    int e = seed * seed;
    int f = seed | 0xFF00;
    
    /* First computation using multiple registers */
    int t1 = a * b + c;
    int t2 = d ^ e ^ f;
    
    /* First call - clobbers caller-saved registers */
    int r1 = helper1(t1, t2);
    
    /* Interleaved computation between calls */
    a = r1 + b;
    c = t2 * d;
    e = a ^ c;
    
    /* Second call with different arguments */
    int r2 = helper2(a, c, e);
    
    /* More computations creating live values across calls */
    b = r2 * t1;
    d = e + r1;
    f = b ^ d;
    
    /* Third call with more arguments */
    int r3 = helper3(b, d, f, r2);
    
    /* Complex computation chain */
    t1 = r3 * a;
    t2 = b + c;
    int t3 = d ^ e;
    int t4 = f * r1;
    
    /* Volatile function pointer call - unpredictable for compiler */
    int r4 = volatile_func(t1, t2, t3, t4, r3);
    
    /* Final computation using all values to prevent elimination */
    int result = (r1 * r2) + (r3 ^ r4) - (a + b + c + d + e + f);
    
    /* Additional computation after the volatile call */
    /* This creates a scenario where save/restore might be needed after the call */
    int post_call_compute = (result * 2) + (t1 - t2);
    
    /* Use all variables in final return to prevent dead code elimination */
    return result + post_call_compute + (r1 & r2) + (r3 | r4);
}

/* Another helper to increase call density */
__attribute__((noinline)) int helper5(int x) {
    return x * 3 + 1;
}

/* Main function with loop to prevent constant propagation */
int main() {
    int total = 0;
    
    /* Loop with varying input to prevent optimization */
    for (int i = 0; i < 100; i++) {
        /* Mix of different computations to vary register usage */
        int val = test_caller_save(i);
        
        /* Additional calls to create more caller-save opportunities */
        if (i % 3 == 0) {
            val += helper5(val);
        }
        
        total += val;
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 7 == 0) {
            volatile int dummy = i;
            (void)dummy;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
