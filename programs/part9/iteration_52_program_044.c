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
    register long v1 asm ("r12") = rand() % 100 + 1;
    register long v2 asm ("r13") = rand() % 100 + 1;
    volatile long v3 = rand() % 100 + 1;
    volatile long v4 = rand() % 100 + 1;
    long v5 = rand() % 100 + 1;
    long v6 = rand() % 100 + 1;
    long v7 = rand() % 100 + 1;
    long v8 = rand() % 100 + 1;
    long v9 = rand() % 100 + 1;
    long v10 = rand() % 100 + 1;
    long v11 = rand() % 100 + 1;
    long v12 = rand() % 100 + 1;
    long v13 = rand() % 100 + 1;
    long v14 = rand() % 100 + 1;
    long v15 = rand() % 100 + 1;
    long v16 = rand() % 100 + 1;
    long v17 = rand() % 100 + 1;
    long v18 = rand() % 100 + 1;
    long v19 = rand() % 100 + 1;
    long v20 = rand() % 100 + 1;
    
    /* Complex array indexing with multiple variables - forces address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex memory addressing that may require reloads */
    long idx = v5 + v6 * v7 - v8 / (v9 + 1);
    if (idx < 0) idx = -idx;
    idx = idx % 100;
    
    /* Use mismatched modes - char in int operation */
    unsigned char c1 = v10 & 0xFF;
    unsigned char c2 = v11 & 0xFF;
    long mixed_mode = (long)c1 * (long)c2 + v12;
    
    /* Inline assembly with fixed register constraints and clobbers */
    long asm_result;
    asm volatile (
        /* Force specific register usage and create conflicts */
        "movq %[input1], %%rax\n\t"
        "addq %[input2], %%rax\n\t"
        "imulq %[input3], %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=r" (asm_result)
        : [input1] "r" (v13),
          [input2] "r" (v14),
          [input3] "r" (v15)
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    /* Another inline asm with explicit output constraint */
    long asm_result2;
    asm volatile (
        "movl $1, %%eax\n\t"
        "cpuid\n\t"
        : "=a"(asm_result2)
        :: "rbx", "rcx", "rdx", "memory"
    );
    
    /* Complex expression using most variables - maximizes register pressure */
    long complex_expr = 
        v1 + v2 * v3 - v4 / (v5 + 1) +
        v6 * v7 - v8 + v9 * v10 +
        v11 - v12 * v13 + v14 / (v15 + 1) +
        v16 * v17 - v18 + v19 * v20 +
        asm_result + asm_result2 + mixed_mode;
    
    /* Force conditional with volatile to prevent elimination */
    volatile int condition = complex_expr > 1000;
    if (condition) {
        complex_expr = complex_expr / 2;
    }
    
    /* Complex array access with multiple index calculations */
    long array_val = arr[(v1 + v2 * v3 - v4) % 100] +
                     arr[(v5 + v6) % 100] +
                     arr[(v7 * v8) % 100] +
                     arr[idx];
    
    /* Function call with many arguments - forces register parameter passing */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Use struct with nested addressing */
    struct nested {
        long a;
        struct {
            long x;
            long y;
        } inner;
        long b;
    };
    
    struct nested nst;
    nst.a = v7;
    nst.inner.x = v8;
    nst.inner.y = v9;
    nst.b = v10;
    
    /* Take address of nested member - may require address reload */
    long *nested_ptr = &nst.inner.x;
    *nested_ptr = *nested_ptr + v11;
    
    /* Final computation using all values */
    long final_result = 
        complex_expr + 
        array_val + 
        func_result + 
        nst.a + nst.inner.x + nst.inner.y + nst.b +
        arr[0] + arr[99];
    
    /* Prevent dead code elimination */
    volatile long output = final_result;
    
    printf("Result: %ld\n", output);
    
    return 0;
}
