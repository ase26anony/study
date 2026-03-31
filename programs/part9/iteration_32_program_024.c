#include <stdio.h>
#include <stdint.h>

/* Define vector types with 10 and 11 elements */
typedef int v10si __attribute__((vector_size(40)));  /* 10 * 4 bytes */
typedef int v11si __attribute__((vector_size(44)));  /* 11 * 4 bytes */

/* Intrinsic-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_11(int a, int b, int c, int d, int e, int f, 
            int g, int h, int i, int j, int k) {
    return (int64_t)((((((((((a + b) * c) - d) << e) & f) | g) ^ h) + i) * j) / k);
}

/* Intrinsic-like function with 10 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_10(int a, int b, int c, int d, int e, int f, 
            int g, int h, int i, int j) {
    return (int64_t)(((((((((a + b) * c) - d) << e) & f) | g) ^ h) + i) * j);
}

int main() {
    /* Declare 11 volatile variables with distinct prime numbers */
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
    
    /* Mixed type variables */
    volatile char c1 = 37;
    volatile short s1 = 41;
    volatile long l1 = 43;
    volatile float f1 = 47.0f;
    volatile double d1 = 53.0;
    
    /* Results accumulators */
    int64_t asm_result = 0;
    int64_t expr_result = 0;
    int64_t vector_result = 0;
    int64_t intrinsic_result = 0;
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int out1;
        __asm__ volatile (
            "/* Custom 10-operand operation */\n\t"
            "add %0, %1, %2\n\t"
            "add %0, %0, %3\n\t"
            "add %0, %0, %4\n\t"
            "add %0, %0, %5\n\t"
            "add %0, %0, %6\n\t"
            "add %0, %0, %7\n\t"
            "add %0, %0, %8\n\t"
            "add %0, %0, %9"
            : "=r"(out1)
            : "r"(v0 + iter), "r"(v1), "r"(v2), "r"(v3), 
              "r"(v4), "r"(v5), "r"(v6), "r"(v7), "r"(v8)
            : "cc"
        );
        asm_result += out1;
        
        /* 2. 11-operand C expression using all volatile variables */
        int64_t out2 = ((((((((((v0 + v1) * v2) - v3) << (v4 % 8)) & v5) | v6) ^ v7) + v8) * v9) / v10);
        expr_result += out2;
        
        /* 3. 11-operand inline assembly */
        int out3;
        __asm__ volatile (
            "/* Custom 11-operand operation */\n\t"
            "add %0, %1, %2\n\t"
            "add %0, %0, %3\n\t"
            "add %0, %0, %4\n\t"
            "add %0, %0, %5\n\t"
            "add %0, %0, %6\n\t"
            "add %0, %0, %7\n\t"
            "add %0, %0, %8\n\t"
            "add %0, %0, %9\n\t"
            "add %0, %0, %10"
            : "=r"(out3)
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), 
              "r"(v4), "r"(v5), "r"(v6), "r"(v7), 
              "r"(v8), "r"(v9), "r"(v10)
            : "cc"
        );
        asm_result += out3;
        
        /* 4. Vector operations with 10 elements */
        v10si vec10_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v10si vec10_c = vec10_a + vec10_b;
        
        /* Extract all elements from vector */
        int vec_sum = 0;
        for (int i = 0; i < 10; i++) {
            vec_sum += ((int*)&vec10_c)[i];
        }
        vector_result += vec_sum;
        
        /* 5. Vector operations with 11 elements */
        v11si vec11_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec11_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v0};
        v11si vec11_c = vec11_a * vec11_b;
        
        /* Extract all elements from 11-element vector */
        int vec11_sum = 0;
        for (int i = 0; i < 11; i++) {
            vec11_sum += ((int*)&vec11_c)[i];
        }
        vector_result += vec11_sum;
        
        /* 6. Call intrinsic-like function with 11 arguments */
        int64_t out4 = multi_op_11(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        intrinsic_result += out4;
        
        /* 7. Call intrinsic-like function with 10 arguments */
        int64_t out5 = multi_op_10(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9);
        intrinsic_result += out5;
        
        /* 8. Mixed-type expression with 11 operands */
        double mixed_result = (double)v0 + (double)v1 + (double)c1 + 
                             (double)s1 + (double)l1 + f1 + d1 + 
                             (double)v2 + (double)v3 + (double)v4 + 
                             (double)v5;
        intrinsic_result += (int64_t)mixed_result;
        
        /* 9. Complex chain with 11 variables using different operations */
        int64_t chain_result = (v0 * v1) + (v2 - v3) * (v4 / (v5 + 1)) + 
                              (v6 & v7) | (v8 ^ v9) + (v10 << 2) + 
                              (c1 * s1) - (l1 % 7);
        expr_result += chain_result;
    }
    
    /* Compute final checksum to prevent dead code elimination */
    int64_t checksum = asm_result + expr_result + vector_result + intrinsic_result;
    
    /* Conditional branch based on checksum */
    if (checksum > 0) {
        printf("Checksum: %lld\n", (long long)checksum);
    } else {
        printf("Negative checksum: %lld\n", (long long)checksum);
    }
    
    /* Additional complex expression using all variables one more time */
    volatile int final_expr = 
        ((((((((((v0 + v1 + v2) * v3 - v4) << (v5 % 4)) & v6) | 
        v7) ^ v8) + v9) * v10) + c1) * s1) / (l1 % 10 + 1);
    
    printf("Final expression: %d\n", final_expr);
    
    return (checksum > 0) ? 0 : 1;
}
