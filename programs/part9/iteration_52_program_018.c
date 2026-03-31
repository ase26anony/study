/* reload_trigger.c - Program to trigger GCC reload pass initialization */
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
    /* Use volatile to prevent optimization */
    volatile long v1 = rand() % 100 + 1;
    volatile long v2 = rand() % 100 + 2;
    volatile long v3 = rand() % 100 + 3;
    volatile long v4 = rand() % 100 + 4;
    volatile long v5 = rand() % 100 + 5;
    volatile long v6 = rand() % 100 + 6;
    volatile long v7 = rand() % 100 + 7;
    volatile long v8 = rand() % 100 + 8;
    volatile long v9 = rand() % 100 + 9;
    volatile long v10 = rand() % 100 + 10;
    volatile long v11 = rand() % 100 + 11;
    volatile long v12 = rand() % 100 + 12;
    volatile long v13 = rand() % 100 + 13;
    volatile long v14 = rand() % 100 + 14;
    volatile long v15 = rand() % 100 + 15;
    volatile long v16 = rand() % 100 + 16;
    volatile long v17 = rand() % 100 + 17;
    volatile long v18 = rand() % 100 + 18;
    volatile long v19 = rand() % 100 + 19;
    volatile long v20 = rand() % 100 + 20;
    
    /* Force specific registers with explicit register variables */
    register long r12_var asm("r12") = v1 + v2;
    register long r13_var asm("r13") = v3 * v4;
    register long r14_var asm("r14") = v5 - v6;
    register long r15_var asm("r15") = v7 / (v8 + 1);
    
    /* Inline assembly with fixed register constraints */
    /* This forces reloads by clobbering specific registers */
    long asm_result;
    asm volatile (
        /* Complex operation requiring multiple registers */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "imulq %[in3], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (asm_result)
        : [in1] "r" (r12_var), 
          [in2] "r" (r13_var), 
          [in3] "r" (r14_var)
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    /* Another inline asm with mismatched constraints */
    long asm_result2;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (asm_result2)
        : "r" ((int)v9), "r" ((int)v10)
        : "eax", "ebx"
    );
    
    /* Complex expression using most variables - creates register pressure */
    long complex_expr = 
        v1 + v2 * v3 - v4 / (v5 + 1) +
        v6 * v7 - v8 + v9 / (v10 + 2) +
        v11 - v12 * v13 + v14 / (v15 + 3) +
        v16 + v17 - v18 * v19 + v20;
    
    /* Force address reloads with complex memory addressing */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex array indexing - may require address reloads */
    long array_access = 
        arr[v1 + v2 * 3] +
        arr[v3 + v4 * 5] +
        arr[v5 + v6 * 7] +
        arr[v7 + v8 * 9];
    
    /* Mix different data types to force mode conversions */
    {
        char c1 = v1 & 0xFF;
        short s1 = v2 & 0xFFFF;
        int i1 = v3;
        long l1 = v4;
        
        /* Operations requiring mode conversions */
        long mixed = c1 + s1 * i1 - l1 / (c1 + 1);
        complex_expr += mixed;
    }
    
    /* Force conditional with volatile to prevent elimination */
    volatile int condition = (complex_expr > 1000);
    if (condition) {
        /* Call helper function - forces parameter passing in registers */
        long func_result = helper_func(
            v1, v2, v3, v4, v5, v6
        );
        complex_expr += func_result;
    } else {
        /* Alternative path with more register pressure */
        long alt_expr = v11 * v12 + v13 - v14 / v15 +
                       v16 * v17 - v18 + v19 / v20;
        complex_expr += alt_expr;
    }
    
    /* Use all results to prevent optimization */
    long final_result = complex_expr + asm_result + asm_result2 + array_access;
    
    printf("Result: %ld\n", final_result);
    return (int)(final_result % 256);
}
