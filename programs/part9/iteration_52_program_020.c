/* reload_test.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f) {
    volatile long result = a + b - c * d / (e + f + 1);
    return result;
}

int main(void) {
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Create register pressure with many live variables */
    volatile long v1 = rand() % 100;
    register long v2 asm ("r12") = v1 + 1;  /* Pin to specific register */
    long v3 = v2 * 2;
    long v4 = v3 - 5;
    long v5 = v4 / 2;
    long v6 = v5 + 10;
    long v7 = v6 * 3;
    long v8 = v7 - 15;
    long v9 = v8 / 4;
    long v10 = v9 + 20;
    long v11 = v10 * 5;
    long v12 = v11 - 25;
    long v13 = v12 / 6;
    long v14 = v13 + 30;
    long v15 = v14 * 7;
    long v16 = v15 - 35;
    long v17 = v16 / 8;
    long v18 = v17 + 40;
    long v19 = v18 * 9;
    long v20 = v19 - 45;
    
    /* Complex array indexing with multiple variables - forces address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex memory addressing with multiple index variables */
    volatile long array_access = arr[(v1 + v2 * v3 - v4 / (v5 + 1)) % 100];
    
    /* Inline assembly with fixed register constraints and clobbers */
    long asm_result;
    asm volatile (
        /* Force specific register usage and create conflicts */
        "movq %[input1], %%rax\n\t"
        "addq %[input2], %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=r" (asm_result)      /* Output in any register */
        : [input1] "r" (v10),             /* Input in any register */
          [input2] "r" (v11)              /* Another input */
        : "rax", "rbx", "rcx", "rdx"      /* Clobber multiple registers */
    );
    
    /* Another inline asm with explicit output register constraint */
    long asm_result2;
    register long fixed_reg asm ("r10") = v15;
    asm volatile (
        "movq %1, %0\n\t"
        "imulq $37, %0\n\t"
        : "=r" (asm_result2)              /* Output constraint */
        : "r" (fixed_reg)                 /* Input from pinned register */
        : "r10"                           /* Clobber the pinned register */
    );
    
    /* Complex expression using most variables - maximizes live ranges */
    long complex_expr = 
        v1 + v2 * v3 - v4 / (v5 + 1) +
        v6 * v7 - v8 / (v9 + 2) +
        v10 + v11 * v12 - v13 / (v14 + 3) +
        v15 * v16 - v17 / (v18 + 4) +
        v19 + v20;
    
    /* Mix different data types to force mode conversions */
    {
        char c1 = complex_expr & 0xFF;
        short s1 = complex_expr & 0xFFFF;
        int i1 = complex_expr;
        double d1 = (double)complex_expr * 1.5;
        float f1 = (float)complex_expr * 0.5f;
        
        /* Operations mixing types force reloads with different modes */
        volatile double mixed_op = d1 + f1 + i1 + s1 + c1;
    }
    
    /* Force function call with many parameters - uses register ABI */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Use volatile condition to prevent optimization */
    volatile int condition = (complex_expr > 1000);
    if (condition) {
        /* More complex operations in conditional path */
        long temp = v20;
        for (int i = 0; i < 10; i++) {
            temp = temp * 2 + arr[i % 100];
        }
        complex_expr += temp;
    }
    
    /* Final computation using all results */
    long final_result = 
        complex_expr + asm_result + asm_result2 + array_access + func_result;
    
    printf("Result: %ld\n", final_result);
    return (int)(final_result % 100);
}
