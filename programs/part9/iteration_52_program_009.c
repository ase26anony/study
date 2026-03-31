/* reload_trigger.c - Program to trigger GCC reload pass initialization */
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
    volatile long v1 = 1;
    register long v2 asm ("r12") = 2;  /* Pin to specific register */
    long v3 = 3;
    volatile long v4 = 4;
    long v5 = 5;
    register long v6 asm ("r13") = 6;  /* Another pinned register */
    long v7 = 7;
    volatile long v8 = 8;
    long v9 = 9;
    long v10 = 10;
    long v11 = 11;
    volatile long v12 = 12;
    long v13 = 13;
    long v14 = 14;
    long v15 = 15;
    volatile long v16 = 16;
    long v17 = 17;
    long v18 = 18;
    long v19 = 19;
    long v20 = 20;
    
    /* Complex addressing mode - forces address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Inline assembly with fixed register constraints and clobbers */
    /* This forces specific register allocation and creates conflicts */
    asm volatile (
        "movq %[input], %%rax\n\t"      /* Force use of rax */
        "addq $100, %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=r" (v1)            /* Output constraint */
        : [input] "r" (v2)              /* Input constraint */
        : "rax", "rbx", "rcx", "rdx"    /* Clobber specific registers */
    );
    
    /* Another inline asm with mismatched constraints */
    long temp;
    asm volatile (
        "movl %1, %%eax\n\t"           /* 32-bit operation */
        "addl $1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (temp)                  /* 64-bit output */
        : "r" (v3)                     /* 64-bit input */
        : "eax"
    );
    
    /* Complex expression using many variables - creates register pressure */
    v4 = v5 + v6 * v7 - v8 / (v9 + 1) + v10;
    v11 = v12 * v13 - v14 + v15 / (v16 + 1) * v17;
    v18 = v19 + v20 * v1 - v2 / (v3 + 1) + v4;
    
    /* Array access with complex addressing - forces address reload */
    long idx = v5 + v6 * v7 - v8;
    volatile long array_val = arr[idx % 100] + arr[(idx + v9) % 100];
    
    /* Mixed mode operations - char to int conversion */
    char c1 = 65;
    char c2 = 66;
    int mixed_result = c1 * c2 + v10;  /* char promoted to int */
    
    /* Force function call with many parameters - uses calling convention regs */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* More complex arithmetic chain */
    for (int i = 0; i < 10; i++) {
        v1 = v1 + v2 * v3 - v4 / (v5 + i) + v6;
        v2 = v2 - v3 + v4 * v5 / (v6 + i + 1);
        v3 = v3 * v4 - v5 + v6 / (v7 + i + 2);
    }
    
    /* Use volatile to prevent optimization */
    volatile long final_result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                                 temp + mixed_result + func_result + array_val;
    
    printf("Result: %ld\n", final_result);
    return (int)(final_result % 256);
}
