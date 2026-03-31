/* reload_coverage.c
 * Compile with: gcc -O2 -fomit-frame-pointer -march=x86-64 reload_coverage.c -o reload_test
 * For detailed reload analysis: gcc -O2 -fomit-frame-pointer -fdump-rtl-reload -march=x86-64 reload_coverage.c 2>&1
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static int64_t use_registers(int64_t a, int64_t b, int64_t c, int64_t d, 
                             int64_t e, int64_t f, int64_t g, int64_t h) {
    /* Complex expression to create register pressure */
    return a + b * c - d / (e + 1) + (f << 2) - (g >> 3) + h * 7;
}

/* Another helper with mixed types to force mode conversions */
__attribute__((noinline))
static double mixed_mode_op(int32_t a, int64_t b, float c, double d) {
    return (double)a + (double)b + (double)c + d;
}

int main(void) {
    /* Create massive register pressure with many live variables */
    /* Use volatile to prevent optimization removal */
    volatile int64_t v1 = 1;
    int64_t v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int64_t v9 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14;
    int64_t v15 = 15, v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    /* Additional variables with different types for mode mismatches */
    int32_t i32_1 = 100, i32_2 = 200;
    float f1 = 3.14f, f2 = 2.71f;
    double d1 = 1.414, d2 = 1.732;
    
    /* Use explicit register variables to pin values */
    register int64_t r12_var asm ("r12") = 0x12345678;
    register int64_t r13_var asm ("r13") = 0x87654321;
    
    /* Complex array indexing to force address reloads */
    int64_t arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Inline assembly with fixed register constraints */
    /* This forces specific register allocation and creates conflicts */
    asm volatile (
        /* Clobber multiple registers to force saves/restores */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (v1)          /* Output in register */
        : [in1] "r" (v2),          /* Input in register */
          [in2] "r" (v3)
        : "rax", "rbx", "rcx", "rdx"  /* Clobber specific registers */
    );
    
    /* Another asm with different constraints */
    int64_t asm_result;
    asm volatile (
        "imulq %[a], %[b]\n\t"
        "movq %[b], %[res]\n\t"
        : [res] "=&r" (asm_result)  /* Early clobber to force separate reg */
        : [a] "r" (v4),
          [b] "r" (v5)
        : "cc"                      /* Clobber condition codes */
    );
    
    /* Complex expression using many variables - creates register pressure */
    int64_t complex_result = 
        v1 + v2 * v3 - v4 / (v5 + 1) + 
        (v6 << (v7 % 4)) - (v8 >> (v9 & 3)) + 
        v10 * v11 - v12 / (v13 | 1) +
        (v14 & v15) | (v16 ^ v17) +
        v18 * 3 - v19 / 2 + v20;
    
    /* Force mode conversions */
    double mixed_result = mixed_mode_op(i32_1, v1, f1, d1);
    
    /* Complex array access with multiple index variables */
    int64_t array_access = 
        arr[v1 % 50 + v2 % 30] +
        arr[v3 % 40 * v4 % 60] +
        arr[(v5 + v6 * v7) % 80];
    
    /* Use register-pinned variables in computation */
    r12_var = r12_var + v1 + v2;
    r13_var = r13_var * v3 - v4;
    
    /* Call function that uses many parameters - forces register passing */
    int64_t func_result = use_registers(
        v1, v2, v3, v4, v5, v6, v7, v8
    );
    
    /* Use volatile variable in condition to prevent dead code elimination */
    if (v1 > 0) {
        complex_result += func_result + array_access + asm_result;
        mixed_result += (double)complex_result;
    }
    
    /* Final computation using all results */
    int64_t final_result = 
        complex_result + 
        (int64_t)mixed_result + 
        func_result + 
        array_access + 
        asm_result +
        r12_var + r13_var;
    
    printf("Result: %ld\n", final_result);
    return (int)(final_result % 256);
}
