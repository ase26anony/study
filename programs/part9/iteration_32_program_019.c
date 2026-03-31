#include <stdio.h>
#include <stdint.h>

/* Vector types for 10 and 11 elements */
typedef int v10si __attribute__((vector_size(10 * sizeof(int))));
typedef int v11si __attribute__((vector_size(11 * sizeof(int))));
typedef float v10sf __attribute__((vector_size(10 * sizeof(float))));
typedef double v11df __attribute__((vector_size(11 * sizeof(double))));

/* Intrinsic-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_intrinsic(int a, short b, char c, long d, int e, 
                   float f, double g, int h, short i, 
                   char j, long k) {
    /* Complex operation mixing types */
    return (int64_t)((a * b) + (c << 3) - (d / 2) + 
                     (int)(f * 100) + (int)(g * 1000) + 
                     (h & 0xFF) | (i << 8) ^ (j * 7) + k);
}

/* Function with 10 arguments */
static inline float __attribute__((always_inline))
multi_op_float(float a, double b, float c, double d, float e,
               float f, double g, float h, double i, float j) {
    return a + (float)b + c + (float)d + e + f + (float)g + h + (float)i + j;
}

int main() {
    /* Declare 11 volatile variables with prime numbers */
    volatile int v0 = 2;
    volatile short v1 = 3;
    volatile char v2 = 5;
    volatile long v3 = 7;
    volatile int v4 = 11;
    volatile float v5 = 13.0f;
    volatile double v6 = 17.0;
    volatile int v7 = 19;
    volatile short v8 = 23;
    volatile char v9 = 29;
    volatile long v10 = 31;
    
    /* Results accumulators */
    int64_t asm_result = 0;
    int64_t expr_result = 0;
    int64_t vec_result = 0;
    int64_t intrinsic_result = 0;
    float float_result = 0.0f;
    
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
            : "r"((int64_t)v0), "r"((int64_t)v1), "r"((int64_t)v2),
              "r"((int64_t)v3), "r"((int64_t)v4), "r"((int64_t)v7),
              "r"((int64_t)v8), "r"((int64_t)v9), "r"((int64_t)iter)
        );
        asm_result += out1;
        
        /* 2. 11-operand complex expression */
        int64_t out2 = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v7) | 
                          v8) ^ v9) + v10) * (iter & 0xFF)) / 
                          ((v0 > 0) ? 1 : 2)) % 256;
        expr_result += out2;
        
        /* 3. 11-operand inline assembly */
        int64_t out3;
        __asm__ volatile (
            "/* Custom 11-operand operation */\n\t"
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
            : "r"((int64_t)v0), "r"((int64_t)v1), "r"((int64_t)v2),
              "r"((int64_t)v3), "r"((int64_t)v4), "r"((int64_t)v7),
              "r"((int64_t)v8), "r"((int64_t)v9), "r"((int64_t)v10),
              "r"((int64_t)iter)
        );
        asm_result += out3;
        
        /* 4. Vector operations */
        v10si vec10_a = {v0, v1, v2, v3, v4, v7, v8, v9, iter, iter+1};
        v10si vec10_b = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        v10si vec10_res = vec10_a + vec10_b;
        
        v11si vec11_a = {v0, v1, v2, v3, v4, v7, v8, v9, v10, iter, iter*2};
        v11si vec11_b = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110};
        v11si vec11_res = vec11_a * vec11_b;
        
        /* Extract results from vectors */
        for (int i = 0; i < 10; i++) {
            vec_result += vec10_res[i];
        }
        for (int i = 0; i < 11; i++) {
            vec_result += vec11_res[i];
        }
        
        /* 5. Mixed floating-point vector operations */
        v10sf vec10f_a = {v5, v5*2, v5*3, v5*4, v5*5, 
                         v5*6, v5*7, v5*8, v5*9, v5*10};
        v10sf vec10f_b = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 
                         6.6f, 7.7f, 8.8f, 9.9f, 10.1f};
        v10sf vec10f_res = vec10f_a * vec10f_b;
        
        v11df vec11d_a = {v6, v6*2, v6*3, v6*4, v6*5, v6*6,
                         v6*7, v6*8, v6*9, v6*10, v6*11};
        v11df vec11d_b = {1.01, 2.02, 3.03, 4.04, 5.05, 6.06,
                         7.07, 8.08, 9.09, 10.10, 11.11};
        v11df vec11d_res = vec11d_a + vec11d_b;
        
        /* 6. Intrinsic calls with many arguments */
        intrinsic_result += multi_op_intrinsic(v0, v1, v2, v3, v4,
                                              v5 + iter, v6 + iter,
                                              v7, v8, v9, v10);
        
        float_result += multi_op_float(v5, v6, v5*0.5f, v6*0.5,
                                      v5*0.25f, v5*0.75f, v6*0.75,
                                      v5*1.5f, v6*1.5, v5*2.0f);
        
        /* Modify variables slightly each iteration */
        v0 += 1;
        v1 += 1;
        v2 += 1;
        v3 += 1;
        v4 += 1;
        v5 += 0.1f;
        v6 += 0.01;
        v7 += 1;
        v8 += 1;
        v9 += 1;
        v10 += 1;
    }
    
    /* Compute final checksum to prevent dead code elimination */
    int64_t checksum = asm_result + expr_result + vec_result + 
                      intrinsic_result + (int64_t)float_result;
    
    /* Conditional branch depending on checksum */
    if (checksum > 0) {
        printf("Checksum: %lld\n", (long long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    /* Additional complex expression using all 11 variables one last time */
    volatile int final_expr = 
        (((((((((v0 * v1) + v2) - v3) / (v4 ? v4 : 1)) & v7) | 
           v8) ^ v9) << (v10 % 8)) + (int)v5 + (int)v6) % 65536;
    
    printf("Final expression: %d\n", final_expr);
    
    return (checksum > 1000000) ? 0 : 1;
}
