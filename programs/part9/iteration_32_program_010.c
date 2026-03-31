#include <stdio.h>
#include <stdint.h>

/* Vector type with 11 elements */
typedef int v11si __attribute__((vector_size(44)));
typedef float v11sf __attribute__((vector_size(44)));

/* Custom builtin-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_11(int a, int b, int c, int d, int e, 
            int f, int g, int h, int i, int j, int k) {
    return ((((((((((a + b) * c) - d) << e) & f) | g) ^ h) + i) * j) / (k ? k : 1));
}

/* Function with mixed types */
static inline float __attribute__((always_inline))
mixed_multi_op(float a, double b, int c, short d, char e,
               long f, float g, double h, int i, short j, char k) {
    return (a + (float)b + c + d + e + f + g + (float)h + i + j + k) / 11.0f;
}

int main(void) {
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
    
    /* Additional variables for mixed types */
    volatile float f0 = 2.5f;
    volatile double d0 = 3.14159;
    volatile short s0 = 5;
    volatile char c0 = 7;
    volatile long l0 = 11L;
    
    volatile float f1 = 13.7f;
    volatile double d1 = 17.89;
    volatile short s1 = 19;
    volatile char c1 = 23;
    
    /* Results accumulators */
    int64_t asm_result = 0;
    int64_t expr_result = 0;
    int64_t vec_result = 0;
    float mixed_result = 0.0f;
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int64_t out1;
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
        int64_t out2 = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                          v6) ^ v7) + v8) * v9) / (v10 ? v10 : 1)) + iter;
        expr_result += out2;
        
        /* 3. 11-element vector operations */
        v11si vec_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec_b = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        v11si vec_c = vec_a + vec_b + iter;
        
        /* Extract and sum all elements */
        int vec_sum = 0;
        for (int i = 0; i < 11; i++) {
            vec_sum += vec_c[i];
        }
        vec_result += vec_sum;
        
        /* 4. Call 11-argument function */
        int64_t func_result = multi_op_11(
            v0 + iter, v1, v2, v3, v4,
            v5, v6, v7, v8, v9, v10
        );
        expr_result += func_result;
        
        /* 5. Mixed-type 11-operand function */
        mixed_result += mixed_multi_op(
            f0 + iter * 0.1f, d0, v0, s0, c0,
            l0, f1, d1, v1, s1, c1
        );
        
        /* 6. Another 11-operand inline assembly attempt */
        int64_t out3;
        __asm__ volatile (
            "/* Another 11-operand pattern */\n\t"
            "mov %0, #0\n\t"
            "add %0, %0, %1\n\t"
            "add %0, %0, %2\n\t"
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
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = asm_result + expr_result + vec_result + (int64_t)mixed_result;
    
    /* Conditional branch depending on checksum */
    if (checksum > 0) {
        printf("Checksum: %lld\n", (long long)checksum);
    } else {
        printf("Negative checksum detected\n");
    }
    
    /* Use all variables in final computation to prevent optimization */
    volatile int final_check = 
        v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        (int)f0 + (int)d0 + s0 + c0 + (int)l0 +
        (int)f1 + (int)d1 + s1 + c1;
    
    return (final_check > 0) ? 0 : 1;
}
