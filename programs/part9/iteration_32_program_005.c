#include <stdio.h>
#include <stdint.h>

/* Vector type with 11 elements */
typedef int v11si __attribute__((vector_size(44)));
typedef int v10si __attribute__((vector_size(40)));

/* Intrinsic-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_intrinsic(int a, int b, int c, int d, int e, 
                   int f, int g, int h, int i, int j, int k) {
    return ((int64_t)a * b + c - d) ^ (e << f) | (g & h) + (i * j) - k;
}

/* Intrinsic-like function with 10 arguments */
static inline int __attribute__((always_inline))
multi_op_10_intrinsic(int a, int b, int c, int d, int e,
                      int f, int g, int h, int i, int j) {
    return (((((((a + b) * c) - d) << e) & f) | g) ^ h) + i * j;
}

int main(void) {
    /* Declare 11 volatile variables with prime numbers */
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
    
    /* Mixed types for additional coverage */
    volatile char c1 = 37;
    volatile short s1 = 41;
    volatile long l1 = 43;
    volatile float f1 = 47.0f;
    volatile double d1 = 53.0;
    
    /* Results accumulators */
    int64_t asm_result = 0;
    int64_t expr_result = 0;
    int64_t vec_result = 0;
    int64_t intrinsic_result = 0;
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int temp_asm;
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
            : "=r"(temp_asm)
            : "r"(v0 + iter), "r"(v1), "r"(v2), "r"(v3), 
              "r"(v4), "r"(v5), "r"(v6), "r"(v7), "r"(v8)
            : "cc"
        );
        asm_result += temp_asm;
        
        /* 2. 11-operand inline assembly */
        int temp_asm11;
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
            : "=r"(temp_asm11)
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), 
              "r"(v4), "r"(v5), "r"(v6), "r"(v7), 
              "r"(v8), "r"(v9), "r"(v10)
            : "cc"
        );
        asm_result += temp_asm11;
        
        /* 3. Complex 11-operand expression tree */
        int temp_expr = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                           v6) ^ v7) + v8) * v9) / (v10 > 0 ? v10 : 1)) + 
                        ((int)c1 * s1) - (l1 % 17);
        expr_result += temp_expr;
        
        /* 4. 10-operand complex expression */
        int temp_expr10 = ((((((((v0 ^ v1) + v2) * v3) - v4) & v5) | 
                           v6) << (v7 & 3)) ^ v8) + v9 * (iter & 1);
        expr_result += temp_expr10;
        
        /* 5. Vector operations with 11 elements */
        v11si vec_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v0};
        v11si vec_c = vec_a + vec_b;
        
        /* Extract all elements from vector */
        int vec_sum = 0;
        for (int i = 0; i < 11; i++) {
            vec_sum += vec_c[i];
        }
        vec_result += vec_sum;
        
        /* 6. Vector operations with 10 elements */
        v10si vec10_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v10si vec10_c = vec10_a * vec10_b;
        
        int vec10_sum = 0;
        for (int i = 0; i < 10; i++) {
            vec10_sum += vec10_c[i];
        }
        vec_result += vec10_sum;
        
        /* 7. Intrinsic-like function with 11 arguments */
        int temp_intrinsic = multi_op_intrinsic(
            v0 + iter, v1, v2, v3, v4,
            v5, v6, v7, v8, v9, v10
        );
        intrinsic_result += temp_intrinsic;
        
        /* 8. Intrinsic-like function with 10 arguments */
        int temp_intrinsic10 = multi_op_10_intrinsic(
            v0, v1, v2, v3, v4,
            v5, v6, v7, v8, v9
        );
        intrinsic_result += temp_intrinsic10;
        
        /* 9. Mixed-type 11-operand expression */
        double mixed_result = (double)v0 + (float)v1 + (int)c1 + 
                             (long)s1 + v2 * 1.5 + v3 / 2.0 + 
                             (double)v4 - (float)v5 + v6 * 0.5 + 
                             v7 + (int)l1;
        intrinsic_result += (int64_t)mixed_result;
    }
    
    /* Compute final checksum to prevent dead code elimination */
    int64_t checksum = asm_result ^ expr_result ^ vec_result ^ intrinsic_result;
    
    /* Conditional branch depending on checksum */
    if (checksum != 0) {
        printf("Checksum: %lld\n", (long long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    /* Use all volatile variables in final expression */
    volatile int final_check = v0 + v1 + v2 + v3 + v4 + v5 + 
                              v6 + v7 + v8 + v9 + v10 + 
                              c1 + s1 + (int)l1 + (int)f1 + (int)d1;
    
    return (checksum & 0xFF) | (final_check & 0xFF);
}
