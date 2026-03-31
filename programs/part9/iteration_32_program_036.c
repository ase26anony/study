#include <stdio.h>
#include <stdint.h>

/* Define vector types with 10 and 11 elements */
typedef int v10si __attribute__((vector_size(40)));
typedef int v11si __attribute__((vector_size(44)));
typedef float v10sf __attribute__((vector_size(40)));
typedef float v11sf __attribute__((vector_size(44)));

/* Intrinsic-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_11(int a, int b, int c, int d, int e, int f, int g, 
            int h, int i, int j, int k) {
    return ((int64_t)a * b + c * d - e * f + g * h - i * j) * k;
}

/* Another function with 10 arguments */
static inline float __attribute__((always_inline))
multi_op_10f(float a, float b, float c, float d, float e,
             float f, float g, float h, float i, float j) {
    return a + b - c * d / e + f - g * h / i + j;
}

int main() {
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
    volatile float f10 = 31.0f;
    
    /* Mixed type variables */
    volatile char c0 = 2;
    volatile short s0 = 3;
    volatile long l0 = 5L;
    volatile double d0 = 7.0;
    
    int64_t checksum = 0;
    
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int asm_result;
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
            : "=r"(asm_result)
            : "r"(v0 + iter), "r"(v1), "r"(v2), "r"(v3), 
              "r"(v4), "r"(v5), "r"(v6), "r"(v7), "r"(v8)
        );
        checksum += asm_result;
        
        /* 2. 11-operand C expression with mixed types */
        int64_t expr_result = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                                 v6) ^ v7) + v8) * v9) / (v10 > 0 ? v10 : 1)) + 
                              c0 - s0 + l0;
        checksum += expr_result;
        
        /* 3. 11-operand inline assembly with mixed registers */
        int asm_result2;
        __asm__ volatile (
            "/* Custom 11-operand operation */\n\t"
            "mov %0, %1\n\t"
            "add %0, %0, %2\n\t"
            "sub %0, %0, %3\n\t"
            "add %0, %0, %4\n\t"
            "sub %0, %0, %5\n\t"
            "add %0, %0, %6\n\t"
            "sub %0, %0, %7\n\t"
            "add %0, %0, %8\n\t"
            "sub %0, %0, %9\n\t"
            "add %0, %0, %10"
            : "=r"(asm_result2)
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
              "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9),
              "r"(v10)
        );
        checksum += asm_result2;
        
        /* 4. Vector operations with 10 and 11 elements */
        v10si vec10_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v10si vec10_result = vec10_a + vec10_b;
        
        v11si vec11_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec11_b = {v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0};
        v11si vec11_result = vec11_a - vec11_b;
        
        /* Extract results from vectors */
        for (int i = 0; i < 10; i++) checksum += vec10_result[i];
        for (int i = 0; i < 11; i++) checksum += vec11_result[i];
        
        /* 5. Call 11-argument function */
        int64_t func_result = multi_op_11(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum += func_result;
        
        /* 6. Mixed floating-point operation with 10 operands */
        float float_result = multi_op_10f(f0, f1, f2, f3, f4, f5, f6, f7, f8, f9);
        checksum += (int64_t)float_result;
        
        /* 7. Complex expression with 11 volatile variables */
        int complex_expr = v0 + 
                          (v1 * v2) - 
                          (v3 / (v4 ? v4 : 1)) + 
                          (v5 & v6) | 
                          (v7 ^ v8) + 
                          (v9 << 2) - 
                          (v10 >> 1) +
                          c0 * s0 -
                          l0 % 17;
        checksum += complex_expr;
        
        /* 8. Another 11-operand expression with type conversions */
        double mixed_expr = (double)v0 + 
                           (double)v1 * (double)v2 - 
                           (double)v3 / (double)v4 + 
                           (double)v5 - 
                           (double)v6 * (double)v7 + 
                           (double)v8 / (double)v9 - 
                           (double)v10 + 
                           d0;
        checksum += (int64_t)mixed_expr;
    }
    
    /* Prevent dead code elimination */
    if (checksum != 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return 0;
}
