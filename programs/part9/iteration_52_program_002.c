/* reload_trigger.c
 * Designed to trigger GCC's reload pass initialization code
 * Compile with: gcc -O2 -fomit-frame-pointer -march=x86-64 reload_trigger.c -o reload_trigger
 */

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
    long v4 = 4;
    long v5 = 5;
    long v6 = 6;
    long v7 = 7;
    long v8 = 8;
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
    
    /* Force mode mismatches */
    char c1 = 100;
    short s1 = 2000;
    int i1 = 30000;
    float f1 = 3.14f;
    double d1 = 2.71828;
    
    /* Complex expression using many variables - creates register pressure */
    long complex_expr = 
        v1 + v2 * v3 - v4 / (v5 + 1) +
        v6 * v7 - v8 / (v9 + 1) +
        v10 + v11 * v12 - v13 / (v14 + 1) +
        v15 * v16 - v17 / (v18 + 1) +
        v19 + v20;
    
    /* Inline assembly with fixed register constraints and clobbers */
    /* This forces specific register allocation and potential reloads */
    asm volatile (
        "movq %[in1], %%rax\n\t"        /* Input to fixed register */
        "addq %[in2], %%rax\n\t"        /* Another input */
        "movq %%rax, %[out]\n\t"        /* Output */
        : [out] "=r" (v1)               /* Output operand */
        : [in1] "r" (v2),               /* Input operand 1 */
          [in2] "r" (v3)                /* Input operand 2 */
        : "rax", "rbx", "rcx", "rdx"    /* Clobber specific registers */
    );
    
    /* More inline assembly with mismatched constraints */
    long asm_out;
    asm volatile (
        "movl %1, %0\n\t"               /* 32-bit move */
        "addl $1, %0\n\t"
        : "=r" (asm_out)                /* Output constraint */
        : "r" (i1)                      /* Input constraint */
        : "cc"                          /* Clobber flags */
    );
    
    /* Force address reloads with complex memory addressing */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Complex array indexing - may require address reloads */
    long idx = v1 + v2 * v3 - v4;
    if (idx < 0) idx = -idx;
    idx = idx % 100;
    
    volatile long array_access = arr[idx + v5 * v6 - v7 / (v8 + 1)];
    
    /* Mode conversion operations */
    double mixed_calc = f1 + d1 + c1 + s1 + i1;
    
    /* Call helper function - forces parameter passing in registers */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Use volatile to prevent optimization */
    volatile long final_result = 
        complex_expr + 
        asm_out + 
        array_access + 
        (long)mixed_calc + 
        func_result;
    
    printf("Result: %ld\n", final_result);
    
    /* Additional stress test with more inline assembly */
    {
        long a = v10, b = v11, c = v12, d = v13;
        asm volatile (
            "imulq %[b], %[a]\n\t"
            "addq %[c], %[a]\n\t"
            "subq %[d], %[a]\n\t"
            : [a] "+r" (a)
            : [b] "r" (b), [c] "r" (c), [d] "r" (d)
            : "cc", "memory"
        );
        final_result += a;
    }
    
    /* Create more register pressure with pointer arithmetic */
    long *ptr1 = &v1;
    long *ptr2 = &v2;
    long *ptr3 = &v3;
    
    volatile long ptr_calc = 
        (*ptr1) * (*ptr2) + (*ptr3) - (ptr2 - ptr1) * 8;
    
    /* Final complex expression using all variables */
    v20 = 
        v1 + v2 - v3 * v4 / (v5 + 1) +
        v6 - v7 * v8 / (v9 + 1) +
        v10 + v11 - v12 * v13 / (v14 + 1) +
        v15 - v16 * v17 / (v18 + 1) +
        v19 + asm_out + (long)ptr_calc;
    
    printf("Final v20: %ld\n", v20);
    
    return 0;
}
