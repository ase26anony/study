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

/* Another helper with mismatched types */
__attribute__((noinline))
static double mixed_helper(int a, double b, long c, float d) {
    return (double)a + b * (double)c + (double)d;
}

int main(void) {
    /* Create register pressure with many live variables */
    volatile long v1 = 1;
    register long v2 asm ("r12") = 2;  /* Pin to specific register */
    long v3 = 3;
    volatile long v4 = 4;
    long v5 = 5;
    register int v6 asm ("r13") = 6;   /* Another pinned register */
    long v7 = 7;
    volatile long v8 = 8;
    long v9 = 9;
    long v10 = 10;
    long v11 = 11;
    long v12 = 12;
    long v13 = 13;
    long v14 = 14;
    long v15 = 15;
    long v16 = 16;
    long v17 = 17;
    long v18 = 18;
    long v19 = 19;
    long v20 = 20;
    
    /* Complex array indexing to force address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex expression using many variables - creates register pressure */
    long complex_result = 
        v1 + v2 * v3 - v4 / (v5 + 1) +
        v6 * v7 + v8 - v9 * v10 +
        v11 + v12 * v13 - v14 / (v15 + 1) +
        v16 * v17 + v18 - v19 * v20;
    
    /* Inline assembly with fixed register constraints and clobbers */
    /* This forces specific register allocation and potential reloads */
    asm volatile (
        "movq %[in], %%rax\n\t"        /* Input to rax */
        "addq %%r12, %%rax\n\t"        /* Use pinned register r12 */
        "movq %%rax, %[out]\n\t"       /* Output from rax */
        : [out] "=r" (v3)              /* Output operand */
        : [in] "r" (v1), "r" (v2)      /* Input operands */
        : "rax", "rbx", "rcx", "rdx",   /* Clobber specific registers */
          "rsi", "rdi", "r8", "r9",
          "r10", "r11", "cc", "memory"
    );
    
    /* More inline assembly with mismatched constraints */
    long temp1, temp2;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (temp1)                /* Output constraint */
        : "r" (v6), "r" (v7)          /* Input constraints */
        : "%eax", "%ebx"              /* Clobber registers */
    );
    
    /* Force mode mismatches */
    char char_var = 100;
    int int_var = char_var * 2;       /* Mode conversion needed */
    double double_var = int_var * 3.14159;
    float float_var = double_var / 2.0f;
    
    /* Complex addressing mode */
    long idx = v1 + v2 * v3 - v4;
    if (idx < 0) idx = -idx;
    idx = idx % 100;
    
    /* This complex addressing may require address reloads */
    volatile long array_access = arr[idx + v5 * v6 - v7 / (v8 + 1)];
    
    /* Mixed type operations */
    double mixed_result = mixed_helper(v6, double_var, v2, float_var);
    
    /* Function call with many parameters - forces register passing */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Another complex expression preventing optimization */
    volatile long final_result = 
        complex_result + 
        array_access * 2 - 
        func_result / (mixed_result + 1) +
        temp1 * v9 - v10;
    
    /* Use all variables in a way that prevents dead code elimination */
    if (final_result > 1000) {
        printf("Result: %ld\n", final_result);
    } else {
        printf("Alternative: %ld\n", 
               v11 + v12 + v13 + v14 + v15 + 
               v16 + v17 + v18 + v19 + v20);
    }
    
    /* Additional register pressure with pointer arithmetic */
    long *ptr1 = &v1;
    long *ptr2 = &v2;
    long *ptr3 = &v3;
    
    /* Complex pointer expression */
    volatile long ptr_diff = (ptr3 - ptr1) * (ptr2 - ptr1);
    
    /* Nested struct with complex addressing */
    struct nested {
        long a;
        struct {
            long x;
            long y;
            long z[5];
        } inner;
        long b;
    } nested_var;
    
    /* Force address computation reloads */
    long *nested_ptr = &nested_var.inner.z[2];
    *nested_ptr = final_result;
    
    /* More inline asm with output/input mismatches */
    asm volatile (
        "movq %1, %%r10\n\t"
        "imulq %2, %%r10\n\t"
        "movq %%r10, %0\n\t"
        : "=rm" (v20)      /* Allow memory or register */
        : "r" (v19), "r" (v18)
        : "%r10", "%r11", "memory"
    );
    
    return (int)(final_result % 1000);
}
