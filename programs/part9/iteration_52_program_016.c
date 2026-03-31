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
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Create register pressure with many live variables */
    volatile long v1 = rand() % 100 + 1;
    register long v2 asm ("r12") = v1 + 1;  /* Pin to specific register */
    long v3 = v2 * 2;
    long v4 = v3 - 5;
    long v5 = v4 / 2;
    long v6 = v5 * 3;
    long v7 = v6 + 7;
    long v8 = v7 - v1;
    long v9 = v8 * 2;
    long v10 = v9 / 3;
    long v11 = v10 + 11;
    long v12 = v11 - v2;
    long v13 = v12 * 5;
    long v14 = v13 / 7;
    long v15 = v14 + 13;
    long v16 = v15 - v3;
    long v17 = v16 * 11;
    long v18 = v17 / 13;
    long v19 = v18 + 17;
    long v20 = v19 - v4;
    
    /* Complex array indexing with multiple variables - forces address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex memory addressing that may require reloads */
    volatile long array_access = arr[v5 + v6 * v7 - v8];
    
    /* Inline assembly with fixed register constraints and clobbers */
    long asm_result;
    __asm__ volatile (
        /* Force register conflicts with explicit constraints */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (asm_result)          /* Output in any register */
        : [in1] "r" (v10),                 /* Input in any register */
          [in2] "r" (v11)                  /* Another input */
        : "rax", "rbx", "rcx", "rdx", "cc" /* Clobber specific registers */
    );
    
    /* More inline assembly with mismatched constraints */
    long asm_result2;
    __asm__ volatile (
        "imulq %[in1], %[in2]\n\t"
        "movq %[in2], %[out]"
        : [out] "=r" (asm_result2)
        : [in1] "r" (v12),
          [in2] "0" (v13)  /* Same as output - creates constraint */
        : "cc"
    );
    
    /* Complex expression using most variables - maximizes live ranges */
    volatile long complex_expr = 
        v1 + v2 * v3 - v4 / (v5 + 1) + 
        v6 * v7 - v8 / (v9 + 1) +
        v10 + v11 * v12 - v13 / (v14 + 1) +
        v15 * v16 - v17 / (v18 + 1) +
        v19 + v20;
    
    /* Force mode mismatches with different sized operations */
    char char_var = complex_expr & 0xFF;
    int int_var = char_var * 1000;  /* Mode conversion */
    long long_var = int_var * 1000000L;
    
    /* Mixed float/double operations */
    double dbl_var = long_var * 1.5;
    float flt_var = dbl_var;  /* Mode conversion */
    
    /* Call helper function - forces parameter passing in registers */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Use volatile to prevent optimization */
    volatile long final_result = 
        asm_result + asm_result2 + complex_expr + 
        long_var + (long)dbl_var + (long)flt_var + 
        func_result + array_access;
    
    /* Print to prevent dead code elimination */
    printf("Result: %ld\n", final_result);
    
    /* Additional inline assembly with memory constraints */
    long mem_result;
    __asm__ volatile (
        "leaq (%[base], %[index], 8), %[out]\n\t"
        "movq (%[out]), %[out]"
        : [out] "=r" (mem_result)
        : [base] "r" (arr),
          [index] "r" (v7)
        : "memory"
    );
    
    printf("Memory result: %ld\n", mem_result);
    
    return 0;
}
