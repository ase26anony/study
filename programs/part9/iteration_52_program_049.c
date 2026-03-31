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
    /* Use volatile to prevent optimization removal */
    volatile long v1 = rand() % 100;
    volatile long v2 = rand() % 100;
    volatile long v3 = rand() % 100;
    volatile long v4 = rand() % 100;
    volatile long v5 = rand() % 100;
    volatile long v6 = rand() % 100;
    volatile long v7 = rand() % 100;
    volatile long v8 = rand() % 100;
    volatile long v9 = rand() % 100;
    volatile long v10 = rand() % 100;
    volatile long v11 = rand() % 100;
    volatile long v12 = rand() % 100;
    volatile long v13 = rand() % 100;
    volatile long v14 = rand() % 100;
    volatile long v15 = rand() % 100;
    volatile long v16 = rand() % 100;
    volatile long v17 = rand() % 100;
    volatile long v18 = rand() % 100;
    volatile long v19 = rand() % 100;
    volatile long v20 = rand() % 100;
    
    /* Force specific registers with explicit register variables */
    register long r12_var asm ("r12") = v1 + v2;
    register long r13_var asm ("r13") = v3 * v4;
    register long r14_var asm ("r14") = v5 - v6;
    register long r15_var asm ("r15") = v7 / (v8 + 1);
    
    /* Complex expression using most variables - creates register pressure */
    long complex_expr = 
        v1 + v2 * v3 - v4 / (v5 + 1) +
        v6 * v7 - v8 + v9 * v10 -
        v11 / (v12 + 1) + v13 - v14 * v15 +
        v16 / (v17 + 1) - v18 + v19 * v20 +
        r12_var - r13_var + r14_var * r15_var;
    
    /* Inline assembly with fixed register constraints and clobbers */
    /* This forces the compiler to work around specific registers */
    long asm_result;
    asm volatile (
        /* Perform some dummy operations */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "imulq %[in3], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (asm_result)
        : [in1] "r" (v1),
          [in2] "r" (v2),
          [in3] "r" (v3)
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    /* More inline assembly with different constraints */
    long asm_result2;
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (asm_result2)
        : "r" ((int)v4), "r" ((int)v5)
        : "eax", "ebx", "ecx"
    );
    
    /* Force address reloads with complex memory addressing */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex array indexing that may require address reloads */
    long idx1 = v6 % 50;
    long idx2 = v7 % 30;
    long idx3 = v8 % 20;
    
    volatile long complex_addr = arr[idx1 + idx2 * idx3 - (v9 % 10)];
    
    /* Mix different data types to force mode conversions */
    char c1 = v10 & 0xFF;
    short s1 = v11 & 0xFFFF;
    int i1 = v12;
    long l1 = v13;
    
    /* Operations requiring mode conversions */
    volatile long mixed_ops = c1 + s1 * i1 - l1 / (c1 + 1);
    
    /* Use floating point to create different register class pressure */
    double d1 = v14 * 1.5;
    double d2 = v15 * 2.5;
    volatile double fp_result = d1 * d2 - d1 / (d2 + 1.0);
    
    /* Call helper function - forces parameter passing in registers */
    long func_result = helper_func(v16, v17, v18, v19, v20, v1);
    
    /* Use volatile variable in condition to prevent dead code elimination */
    volatile int condition = (complex_expr > 1000);
    
    if (condition) {
        /* Another complex expression block */
        long final_result = 
            asm_result + asm_result2 + complex_addr + mixed_ops + 
            (long)fp_result + func_result + complex_expr;
        
        /* Force output to prevent optimization */
        printf("Final result: %ld\n", final_result);
        
        /* More inline assembly with output constraints */
        long final_asm;
        asm volatile (
            "leaq (%[a], %[b], 4), %[out]\n\t"
            : [out] "=r" (final_asm)
            : [a] "r" (final_result),
              [b] "r" (v2)
            : "cc"
        );
        
        return (int)(final_asm % 256);
    }
    
    return 0;
}
