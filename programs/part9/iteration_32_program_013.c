#include <stdio.h>
#include <stdint.h>

/* Define vector types with exactly 10 and 11 elements */
typedef int v10si __attribute__((vector_size(10 * sizeof(int))));
typedef int v11si __attribute__((vector_size(11 * sizeof(int))));
typedef float v10sf __attribute__((vector_size(10 * sizeof(float))));
typedef float v11sf __attribute__((vector_size(11 * sizeof(float))));

/* Intrinsic-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_11(int a, int b, int c, int d, int e, int f, int g, 
            int h, int i, int j, int k) {
    return ((int64_t)a * b + c - d) << (e & 3) | (f ^ g) + (h * i) - (j / (k ? k : 1));
}

/* Function with 10 mixed-type arguments */
static inline float __attribute__((always_inline))
mixed_multi_op(char a, short b, int c, long d, float e, 
               double f, char g, short h, int i, float j) {
    return (a + b + c + d) * (e + (float)f) + (g * h * i) / j;
}

int main() {
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
    
    /* Additional mixed-type variables */
    volatile char c1 = 37;
    volatile short s1 = 41;
    volatile long l1 = 43;
    volatile float f1 = 47.0f;
    volatile double d1 = 53.0;
    
    int64_t checksum = 0;
    
    for (int iter = 0; iter < 1000; iter++) {
        int result_asm_10, result_asm_11;
        int64_t result_expr_11;
        float result_mixed;
        v10si vec_result_10;
        v11si vec_result_11;
        
        /* 1. 10-operand inline assembly */
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
            : "=r"(result_asm_10)
            : "r"(v0 + iter), "r"(v1), "r"(v2), "r"(v3), 
              "r"(v4), "r"(v5), "r"(v6), "r"(v7), "r"(v8)
            : "cc"
        );
        
        /* 2. 11-operand inline assembly */
        __asm__ volatile (
            "/* Custom 11-operand operation */\n\t"
            "mov %0, %1\n\t"
            "imul %0, %2\n\t"
            "add %0, %3\n\t"
            "sub %0, %4\n\t"
            "add %0, %5\n\t"
            "xor %0, %6\n\t"
            "or %0, %7\n\t"
            "and %0, %8\n\t"
            "add %0, %9\n\t"
            "sub %0, %10"
            : "=r"(result_asm_11)
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), 
              "r"(v4), "r"(v5), "r"(v6), "r"(v7),
              "r"(v8), "r"(v9), "r"(v10)
            : "cc"
        );
        
        /* 3. Complex 11-operand expression tree */
        result_expr_11 = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                           v6) ^ v7) + v8) * v9) / (v10 ? v10 : 1)) + 
                           ((v0 & v1) | (v2 ^ v3)) - ((v4 + v5) * (v6 - v7));
        
        /* 4. Call intrinsic-like function with 11 arguments */
        result_expr_11 += multi_op_11(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        
        /* 5. Mixed-type operation with 10 arguments */
        result_mixed = mixed_multi_op(c1, s1, v0, l1, f1, d1, 
                                      c1 + 1, s1 + 1, v1, f1 + 1.0f);
        
        /* 6. Vector operations */
        v10si vec_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        vec_result_10 = vec_a + vec_b;
        
        v11si vec_c = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec_d = {v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0};
        vec_result_11 = vec_c * vec_d;
        
        /* Extract results from vectors */
        int vec_sum_10 = 0, vec_sum_11 = 0;
        for (int i = 0; i < 10; i++) {
            vec_sum_10 += vec_result_10[i];
        }
        for (int i = 0; i < 11; i++) {
            vec_sum_11 += vec_result_11[i];
        }
        
        /* Update checksum with all results */
        checksum += result_asm_10 + result_asm_11 + result_expr_11 + 
                   (int64_t)result_mixed + vec_sum_10 + vec_sum_11;
        
        /* Modify volatile variables slightly to prevent optimization */
        v0 = v0 + 1;
        v1 = v1 - 1;
        v2 = v2 ^ iter;
    }
    
    /* Conditional branch based on checksum */
    if (checksum > 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    /* Additional complex expression using all 11 variables in one go */
    volatile int final_result = 
        (v0 * v1 + v2 - v3) | (v4 & v5) ^ (v6 << 2) + (v7 >> 1) * (v8 % 3) - (v9 / 2) + (v10 & 0xFF);
    
    printf("Final: %d\n", final_result);
    
    return (checksum > 1000000) ? 0 : 1;
}
