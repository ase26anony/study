#include <stdio.h>
#include <stdint.h>

/* Vector types for 10 and 11 elements */
typedef int v10si __attribute__((vector_size(40)));
typedef int v11si __attribute__((vector_size(44)));
typedef float v10sf __attribute__((vector_size(40)));
typedef float v11sf __attribute__((vector_size(44)));

/* Custom builtin-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_11(int a, int b, int c, int d, int e, 
            int f, int g, int h, int i, int j, int k) {
    return ((((((((((int64_t)a + b) * c) - d) << (e & 31)) & f) | 
               g) ^ h) + i) * j) / (k ? k : 1);
}

/* Function with mixed types and 10 arguments */
static inline float __attribute__((always_inline))
mixed_multi_op(char a, short b, int c, long d, float e,
               double f, char g, short h, int i, float j) {
    return (float)(a + b + c + d) + e + (float)f + g + h + i + j;
}

int main() {
    /* 11 volatile variables with prime numbers */
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
    
    /* Additional volatile variables for mixed types */
    volatile char cv0 = 37;
    volatile short sv0 = 41;
    volatile long lv0 = 43;
    volatile float fv0 = 47.0f;
    volatile double dv0 = 53.0;
    
    /* Results accumulators */
    int64_t asm_result = 0;
    int64_t expr_result = 0;
    int64_t vec_result = 0;
    float mixed_result = 0.0f;
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++) {
        /* 10-operand inline assembly */
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
        
        /* 11-operand C expression using all volatile variables */
        int temp_expr = (((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                          v6) ^ v7) + v8) * v9) / (v10 ? v10 : 1);
        expr_result += temp_expr + iter;
        
        /* 11-element vector operations */
        v11si vec_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec_b = {v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0};
        v11si vec_c = vec_a + vec_b + iter;
        
        /* Sum all vector elements */
        int temp_vec = 0;
        for (int i = 0; i < 11; i++) {
            temp_vec += vec_c[i];
        }
        vec_result += temp_vec;
        
        /* 10-element mixed-type vector */
        v10sf vec_f1 = {fv0, fv0+1, fv0+2, fv0+3, fv0+4, 
                       fv0+5, fv0+6, fv0+7, fv0+8, fv0+9};
        v10sf vec_f2 = vec_f1 * (float)(iter + 1);
        float temp_vecf = 0.0f;
        for (int i = 0; i < 10; i++) {
            temp_vecf += vec_f2[i];
        }
        mixed_result += temp_vecf;
        
        /* Call 11-argument function */
        expr_result += multi_op_11(v0 + iter, v1, v2, v3, v4, v5, v6, 
                                  v7, v8, v9, v10);
        
        /* Call mixed-type 10-argument function */
        mixed_result += mixed_multi_op(cv0, sv0, v0, lv0, fv0,
                                      dv0, cv0+1, sv0+1, v1, fv0+1);
    }
    
    /* Complex checksum to prevent dead code elimination */
    int64_t checksum = asm_result ^ expr_result ^ vec_result ^ (int64_t)mixed_result;
    
    /* Conditional branch depending on checksum */
    if (checksum > 0) {
        printf("Positive checksum: %lld\n", (long long)checksum);
    } else if (checksum < 0) {
        printf("Negative checksum: %lld\n", (long long)checksum);
    } else {
        printf("Zero checksum\n");
    }
    
    /* Additional complex expression with exactly 11 operands */
    volatile int a1 = 59, a2 = 61, a3 = 67, a4 = 71, a5 = 73;
    volatile int a6 = 79, a7 = 83, a8 = 89, a9 = 97, a10 = 101, a11 = 103;
    
    int final_expr = (((((((((a1 * a2) + a3) - a4) & a5) | a6) ^ a7) 
                      << (a8 & 7)) + a9) * a10) / a11;
    
    printf("Final expression result: %d\n", final_expr);
    
    /* 11-operand inline assembly for direct triggering */
    int final_asm;
    __asm__ volatile (
        "/* 11-operand assembly pattern */\n\t"
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        "add %0, %0, %11"
        : "=r"(final_asm)
        : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
          "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10), "r"(a11)
        : "cc"
    );
    
    printf("Final assembly result: %d\n", final_asm);
    
    return (checksum != 0) ? 0 : 1;
}
