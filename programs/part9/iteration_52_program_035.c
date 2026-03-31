/* reload_test.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f) {
    volatile long result = a + b * c - d / (e + 1) + f;
    return result;
}

/* Another helper to create more register pressure */
__attribute__((noinline))
static int complex_calc(int x1, int x2, int x3, int x4, int x5) {
    return (x1 * x2) + (x3 / x4) - (x5 << 2);
}

int main(void) {
    /* Create register pressure with many live variables */
    register long r1 asm ("r12") = 100;
    register long r2 asm ("r13") = 200;
    volatile long v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    volatile long v7 = 7, v8 = 8, v9 = 9, v10 = 10, v11 = 11, v12 = 12;
    long v13 = 13, v14 = 14, v15 = 15, v16 = 16, v17 = 17, v18 = 18;
    long v19 = 19, v20 = 20, v21 = 21, v22 = 22, v23 = 23, v24 = 24;
    long v25 = 25, v26 = 26, v27 = 27, v28 = 28, v29 = 29, v30 = 30;
    
    /* Force mismatched modes and register classes */
    char c1 = 'A', c2 = 'B', c3 = 'C';
    short s1 = 1000, s2 = 2000, s3 = 3000;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33;
    
    /* Complex memory addressing to force address reloads */
    int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Inline assembly with fixed register constraints */
    long asm_result;
    asm volatile (
        /* Clobber specific registers to force reloads */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (asm_result)
        : [in1] "r" (v1), [in2] "r" (v2)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    /* More inline assembly with different constraints */
    long asm_result2;
    asm volatile (
        "imulq %[a], %[b]\n\t"
        "movq %[b], %[out]"
        : [out] "=r" (asm_result2)
        : [a] "r" (v3), [b] "r" (v4)
        : "cc"
    );
    
    /* Complex expression using many variables - creates register pressure */
    long complex_result = 
        v1 + v2 * v3 - v4 / (v5 + 1) + 
        v6 * v7 - v8 / (v9 + 1) +
        v10 + v11 * v12 - v13 / (v14 + 1) +
        v15 * v16 - v17 / (v18 + 1) +
        v19 + v20 * v21 - v22 / (v23 + 1) +
        v24 * v25 - v26 / (v27 + 1) +
        v28 + v29 * v30 - r1 / (r2 + 1);
    
    /* Mix different data types to force mode conversions */
    int mixed_calc = 
        c1 + c2 * c3 +          /* char operations */
        s1 - s2 / s3 +          /* short operations */
        (int)(f1 * f2) +        /* float to int */
        (int)(d1 / d2);         /* double to int */
    
    /* Force address reloads with complex array indexing */
    int addr_reload = 
        arr[v1 + v2 * v3 - v4] +
        arr[v5 + v6 * v7 - v8] +
        arr[v9 + v10 * v11 - v12] +
        arr[v13 + v14 * v15 - v16];
    
    /* Call helper function - forces parameter passing in registers */
    long func_result = helper_func(
        v1, v2, v3, v4, v5, v6
    );
    
    /* Another function call with different types */
    int calc_result = complex_calc(
        c1, s1, (int)f1, (int)d1, mixed_calc
    );
    
    /* Use volatile in condition to prevent optimization */
    volatile int condition = 1;
    if (condition) {
        /* More operations in conditional block */
        asm volatile (
            "xor %%rax, %%rax\n\t"
            "add $1, %%rax"
            : : : "rax", "cc"
        );
    }
    
    /* Final computation using all results */
    long final_result = 
        asm_result + asm_result2 + 
        complex_result + mixed_calc +
        addr_reload + func_result + calc_result;
    
    printf("Result: %ld\n", final_result);
    
    /* Use struct member addressing for more reload opportunities */
    struct nested {
        struct {
            int a;
            int b;
            long c;
        } inner;
        int x;
        int y;
    } s = {{1, 2, 3}, 4, 5};
    
    int* ptr = &s.inner.a;
    *ptr = final_result % 100;
    
    return (int)final_result % 100;
}
