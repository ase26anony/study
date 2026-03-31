/* reload_trigger.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

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
    volatile long v11 = 11;
    long v12 = 12;
    long v13 = 13;
    volatile long v14 = 14;
    long v15 = 15;
    long v16 = 16;
    volatile long v17 = 17;
    long v18 = 18;
    long v19 = 19;
    volatile long v20 = 20;
    
    /* Complex addressing mode - forces address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) arr[i] = i;
    
    /* Use complex array indexing with multiple variables */
    long idx = arr[v2 + v3 * v4 - v5 / (v6 + 1)];
    
    /* Inline assembly with fixed register constraints */
    /* This forces specific register allocation and creates conflicts */
    asm volatile (
        "movq %[input], %%rax\n\t"      /* Force use of rax */
        "addq $100, %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=r" (v1)            /* Output constraint */
        : [input] "r" (v2)              /* Input constraint */
        : "%rax", "%rbx", "%rcx"        /* Clobber specific registers */
    );
    
    /* Another inline asm with mismatched constraints */
    long temp;
    asm volatile (
        "movl %1, %0\n\t"
        : "=r" (temp)                   /* General purpose reg */
        : "m" (v3)                      /* Memory operand - may need reload */
        : "%eax"
    );
    
    /* Complex expression using most variables - maximizes register pressure */
    v4 = v5 + v6 * v7 - v8 / (v9 + 1) + 
         v10 * v11 - v12 / (v13 + 1) + 
         v14 + v15 * v16 - v17 / (v18 + 1) + 
         v19 + v20;
    
    /* Mode mixing - int and long operations */
    int small1 = 1000;
    int small2 = 2000;
    long large1 = 3000000000L;
    
    /* This may require mode conversions */
    v5 = small1 * small2 + large1 / small1;
    
    /* Force address computation reload with nested struct */
    struct inner {
        long a;
        long b;
        long c[5];
    };
    
    struct outer {
        struct inner in1;
        struct inner in2;
        long extra;
    } outer_var;
    
    /* Complex address computation */
    long* ptr = &outer_var.in1.c[v2 + v3];
    
    /* Use volatile to prevent optimization */
    volatile int condition = (v1 + v2 + v3) > 100;
    if (condition) {
        /* More complex operations in conditional path */
        v6 = v7 * v8 - v9 / (v10 + 1) + v11 * v12;
    }
    
    /* Function call with many arguments - forces register parameter passing */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Use all variables in final computation to prevent dead code elimination */
    long final_result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                       v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                       idx + temp + func_result + *ptr;
    
    printf("Result: %ld\n", final_result);
    return (int)(final_result % 100);
}
