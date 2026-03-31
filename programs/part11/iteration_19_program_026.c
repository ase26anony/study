/* caller-save-test.c
 * Designed to trigger GCC's caller-save instruction reordering logic
 * Specifically targets lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <stdint.h>

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
    return a + b * c - d / (e + 1);
}

/* Volatile function pointer to create unpredictable call site */
typedef int (*func_ptr_t)(int, int, int, int);
volatile func_ptr_t volatile_func = NULL;

/* Main test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x55AA;
    int d = seed - 100;
    int e = seed << 3;
    int f = seed >> 2;
    
    /* Complex arithmetic creating dependencies between variables */
    a = b * c + d;          /* Uses b, c, d -> multiple call-clobbered regs */
    e = a ^ f;              /* Uses a, f -> creates live value across calls */
    
    /* First call - uses some registers */
    int r1 = helper1(a, b);
    c = r1 * d;             /* Result used immediately -> register pressure */
    
    /* More arithmetic between calls */
    f = (c << 2) | (d >> 1);
    b = e + f * 3;
    
    /* Second call with more arguments */
    int r2 = helper2(a, b, c);
    d = r2 ^ e;             /* Creates dependency across calls */
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    e = r3 + f;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    if (volatile_func) {
        int r4 = volatile_func(a, b, c, d);
        f = r4 & 0xFF;
    }
    
    /* More arithmetic after volatile call */
    a = (b << 1) + (c >> 2) - d;
    b = e * f + a;
    
    /* Fourth call with many arguments */
    int r5 = helper4(a, b, c, d, e);
    c = r5 ^ f;
    
    /* Final complex computation using all variables */
    int result = (a + b) * (c - d) ^ (e | f);
    
    /* Use all variables in final result to prevent dead code elimination */
    result += (a & 0xF) + (b & 0xF0) + (c & 0xF00) + 
              (d & 0xF000) + (e & 0xF0000) + (f & 0xF00000);
    
    return result;
}

/* Another helper to create more call sites */
__attribute__((noinline)) int helper5(int x) {
    return x * x + x;
}

/* Additional test function to create more complex patterns */
__attribute__((noinline)) int nested_calls(int x) {
    int a = helper1(x, x+1);
    int b = helper2(x, a, x+2);
    int c = helper3(x, a, b, x+3);
    int d = helper4(x, a, b, c, x+4);
    return helper5(d);
}

int main() {
    int total = 0;
    
    /* Initialize volatile function pointer */
    volatile_func = (func_ptr_t)helper3;
    
    /* Loop to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        /* Varying arguments to prevent optimization */
        int val1 = test_caller_save(i);
        int val2 = nested_calls(i ^ 0x1234);
        
        /* Use results to prevent dead code elimination */
        total += val1 + val2;
        
        /* Change volatile pointer occasionally */
        if (i % 3 == 0) {
            volatile_func = (func_ptr_t)helper4;
        } else if (i % 7 == 0) {
            volatile_func = (func_ptr_t)helper2;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
