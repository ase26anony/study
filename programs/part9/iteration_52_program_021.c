/* reload_trigger.c - Program to trigger GCC's reload pass initialization */
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
    volatile long v1 = rand() % 100 + 1;
    register long v2 asm ("r12") = v1 + 1;  /* Pin to specific register */
    long v3 = v2 * 2;
    long v4 = v3 - v1;
    long v5 = v4 / 3;
    long v6 = v5 * 7;
    long v7 = v6 + v2;
    long v8 = v7 - v3;
    long v9 = v8 * v4;
    long v10 = v9 / v5;
    long v11 = v10 + v6;
    long v12 = v11 - v7;
    long v13 = v12 * v8;
    long v14 = v13 / v9;
    long v15 = v14 + v10;
    long v16 = v15 - v11;
    long v17 = v16 * v12;
    long v18 = v17 / v13;
    long v19 = v18 + v14;
    long v20 = v19 - v15;
    
    /* Additional variables to increase pressure */
    long v21 = v20 * v16;
    long v22 = v21 / v17;
    long v23 = v22 + v18;
    long v24 = v23 - v19;
    long v25 = v24 * v20;
    long v26 = v25 / v21;
    long v27 = v26 + v22;
    long v28 = v27 - v23;
    long v29 = v28 * v24;
    long v30 = v29 / v25;
    
    /* Inline assembly with fixed register constraints */
    long asm_result;
    __asm__ volatile (
        /* Complex operation with multiple clobbers */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "imulq %[in3], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (asm_result)
        : [in1] "r" (v2), 
          [in2] "r" (v3),
          [in3] "r" (v4)
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    /* Force mode mismatch: char in int operation */
    volatile char char_var = 65;
    int int_from_char = char_var * v5;  /* Mode conversion needed */
    
    /* Complex memory addressing - forces address reloads */
    long array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * i;
    }
    
    /* Complex array indexing with multiple variables */
    volatile long complex_addr = array[v6 + v7 * v8 - v9 / (v10 + 1)];
    
    /* Complex expression using most variables */
    volatile long complex_expr = 
        v1 + v2 * v3 - v4 / v5 + 
        v6 * v7 - v8 / v9 + 
        v10 + v11 * v12 - v13 / v14 +
        v15 * v16 - v17 / v18 +
        v19 + v20 * v21 - v22 / v23 +
        v24 * v25 - v26 / v27 +
        v28 + v29 * v30 - asm_result;
    
    /* Force conditional to prevent elimination */
    if (complex_expr > 1000) {
        /* Call helper with many arguments - forces register allocation */
        long helper_result = helper_func(v1, v2, v3, v4, v5, v6);
        
        /* More inline assembly with explicit constraints */
        long final_result;
        __asm__ volatile (
            "leaq (%[a], %[b], 4), %%rax\n\t"
            "addq %[c], %%rax\n\t"
            "subq %[d], %%rax\n\t"
            "movq %%rax, %[res]\n\t"
            : [res] "=r" (final_result)
            : [a] "r" (helper_result),
              [b] "r" (v7),
              [c] "r" (v8),
              [d] "r" (v9)
            : "rax", "rdx", "cc"
        );
        
        printf("Result: %ld\n", final_result);
    } else {
        printf("Alternative: %ld\n", complex_addr);
    }
    
    /* Use all variables to prevent dead code elimination */
    volatile long dummy = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
        v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    
    return (int)(dummy % 256);
}
