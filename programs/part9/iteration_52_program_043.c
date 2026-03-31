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
    long v8 = v7 - 7;
    long v9 = v8 / 4;
    long v10 = v9 + 20;
    long v11 = v10 * 2;
    long v12 = v11 - 3;
    long v13 = v12 / 2;
    long v14 = v13 + 15;
    long v15 = v14 * 3;
    long v16 = v15 - 8;
    long v17 = v16 / 2;
    long v18 = v17 + 25;
    long v19 = v18 * 2;
    long v20 = v19 - 9;
    
    /* Complex memory addressing to force address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex array indexing with multiple variables */
    volatile long idx1 = v1 % 50;
    volatile long idx2 = v2 % 30;
    volatile long idx3 = v3 % 20;
    
    /* This complex addressing may require reloads */
    long complex_addr = arr[idx1 + idx2 * idx3 - (v4 % 10)];
    
    /* Inline assembly with fixed register constraints */
    long asm_result;
    asm volatile (
        /* Clobber multiple specific registers */
        "movq %[input1], %%rax\n\t"
        "addq %[input2], %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=r" (asm_result)
        : [input1] "r" (v5), [input2] "r" (v6)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    /* Another asm with explicit output constraint */
    long asm_result2;
    asm volatile (
        "movl $0, %%eax\n\t"
        "addl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (asm_result2)
        : "r" (v7), "r" (v8)
        : "eax", "cc"
    );
    
    /* Complex expression using many variables - creates register pressure */
    long complex_expr = 
        v1 + v2 * v3 - v4 / (v5 + 1) + 
        v6 * v7 - v8 / (v9 + 1) + 
        v10 + v11 * v12 - v13 / (v14 + 1) + 
        v15 * v16 - v17 / (v18 + 1) + 
        v19 + v20;
    
    /* Mix different data types to force mode conversions */
    char char_var = complex_expr & 0xFF;
    short short_var = complex_expr & 0xFFFF;
    int int_var = complex_expr;
    long long_var = complex_expr;
    
    /* Operations requiring mode conversions */
    long mixed_ops = char_var + short_var * int_var - long_var / 256;
    
    /* Force function call with many parameters - uses register calling convention */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Use volatile to prevent optimization */
    volatile long final_check = 0;
    if (complex_expr > 1000) {
        final_check = mixed_ops + func_result + asm_result + asm_result2 + complex_addr;
    } else {
        final_check = mixed_ops - func_result - asm_result - asm_result2 - complex_addr;
    }
    
    /* More inline asm with mismatched constraints */
    long final_output;
    asm volatile (
        /* Force reload by using specific register for output */
        "movq %1, %%r10\n\t"
        "addq %2, %%r10\n\t"
        "movq %%r10, %0\n\t"
        : "=r" (final_output)
        : "r" (final_check), "r" (complex_expr)
        : "r10", "r11", "memory"
    );
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %ld\n", final_output);
    
    return (final_output > 0) ? 0 : 1;
}
