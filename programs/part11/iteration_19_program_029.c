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
    return (a * b) + (c * d) - e;
}

/* Volatile function pointer to create unpredictable call site */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func = helper1;

/* Main test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x55AA;
    int d = seed - 123;
    int e = seed * seed;
    int f = seed | 0xFF00;
    
    /* Complex arithmetic creating register dependencies */
    a = b * c + d;
    b = a ^ f + e;
    c = (d << 3) | (e >> 2);
    d = a * b - c;
    e = (b & c) | (d ^ a);
    f = c * d + e;
    
    /* First call - uses some registers */
    int r1 = helper1(a, b);
    /* Use result immediately to keep it live */
    a = r1 + c;
    
    /* More arithmetic between calls */
    b = a * d - e;
    c = (b ^ f) + r1;
    
    /* Second call with more arguments */
    int r2 = helper2(a, b, c);
    /* Complex dependency chain */
    d = r2 * a + b;
    e = (r2 ^ d) | c;
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    f = r3 + e * 2;
    
    /* More arithmetic creating cross-call dependencies */
    a = b * c + d * e - f;
    b = (a << 2) | (c >> 1);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r4 = volatile_func(r3, f);
    
    /* Critical: Use result of volatile call immediately in next computation
       This may force save code insertion AFTER the volatile call */
    c = r4 * a + b;
    
    /* Fourth call with many arguments - maximum register pressure */
    int r5 = helper4(a, b, c, d, e);
    
    /* Final complex computation using all values */
    int result = (a * r1) + (b * r2) - (c * r3) + (d * r4) - (e * r5) + f;
    
    /* Use all variables in return to prevent dead code elimination */
    return result ^ (a + b + c + d + e + f + r1 + r2 + r3 + r4 + r5);
}

/* Additional function to create more call sites in same basic block */
__attribute__((noinline)) int test_caller_save2(int seed) {
    int x = seed * 3;
    int y = seed + 456;
    
    /* Multiple calls in sequence with minimal computation between */
    int t1 = helper1(x, y);
    int t2 = helper2(t1, x, y);
    int t3 = helper3(t1, t2, x, y);
    int t4 = volatile_func(t2, t3);
    
    /* This structure encourages the compiler to insert save/restore
       instructions between these closely spaced calls */
    return t1 + t2 * 2 - t3 + t4 * 3;
}

int main() {
    int total = 0;
    
    /* Loop with varying arguments to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        /* Mix two different test patterns */
        if (i % 3 == 0) {
            total += test_caller_save(i);
        } else {
            total += test_caller_save2(i);
        }
        
        /* Change volatile function pointer occasionally */
        if (i % 7 == 0) {
            volatile_func = helper1;
        } else if (i % 7 == 3) {
            volatile_func = (func_ptr_t)helper2;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
