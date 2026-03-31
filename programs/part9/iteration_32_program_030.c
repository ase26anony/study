#include <stdio.h>
#include <stdint.h>

/* Define vector types with exactly 10 and 11 elements */
typedef int v10si __attribute__((vector_size(10 * sizeof(int))));
typedef int v11si __attribute__((vector_size(11 * sizeof(int))));

/* 11-argument inline function that must be expanded */
static inline int64_t __attribute__((always_inline))
multi_operand_op(int a, int b, int c, int d, int e,
                 int f, int g, int h, int i, int j, int k) {
    /* Complex expression that uses all 11 operands */
    return ((((((((((int64_t)a + b) * c) - d) << (e & 3)) & f) | 
              g) ^ h) + i) * j) / (k ? k : 1);
}

/* 10-argument inline function */
static inline int __attribute__((always_inline))
ten_operand_op(int a, int b, int c, int d, int e,
               int f, int g, int h, int i, int j) {
    return a + b - c + d - e + f - g + h - i + j;
}

int main(void) {
    /* Declare 11 volatile variables with distinct prime values */
    volatile int v0 = 2;
    volatile int v1 = 3;
    volatile int v2 = 5;
    volatile int v3 = 7;
    volatile int v4 = 11;
    volatile int v5 = 13;
    volatile int v6 = 17;
    volatile int v7 = 19;
    volatile int v8 = 23;
    volatile int v9 = 29;
    volatile int v10 = 31;
    
    /* Mixed type variables to trigger different expansion paths */
    volatile char c1 = 127;
    volatile short s1 = 32767;
    volatile long l1 = 65537L;
    volatile float f1 = 3.14159f;
    volatile double d1 = 2.71828;
    
    /* Results accumulators */
    int64_t asm_results = 0;
    int64_t expr_results = 0;
    int64_t vector_results = 0;
    int64_t func_results = 0;
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int asm_out;
        __asm__ volatile (
            "/* Custom 10-operand operation */\n\t"
            "add %0, %1, %2\n\t"
            "add %0, %0, %3\n\t"
            "add %0, %0, %4\n\t"
            "add %0, %0, %5\n\t"
            "add %0, %0, %6\n\t"
            "add %0, %0, %7\n\t"
            "add %0, %0, %8\n\t"
            "add %0, %0, %9\n\t"
            "add %0, %0, %10"
            : "=r"(asm_out)
            : "r"(v0 + iter), "r"(v1), "r"(v2), "r"(v3), 
              "r"(v4), "r"(v5), "r"(v6), "r"(v7), 
              "r"(v8), "r"(v9)
            : "cc"
        );
        asm_results += asm_out;
        
        /* 2. 11-operand inline assembly */
        int asm_out2;
        __asm__ volatile (
            "/* Custom 11-operand operation */\n\t"
            "mov %0, %1\n\t"
            "imul %0, %2\n\t"
            "add %0, %3\n\t"
            "sub %0, %4\n\t"
            "add %0, %5\n\t"
            "sub %0, %6\n\t"
            "add %0, %7\n\t"
            "sub %0, %8\n\t"
            "add %0, %9\n\t"
            "sub %0, %10\n\t"
            "add %0, %11"
            : "=r"(asm_out2)
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3),
              "r"(v4), "r"(v5), "r"(v6), "r"(v7),
              "r"(v8), "r"(v9), "r"(v10)
            : "cc"
        );
        asm_results += asm_out2;
        
        /* 3. Complex 11-operand C expression using all volatile variables */
        int64_t complex_expr = 
            ((((((((((int64_t)v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
               v6) ^ v7) + v8) * v9) / (v10 ? v10 : 1);
        expr_results += complex_expr;
        
        /* 4. Call 11-argument inline function */
        int64_t func_result = multi_operand_op(
            v0, v1, v2, v3, v4,
            v5, v6, v7, v8, v9, v10
        );
        func_results += func_result;
        
        /* 5. Vector operations with 10 and 11 elements */
        v10si vec10_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v10si vec10_result = vec10_a + vec10_b;
        
        v11si vec11_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec11_b = {v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0};
        v11si vec11_result = vec11_a + vec11_b;
        
        /* Extract results from vectors */
        for (int i = 0; i < 10; i++) {
            vector_results += vec10_result[i];
        }
        for (int i = 0; i < 11; i++) {
            vector_results += vec11_result[i];
        }
        
        /* 6. Mixed-type expression with 10+ operands */
        double mixed_result = 
            (double)v0 + (double)v1 - (float)v2 * (double)v3 +
            (long)v4 / (double)(v5 ? v5 : 1) + (short)v6 -
            (char)v7 * (float)v8 + (double)v9 - (float)v10 +
            d1 - f1 + (double)l1 - (double)s1 + (double)c1;
        
        expr_results += (int64_t)mixed_result;
        
        /* Modify variables slightly to prevent complete optimization */
        v0 = v0 ^ 1;
        v1 = v1 + 1;
        v2 = v2 - 1;
    }
    
    /* Compute final checksum to prevent dead code elimination */
    int64_t checksum = asm_results + expr_results + vector_results + func_results;
    
    /* Conditional branch depending on checksum */
    if (checksum > 0) {
        printf("Checksum (positive case): %lld\n", (long long)checksum);
    } else {
        printf("Checksum (negative case): %lld\n", (long long)checksum);
    }
    
    return (checksum > 0) ? 0 : 1;
}
