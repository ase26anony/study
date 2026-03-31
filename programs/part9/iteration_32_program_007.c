#include <stdio.h>
#include <stdint.h>

/* Vector types for 10 and 11 elements */
typedef int v10si __attribute__((vector_size(40)));
typedef int v11si __attribute__((vector_size(44)));
typedef float v10sf __attribute__((vector_size(40)));
typedef float v11sf __attribute__((vector_size(44)));

/* 11-argument inline function */
static inline int64_t __attribute__((always_inline))
multi_op_11(int a, int b, int c, int d, int e, int f, int g, 
            int h, int i, int j, int k) {
    return ((int64_t)a * b + c - d) * (e + f) / (g + 1) ^ (h << 2) | (i & j) + k;
}

/* 10-argument inline function */
static inline float __attribute__((always_inline))
multi_op_10(float a, float b, float c, float d, float e,
            float f, float g, float h, float i, float j) {
    return (((a + b) * c - d) / e + f) * g - h + i / j;
}

int main(void) {
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
    
    /* Additional volatile floats for mixed-type operations */
    volatile float f0 = 2.0f;
    volatile float f1 = 3.0f;
    volatile float f2 = 5.0f;
    volatile float f3 = 7.0f;
    volatile float f4 = 11.0f;
    volatile float f5 = 13.0f;
    volatile float f6 = 17.0f;
    volatile float f7 = 19.0f;
    volatile float f8 = 23.0f;
    volatile float f9 = 29.0f;
    
    volatile long long result_asm_10 = 0;
    volatile long long result_expr_11 = 0;
    volatile int result_vec_10 = 0;
    volatile float result_mixed = 0.0f;
    
    /* Initialize vectors */
    v10si vec10_int = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
    v11si vec11_int = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
    v10sf vec10_float = {f0, f1, f2, f3, f4, f5, f6, f7, f8, f9};
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++) {
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
        
        /* 2. 11-operand complex expression */
        result_expr_11 = ((((((((((v0 + iter) * v1) - v2) << (v3 & 3)) & v4) | 
                            v5) ^ v6) + v7) * v8) / (v9 | 1)) + v10;
        
        /* 3. Vector operations with 10 elements */
        v10si vec_temp = vec10_int + (v10si){iter, iter, iter, iter, iter, 
                                            iter, iter, iter, iter, iter};
        /* Extract result through complex operation */
        result_vec_10 = vec_temp[0] + vec_temp[1] - vec_temp[2] * vec_temp[3] +
                       vec_temp[4] / (vec_temp[5] + 1) | vec_temp[6] & 
                       vec_temp[7] ^ vec_temp[8] + vec_temp[9];
        
        /* 4. Mixed-type operations */
        result_mixed = multi_op_10(f0 + iter, f1, f2, f3, f4, f5, f6, f7, f8, f9);
        
        /* 5. 11-operand function call */
        volatile int64_t func_result = multi_op_11(v0 + iter, v1, v2, v3, v4, v5,
                                                  v6, v7, v8, v9, v10);
        
        /* Use results to prevent elimination */
        if (iter % 2 == 0) {
            result_asm_10 += func_result;
        } else {
            result_expr_11 ^= func_result;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = (uint64_t)result_asm_10 + 
                       (uint64_t)result_expr_11 + 
                       (uint64_t)result_vec_10 + 
                       (uint64_t)result_mixed;
    
    /* Conditional branch depending on checksum */
    if (checksum != 0) {
        printf("Checksum: %llu\n", (unsigned long long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return 0;
}
