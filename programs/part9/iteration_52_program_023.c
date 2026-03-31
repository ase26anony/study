/* reload_trigger.c
 * Designed to trigger GCC's reload pass initialization logic
 * Compile with: gcc -O2 -fomit-frame-pointer -march=x86-64 reload_trigger.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f) {
    volatile long barrier = a + b + c + d + e + f;
    return barrier * 2;
}

/* Another helper that uses fixed register constraints */
__attribute__((noinline))
static long fixed_reg_helper(long x, long y) {
    long result;
    /* Force specific register usage with inline asm */
    asm volatile (
        "addq %1, %0\n\t"
        "subq $1, %0"
        : "=r" (result)
        : "r" (x), "0" (y)
        : "cc", "rax", "rbx", "rcx"
    );
    return result;
}

int main(void) {
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    
    /* Create register pressure with many live variables */
    volatile long v1 = rand() % 100;
    register long v2 asm ("r12") = v1 + 1;  /* Pin to specific register */
    long v3 = v2 * 2;
    long v4 = v3 - 5;
    register long v5 asm ("r13") = v4 / 3;  /* Another pinned register */
    long v6 = v5 + 7;
    long v7 = v6 * 11;
    long v8 = v7 - 13;
    long v9 = v8 + 17;
    long v10 = v9 * 19;
    long v11 = v10 - 23;
    long v12 = v11 + 29;
    long v13 = v12 * 31;
    long v14 = v13 - 37;
    long v15 = v14 + 41;
    long v16 = v15 * 43;
    long v17 = v16 - 47;
    long v18 = v17 + 53;
    long v19 = v18 * 59;
    long v20 = v19 - 61;
    
    /* Complex array indexing to force address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex addressing mode - forces address computation reloads */
    volatile long idx1 = v1 % 50;
    volatile long idx2 = v2 % 30;
    volatile long idx3 = v3 % 20;
    
    /* This complex addressing should trigger reloads */
    long complex_addr_result = arr[idx1 + idx2 * idx3 + v4 % 10];
    
    /* Inline assembly with fixed register constraints */
    long asm_result;
    asm volatile (
        "movq %1, %%rax\n\t"
        "addq %2, %%rax\n\t"
        "imulq %3, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (asm_result)
        : "r" (v5), "r" (v6), "r" (v7)
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    /* More inline asm with mismatched constraints */
    long in_val = v8;
    long out_val;
    asm volatile (
        "movl %1, %0\n\t"  /* Using movl (32-bit) with 64-bit values */
        : "=r" (out_val)
        : "r" (in_val)
        : "%eax", "%ebx"
    );
    
    /* Complex expression using many variables - maximizes live ranges */
    long complex_expr = 
        v1 + v2 * v3 - v4 / (v5 + 1) + 
        v6 * v7 - v8 + v9 * v10 - v11 / (v12 + 1) +
        v13 - v14 * v15 + v16 / (v17 + 1) - 
        v18 + v19 * v20;
    
    /* Force mode mismatches */
    char char_var = complex_expr & 0xFF;
    int int_var = char_var * 1000;  /* Mode conversion */
    long long_var = int_var * 1000000L;
    double double_var = long_var * 1.5;
    float float_var = double_var;  /* Another mode conversion */
    
    /* Use volatile to prevent optimization */
    volatile float volatile_float = float_var;
    
    /* Force conditional with complex computation */
    if (volatile_float > 0) {
        complex_expr += (long)(volatile_float * 2);
    }
    
    /* Call helper function - forces parameter passing in registers */
    long helper_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Call fixed register helper */
    long fixed_result = fixed_reg_helper(v7, v8);
    
    /* More complex operations with pinned registers */
    v2 = v2 + v3;  /* r12 is pinned */
    v5 = v5 - v4;  /* r13 is pinned */
    
    /* Final computation using all variables */
    long final_result = 
        complex_expr + 
        asm_result + 
        out_val + 
        complex_addr_result + 
        helper_result + 
        fixed_result +
        v2 + v5 + v9 + v10 + v11 + v12 + v13 + v14 + v15 + 
        v16 + v17 + v18 + v19 + v20;
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %ld\n", final_result % 1000000);
    
    return (final_result > 0) ? 0 : 1;
}
