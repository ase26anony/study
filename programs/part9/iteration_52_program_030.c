/* reload_test.c - Program to trigger GCC reload pass initialization */
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
    volatile long v4 = 4;
    long v5 = 5, v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    long v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    long v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    long v21 = 21, v22 = 22, v23 = 23, v24 = 24, v25 = 25;
    
    /* Complex expression using many variables - creates register pressure */
    long complex_result = 
        v1 + v2 * v3 - v4 / (v5 + 1) + 
        v6 * v7 - v8 + v9 / (v10 + 1) +
        v11 - v12 * v13 + v14 / (v15 + 1) -
        v16 + v17 * v18 - v19 / (v20 + 1) +
        v21 * v22 - v23 + v24 / (v25 + 1);
    
    /* Inline assembly with fixed register constraints - forces reloads */
    asm volatile (
        /* Clobber multiple specific registers */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (v1)          /* Output operand */
        : [in1] "r" (v2),          /* Input operand 1 */
          [in2] "r" (complex_result) /* Input operand 2 */
        : "rax", "rbx", "rcx", "rdx", "memory"  /* Clobbered registers */
    );
    
    /* Another inline asm with mismatched constraints */
    long temp;
    asm volatile (
        "movl %1, %0\n\t"
        : "=r" (temp)              /* Output in register */
        : "m" (v3)                 /* Input from memory - may need reload */
        : "cc"
    );
    
    /* Force address reloads with complex memory addressing */
    long arr[100];
    for (int i = 0; i < 25; i++) {
        /* Complex index computation */
        arr[v1 + v2 * i - v3 / (i + 1) + v4] = i * 2;
    }
    
    /* Use mismatched types to force mode conversions */
    char c1 = 65;
    short s1 = 1000;
    int i1 = 100000;
    double d1 = 3.14159;
    float f1 = 2.71828;
    
    /* Operations requiring mode conversions */
    i1 = i1 + c1;          /* char to int promotion */
    d1 = d1 + f1;          /* float to double promotion */
    s1 = s1 * c1;          /* char to int, then back to short */
    
    /* Call helper function - forces parameter passing in registers */
    long call_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* More complex expressions with volatile to prevent optimization */
    volatile long final_result = 
        complex_result + call_result + temp + 
        arr[10] + i1 + (long)d1 + s1;
    
    /* Use the result to prevent dead code elimination */
    if (final_result > 0) {
        printf("Result: %ld\n", final_result);
    } else {
        printf("Alternative: %ld\n", -final_result);
    }
    
    return 0;
}
