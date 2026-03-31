/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f) {
    volatile long result = a + b - c * d / e + f;
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
    
    /* Complex array indexing to force address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Inline assembly with fixed register constraints */
    /* This forces specific register allocation and creates conflicts */
    asm volatile (
        "movq %[input], %%rax\n\t"
        "addq %%rbx, %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=r" (v5)
        : [input] "r" (v6), "0" (v5)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    /* Another inline asm with mismatched constraints */
    long temp;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (temp)
        : "r" (v7)
        : "%eax", "cc"
    );
    
    /* Complex expression using most variables - creates register pressure */
    v1 = v2 + v3 * v4 - v5 / (v6 + 1) + v7 % v8;
    v9 = v10 * v11 - v12 + v13 / (v14 | 1) * v15;
    v16 = (v17 + v18) * (v19 - v20) / (v21 + 1);
    v22 = v23 ^ v24 | v25 & v1;
    
    /* More complex arithmetic with mixed types */
    int iv1 = v1;
    short sv1 = v2;
    char cv1 = v3;
    
    /* Mode mismatches - using different sized types together */
    v4 = iv1 + sv1 * cv1 - (long)sv1 / (iv1 + 1);
    
    /* Complex array access with multiple index variables */
    volatile long array_val = arr[v1 + v2 * v3 - v4 / (v5 + 1)];
    
    /* Nested struct with address taken */
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
    
    /* Force address reloads with complex addressing */
    long *ptr1 = &outer_var.in1.c[v1 & 3];
    long *ptr2 = &outer_var.in2.c[v2 % 5];
    *ptr1 = v10;
    *ptr2 = v11;
    
    /* Use volatile in condition to prevent optimization */
    if (v3 > 0) {
        v12 = helper_func(v1, v2, v3, v4, v5, v6);
        v13 = helper_func(v7, v8, v9, v10, v11, v12);
    }
    
    /* Final complex computation using all variables */
    long final_result = 
        v1 + v2 - v3 * v4 / (v5 + 1) +
        v6 % v7 + v8 * v9 - v10 +
        v11 / (v12 | 1) * v13 -
        v14 + v15 * v16 / (v17 + 1) +
        v18 % v19 + v20 * v21 -
        v22 + v23 / (v24 | 1) * v25 +
        array_val + *ptr1 - *ptr2 +
        iv1 + sv1 - cv1;
    
    /* Prevent dead code elimination */
    volatile long sink = final_result;
    
    printf("Result: %ld\n", final_result);
    return (int)(final_result % 256);
}
