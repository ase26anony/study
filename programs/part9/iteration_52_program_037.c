/* reload_coverage.c
 * Designed to trigger GCC's reload pass initialization in reload.cc lines 1381-1399
 * Compile with: gcc -O2 -fomit-frame-pointer -march=x86-64 reload_coverage.c -o reload_test
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
    volatile long v4 = 4;
    register long v5 asm ("r13") = 5;  /* Another pinned register */
    long v6 = 6;
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
    
    /* Complex array indexing to force address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Inline assembly with fixed register constraints and clobbers */
    /* This forces specific register allocation and potential conflicts */
    asm volatile (
        "movq %[input], %%rax\n\t"      /* Force use of rax */
        "addq $100, %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=r" (v3)            /* Output constraint */
        : [input] "r" (v2),             /* Input constraint */
          "0" (v3)                      /* Matching constraint */
        : "rax", "rbx", "rcx", "rdx"    /* Clobber specific registers */
    );
    
    /* Another inline asm with mismatched modes */
    {
        char char_var = 65;
        int int_var;
        /* Mixing char and int modes */
        asm volatile (
            "movsbl %1, %0\n\t"
            : "=r" (int_var)
            : "r" (char_var)
            : "cc"
        );
        v4 = int_var;
    }
    
    /* Complex expression using most variables - creates register pressure */
    v1 = v2 + v3 * v4 - v5 / (v6 + 1) + v7 - v8 * v9 + v10;
    v11 = v12 + v13 - v14 * v15 + v16 / (v17 + 1) - v18 + v19 * v20;
    
    /* More complex expressions with array addressing */
    v1 += arr[v2 + v3 * 2 - v4 / 3] - arr[v5 + v6 - v7 * 3];
    v11 += arr[v8 * v9 + v10 - v12] + arr[v13 + v14 * v15 - v16];
    
    /* Force mode conversion operations */
    {
        float f1 = 3.14f;
        double d1 = 2.71828;
        /* Mixing float and double - may require reloads */
        double mixed = (double)f1 + d1;
        v1 += (long)mixed;
    }
    
    /* Call helper function with many arguments - forces register parameter passing */
    long result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Additional inline asm with output/input reloads */
    asm volatile (
        "imulq %[in1], %[in2]\n\t"
        "addq %%rax, %[in2]\n\t"
        : [in2] "+r" (v7)
        : [in1] "r" (v8),
          "a" (v9)        /* v9 in rax register */
        : "cc", "rdx"     /* imul clobbers rdx too */
    );
    
    /* Use volatile variable in condition to prevent elimination */
    if (v1 > v11) {
        result += arr[v1 % 100];
    } else {
        result -= arr[v11 % 100];
    }
    
    /* Final complex computation using all variables */
    result = v1 + v2 - v3 * v4 + v5 / (v6 + 1) 
             - v7 + v8 * v9 - v10 + v11 - v12 * v13 
             + v14 / (v15 + 1) - v16 + v17 * v18 - v19 + v20;
    
    printf("Result: %ld\n", result);
    return (int)(result % 100);
}
