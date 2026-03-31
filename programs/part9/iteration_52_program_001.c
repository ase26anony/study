/* reload_trigger.c - Program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f) {
    volatile long result = a + b * c - d / (e + 1) + f;
    return result;
}

int main(void) {
    /* Create register pressure with many live variables */
    register long v1 asm ("r12") = 1;
    register long v2 asm ("r13") = 2;
    volatile long v3 = 3;
    long v4 = 4;
    long v5 = 5;
    long v6 = 6;
    long v7 = 7;
    long v8 = 8;
    long v9 = 9;
    long v10 = 10;
    long v11 = 11;
    long v12 = 12;
    long v13 = 13;
    long v14 = 14;
    long v15 = 15;
    long v16 = 16;
    long v17 = 17;
    long v18 = 18;
    long v19 = 19;
    long v20 = 20;
    
    /* Force mode mismatches */
    char c1 = 100;
    short s1 = 2000;
    int i1 = 30000;
    float f1 = 4.5f;
    double d1 = 6.7;
    
    /* Complex array addressing for address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Inline assembly with fixed register constraints */
    long asm_result;
    asm volatile (
        "movq %[input], %%rax\n\t"      /* Force use of rax */
        "addq $100, %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=r" (asm_result)
        : [input] "r" (v1)
        : "rax", "rbx", "rcx", "rdx"    /* Clobber multiple registers */
    );
    
    /* Another inline asm with explicit output constraint */
    long asm_result2;
    asm volatile (
        "imulq %[in1], %[in2], %[out]\n\t"
        : [out] "=r" (asm_result2)
        : [in1] "r" (v2), [in2] "r" (v3)
        : "cc"
    );
    
    /* Complex expression using most variables - creates register pressure */
    v4 = v5 + v6 * v7 - v8 / (v9 + 1) + v10;
    v11 = v12 * v13 + v14 - v15 / (v16 + v17 - v18);
    v19 = v20 + asm_result * asm_result2 - v1;
    
    /* Mix types to force mode conversions */
    i1 = c1 + s1;               /* char + short -> int conversion */
    d1 = f1 + i1;               /* float + int -> double conversion */
    
    /* Complex array indexing */
    long idx = v4 + v11 * v19 - v3;
    volatile long array_val = arr[(idx + v5 * v6 - v7) % 100];
    
    /* Force address computation reloads with nested struct */
    struct inner {
        long a;
        long b;
        long c[5];
    };
    
    struct outer {
        struct inner in1;
        struct inner in2;
        long extra;
    } outer_var;
    
    /* Take address of nested member */
    long *ptr = &outer_var.in2.c[2];
    *ptr = v4 + v11;
    
    /* Use volatile in condition to prevent elimination */
    if (v3 > 0) {
        v4 = v5 + v6;
        /* More inline asm with constraints */
        asm volatile (
            "movq %1, %%r10\n\t"
            "addq %2, %%r10\n\t"
            "movq %%r10, %0\n\t"
            : "=r" (v7)
            : "r" (v8), "r" (v9)
            : "r10", "cc"
        );
    }
    
    /* Call helper with many arguments - forces register parameter passing */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Use all variables in final computation to prevent dead code elimination */
    long final_result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
        i1 + (long)d1 + asm_result + asm_result2 + array_val + *ptr + func_result;
    
    printf("Result: %ld\n", final_result);
    return (int)(final_result % 256);
}
