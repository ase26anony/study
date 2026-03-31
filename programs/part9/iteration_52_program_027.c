/* reload_test.c - Test program to trigger GCC reload pass initialization */
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
    volatile long v1 = rand() % 100;
    register long v2 asm ("r12") = v1 + 1;  /* Pin to specific register */
    long v3 = v2 * 2;
    long v4 = v3 - 5;
    long v5 = v4 / 3;
    long v6 = v5 * 7;
    long v7 = v6 + 11;
    long v8 = v7 - 13;
    long v9 = v8 * 17;
    long v10 = v9 / 19;
    long v11 = v10 + 23;
    long v12 = v11 - 29;
    long v13 = v12 * 31;
    long v14 = v13 / 37;
    long v15 = v14 + 41;
    long v16 = v15 - 43;
    long v17 = v16 * 47;
    long v18 = v17 / 53;
    long v19 = v18 + 59;
    long v20 = v19 - 61;
    
    /* Force mode mismatches */
    char c1 = v1 & 0xFF;
    short s1 = v2 & 0xFFFF;
    int i1 = v3;
    float f1 = v4 * 0.5f;
    double d1 = v5 * 0.25;
    
    /* Complex memory addressing - forces address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex array indexing with multiple variables */
    long idx = arr[(v1 + v2 * v3 - v4 / (v5 + 1)) % 100];
    
    /* Inline assembly with fixed register constraints and clobbers */
    long asm_result;
    asm volatile (
        /* Force specific register usage and clobbers */
        "movq %[input1], %%rax\n\t"
        "addq %[input2], %%rax\n\t"
        "imulq %[input3], %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=r" (asm_result)
        : [input1] "r" (v6), 
          [input2] "r" (v7), 
          [input3] "r" (v8)
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    /* Another inline asm with different constraints */
    long asm_result2;
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (asm_result2)
        : "r" ((int)v9), "r" ((int)v10)
        : "eax", "ebx", "ecx"
    );
    
    /* Complex expression using most variables - maximizes live ranges */
    long complex_expr = 
        v1 + v2 * v3 - v4 / (v5 + 1) +
        v6 * v7 - v8 / (v9 + 1) +
        v10 + v11 * v12 - v13 / (v14 + 1) +
        v15 * v16 - v17 / (v18 + 1) +
        v19 + v20 * idx - asm_result / (asm_result2 + 1);
    
    /* Force conditional with volatile to prevent elimination */
    volatile int condition = complex_expr > 1000;
    if (condition) {
        /* Use mismatched types in operation */
        complex_expr += (long)f1 + (long)d1 + c1 + s1 + i1;
    }
    
    /* Force function call with many parameters - uses register ABI */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* More register pressure after call */
    long v21 = func_result * 2;
    long v22 = v21 + complex_expr;
    long v23 = v22 - asm_result;
    long v24 = v23 * asm_result2;
    
    /* Final complex memory access with addressing mode that may need reload */
    struct nested {
        long a;
        struct {
            long x;
            long y;
            long arr[10];
        } inner;
        long b;
    } nested_struct;
    
    /* Take address of nested member - may require address reload */
    long *nested_ptr = &nested_struct.inner.arr[(v1 + v2) % 10];
    *nested_ptr = v24;
    
    /* Use all variables in final computation to prevent optimization */
    long final_result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
        complex_expr + asm_result + asm_result2 + func_result +
        v21 + v22 + v23 + v24 + *nested_ptr + idx;
    
    printf("Result: %ld\n", final_result);
    return (final_result > 0) ? 0 : 1;
}
