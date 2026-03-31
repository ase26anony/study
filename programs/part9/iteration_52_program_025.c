/* reload_coverage.c - Program to trigger GCC reload pass initialization */
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
    register long v1 asm ("r12") = rand() % 100;
    register long v2 asm ("r13") = rand() % 100;
    volatile long v3 = rand() % 100;
    volatile long v4 = rand() % 100;
    long v5 = rand() % 100;
    long v6 = rand() % 100;
    long v7 = rand() % 100;
    long v8 = rand() % 100;
    long v9 = rand() % 100;
    long v10 = rand() % 100;
    long v11 = rand() % 100;
    long v12 = rand() % 100;
    long v13 = rand() % 100;
    long v14 = rand() % 100;
    long v15 = rand() % 100;
    long v16 = rand() % 100;
    long v17 = rand() % 100;
    long v18 = rand() % 100;
    long v19 = rand() % 100;
    long v20 = rand() % 100;
    
    /* Complex memory addressing to force address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
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
    v5 = v6 + v7 * v8 - v9 / (v10 + 1) + v11;
    v12 = v13 - v14 * v15 + v16 / (v17 + 1) - v18;
    
    /* More complex chain of operations */
    long temp1 = v1 + v2 * v3 - v4 / (v5 + 1) + v6;
    long temp2 = v7 - v8 * v9 + v10 / (v11 + 1) - v12;
    long temp3 = v13 + v14 * v15 - v16 / (v17 + 1) + v18;
    long temp4 = v19 - v20 * temp1 + temp2 / (temp3 + 1) - asm_result;
    
    /* Complex array indexing with multiple variables */
    volatile long array_access = arr[v1 + v2 * v3 - v4 / (v5 + 1) + 10];
    
    /* Force mode mismatches */
    char char_var = 65;
    int int_var = char_var * 1000;  /* char to int conversion */
    double double_var = int_var * 1.5;
    float float_var = double_var;   /* double to float conversion */
    
    /* Use volatile in condition to prevent elimination */
    if (v3 > 50) {
        /* More operations when condition is true */
        temp4 += array_access * 2;
    }
    
    /* Call helper function with many arguments - forces register passing */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Additional inline assembly with output constraints */
    long final_result;
    asm volatile (
        "leaq (%1, %2, 4), %0\n\t"
        : "=r" (final_result)
        : "r" (func_result), "r" (temp4)
        : "cc"
    );
    
    /* Use all variables to prevent dead code elimination */
    printf("Result: %ld\n", final_result + v7 + v8 + v9 + v10 + v11 + v12 + 
           v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 + int_var + (long)float_var);
    
    return 0;
}
