/* reload_test.c - Test program to trigger GCC reload pass initialization */
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
    long v10 = v9 + 15;
    long v11 = v10 * 2;
    long v12 = v11 - 3;
    long v13 = v12 / 2;
    long v14 = v13 + 8;
    long v15 = v14 * 3;
    long v16 = v15 - 9;
    long v17 = v16 / 2;
    long v18 = v17 + 20;
    long v19 = v18 * 2;
    long v20 = v19 - 5;
    
    /* Additional variables to increase pressure */
    long v21 = v20 + 1, v22 = v21 * 2, v23 = v22 - 3;
    long v24 = v23 / 2, v25 = v24 + 4, v26 = v25 * 3;
    long v27 = v26 - 2, v28 = v27 / 2, v29 = v28 + 6;
    long v30 = v29 * 2;
    
    /* Inline assembly with fixed register constraints */
    long asm_result;
    asm volatile (
        /* Complex operation that clobbers multiple registers */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "imulq %[in3], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (asm_result)
        : [in1] "r" (v2), [in2] "r" (v3), [in3] "r" (v4)
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    /* Complex expression using many variables - creates register pressure */
    long complex_result = 
        v1 + v2 * v3 - v4 / (v5 + 1) + 
        v6 * v7 - v8 / (v9 + 1) +
        v10 + v11 * v12 - v13 / (v14 + 1) +
        v15 * v16 - v17 / (v18 + 1) +
        v19 + v20 * v21 - v22 / (v23 + 1) +
        v24 * v25 - v26 / (v27 + 1) +
        v28 + v29 * v30 - asm_result;
    
    /* Force address reloads with complex memory addressing */
    long array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * i;
    }
    
    /* Complex array indexing - may require address reloads */
    long idx_result = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Complex index expression */
            idx_result += array[(i * v1 + j * v2) % 100] * 
                         array[(i * v3 + j * v4) % 100];
        }
    }
    
    /* Call helper function - forces parameter passing in registers */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Use mismatched types to force mode conversions */
    char char_var = complex_result % 256;
    short short_var = complex_result % 65536;
    int int_var = complex_result;
    long long_var = complex_result;
    
    /* Mixed mode operations */
    double double_result = (double)char_var * 1.5 + 
                          (double)short_var * 2.5 +
                          (double)int_var * 3.5 +
                          (double)long_var * 4.5;
    
    /* Another inline asm with different constraints */
    long final_result;
    asm volatile (
        "leaq (%[a], %[b], 4), %%rax\n\t"
        "addq %[c], %%rax\n\t"
        "movq %%rax, %[res]\n\t"
        : [res] "=r" (final_result)
        : [a] "r" (complex_result), 
          [b] "r" (idx_result),
          [c] "r" (func_result)
        : "rax", "cc"
    );
    
    /* Use volatile to prevent optimization */
    volatile long output = final_result + (long)double_result;
    
    printf("Result: %ld\n", output);
    return 0;
}
