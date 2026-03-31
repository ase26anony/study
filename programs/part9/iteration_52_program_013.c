/* reload_test.c - Test program to trigger GCC's reload pass initialization */
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
    volatile long v1 = rand() % 100 + 1;
    volatile long v2 = rand() % 100 + 1;
    volatile long v3 = rand() % 100 + 1;
    volatile long v4 = rand() % 100 + 1;
    volatile long v5 = rand() % 100 + 1;
    volatile long v6 = rand() % 100 + 1;
    volatile long v7 = rand() % 100 + 1;
    volatile long v8 = rand() % 100 + 1;
    volatile long v9 = rand() % 100 + 1;
    volatile long v10 = rand() % 100 + 1;
    volatile long v11 = rand() % 100 + 1;
    volatile long v12 = rand() % 100 + 1;
    volatile long v13 = rand() % 100 + 1;
    volatile long v14 = rand() % 100 + 1;
    volatile long v15 = rand() % 100 + 1;
    volatile long v16 = rand() % 100 + 1;
    volatile long v17 = rand() % 100 + 1;
    volatile long v18 = rand() % 100 + 1;
    volatile long v19 = rand() % 100 + 1;
    volatile long v20 = rand() % 100 + 1;
    
    /* Use explicit register variables to pin values */
    register long r12_var asm("r12") = v1 + v2;
    register long r13_var asm("r13") = v3 * v4;
    
    /* Complex expression using most variables - creates register pressure */
    long complex_expr = 
        v1 + v2 * v3 - v4 / (v5 + 1) +
        v6 * v7 + v8 - v9 * v10 +
        v11 + v12 - v13 * v14 +
        v15 / (v16 + 1) + v17 - v18 * v19 +
        v20 + r12_var - r13_var;
    
    /* Inline assembly with fixed register constraints and clobbers */
    long asm_result;
    asm volatile (
        /* Force register allocation conflicts */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (asm_result)
        : [in1] "r" (complex_expr), [in2] "r" (v1)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    /* Additional inline assembly with mismatched constraints */
    long asm_result2;
    asm volatile (
        "imulq %[in1], %[out]\n\t"
        : [out] "=r" (asm_result2)
        : [in1] "r" (v2), "0" (asm_result)  /* Input-output operand */
        : "cc"
    );
    
    /* Force address reloads with complex array indexing */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex addressing mode that may require reloads */
    long idx = v3 + v4 * v5 - v6;
    if (idx < 0) idx = -idx;
    idx = idx % 100;
    
    volatile long array_access = arr[idx + v7 * v8 - v9];
    
    /* Nested structure with address taking */
    struct inner {
        long a;
        long b;
        long c[5];
    };
    
    struct outer {
        struct inner inner1;
        struct inner inner2;
        long extra;
    };
    
    struct outer outer_var;
    outer_var.inner1.a = v10;
    outer_var.inner1.b = v11;
    outer_var.inner2.a = v12;
    outer_var.inner2.b = v13;
    outer_var.extra = v14;
    
    /* Take address of nested member - may require address reload */
    long* nested_ptr = &outer_var.inner2.c[2];
    *nested_ptr = v15 + v16;
    
    /* Call helper function with many arguments - forces register parameter passing */
    long func_result = helper_func(
        v17, v18, v19, v20,
        asm_result, asm_result2
    );
    
    /* Use volatile condition to prevent dead code elimination */
    volatile int condition = (func_result > 1000);
    if (condition) {
        array_access += func_result;
    }
    
    /* Final computation using all variables to prevent optimization */
    long final_result = 
        complex_expr + asm_result + asm_result2 + 
        array_access + func_result + *nested_ptr;
    
    printf("Result: %ld\n", final_result);
    return (int)(final_result % 256);
}
