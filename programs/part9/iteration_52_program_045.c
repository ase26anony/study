/* reload_trigger.c - Program to trigger GCC's reload pass initialization */
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
    register long v1 asm ("r12") = rand() % 100 + 1;
    register long v2 asm ("r13") = rand() % 100 + 1;
    volatile long v3 = rand() % 100 + 1;
    volatile long v4 = rand() % 100 + 1;
    long v5 = rand() % 100 + 1;
    long v6 = rand() % 100 + 1;
    long v7 = rand() % 100 + 1;
    long v8 = rand() % 100 + 1;
    long v9 = rand() % 100 + 1;
    long v10 = rand() % 100 + 1;
    long v11 = rand() % 100 + 1;
    long v12 = rand() % 100 + 1;
    long v13 = rand() % 100 + 1;
    long v14 = rand() % 100 + 1;
    long v15 = rand() % 100 + 1;
    long v16 = rand() % 100 + 1;
    long v17 = rand() % 100 + 1;
    long v18 = rand() % 100 + 1;
    long v19 = rand() % 100 + 1;
    long v20 = rand() % 100 + 1;
    
    /* Complex addressing mode to force address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Inline assembly with fixed register constraints */
    long asm_result;
    asm volatile (
        /* Clobber multiple registers to force reloads */
        "movq %1, %%rax\n\t"
        "addq %2, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (asm_result)
        : "r" (v1), "r" (v2)
        : "%rax", "%rbx", "%rcx", "%rdx", "memory"
    );
    
    /* Complex expression using most variables - creates register pressure */
    long complex_result = 
        v1 + v2 * v3 - v4 / (v5 + 1) +
        v6 * v7 - v8 + v9 * v10 +
        v11 - v12 * v13 + v14 / (v15 + 1) +
        v16 + v17 * v18 - v19 + v20 +
        asm_result;
    
    /* Force mode mismatch: char in int operation */
    char char_var = 42;
    complex_result += char_var * 100;  /* char promoted to int */
    
    /* More inline assembly with mismatched constraints */
    long another_result;
    asm volatile (
        "imulq %1, %2\n\t"
        "movq %2, %0\n\t"
        : "=r" (another_result), "+r" (v3)
        : "r" (v4)
        : "%rax", "%rdx"
    );
    
    /* Complex array indexing - forces address computation reloads */
    long array_access = arr[v1 + v2 * v3 - v4];
    
    /* Use volatile variable in condition to prevent elimination */
    if (v3 > 50) {
        complex_result += array_access;
    } else {
        complex_result -= array_access;
    }
    
    /* Force function call with many parameters - uses register ABI */
    long func_result = helper_func(v5, v6, v7, v8, v9, v10);
    
    /* Final computation using all results */
    long final_result = complex_result + another_result + func_result;
    
    /* Use the result to prevent optimization */
    printf("Final result: %ld\n", final_result);
    
    /* Additional inline assembly with explicit output constraints */
    long final_asm;
    asm volatile (
        "leaq (%1, %2, 4), %0\n\t"
        : "=r" (final_asm)
        : "r" (final_result), "r" (v11)
        : "cc"
    );
    
    printf("ASM result: %ld\n", final_asm);
    
    return (final_result > 0) ? 0 : 1;
}
