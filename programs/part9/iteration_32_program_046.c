#include <stdio.h>
#include <stdint.h>

/* Vector types for 10 and 11 elements */
typedef int v10si __attribute__((vector_size(40)));
typedef int v11si __attribute__((vector_size(44)));
typedef float v10sf __attribute__((vector_size(40)));
typedef float v11sf __attribute__((vector_size(44)));

/* Custom inline function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_11(int a, short b, char c, long d, int e, 
            float f, double g, int h, short i, char j, long k) {
    return (int64_t)((a + b + c + d + e) * (h + i + j + k)) + (int64_t)(f * g);
}

/* Custom inline function with 10 arguments */
static inline int __attribute__((always_inline))
multi_op_10(int a, int b, int c, int d, int e,
            int f, int g, int h, int i, int j) {
    return (((((((((a + b) * c) - d) << e) & f) | g) ^ h) + i) * j);
}

int main() {
    /* Declare 11 volatile variables with distinct prime numbers */
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
    
    volatile int result_asm_10 = 0;
    volatile int64_t result_expr_11 = 0;
    volatile int result_vector = 0;
    volatile int64_t checksum = 0;
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int asm_result;
        __asm__ volatile (
            "add %0, %1, %2\n\t"
            "add %0, %0, %3\n\t"
            "add %0, %0, %4\n\t"
            "add %0, %0, %5\n\t"
            "add %0, %0, %6\n\t"
            "add %0, %0, %7\n\t"
            "add %0, %0, %8\n\t"
            "add %0, %0, %9"
            : "=r"(asm_result)
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
              "r"(v7), "r"(v8), "r"(v9), "r"(v10)
            : "cc"
        );
        result_asm_10 = asm_result;
        
        /* 2. 11-operand C expression using all volatile variables */
        result_expr_11 = multi_op_11(v0, v1, v2, v3, v4, 
                                     v5, v6, v7, v8, v9, v10);
        
        /* 3. 11-element vector operations */
        v11si vec_a = {v0, v1, v2, v3, v4, v7, v8, v9, v10, 37, 41};
        v11si vec_b = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        v11si vec_result = vec_a + vec_b;
        
        /* Extract result from vector */
        for (int i = 0; i < 11; i++) {
            result_vector += vec_result[i];
        }
        
        /* 4. Mixed-type 10-operand expression */
        int mixed_result = multi_op_10(v0, v7, v4, v1, v8, 
                                       v9, v2, v3, v10, iter);
        
        /* Update checksum */
        checksum += result_asm_10 + result_expr_11 + result_vector + mixed_result;
        
        /* Modify volatile variables slightly to prevent optimization */
        v0 += 1; v1 += 1; v2 += 1; v3 += 1; v4 += 1;
        v5 += 1.0f; v6 += 1.0;
        v7 += 1; v8 += 1; v9 += 1; v10 += 1;
    }
    
    /* 5. Conditional branch depending on checksum */
    if (checksum > 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    /* Additional test with 10-element floating point vector */
    v10sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    v10sf fvec_b = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f};
    v10sf fvec_result = fvec_a * fvec_b;
    
    float fsum = 0.0f;
    for (int i = 0; i < 10; i++) {
        fsum += fvec_result[i];
    }
    printf("Vector float sum: %f\n", fsum);
    
    /* Test with builtin-like pattern using many arguments */
    volatile int b0 = 101, b1 = 103, b2 = 107, b3 = 109, b4 = 113;
    volatile int b5 = 127, b6 = 131, b7 = 137, b8 = 139, b9 = 149, b10 = 151;
    
    /* Complex expression tree with 11 operands */
    int complex_result = ((((((((((b0 + b1) * b2) - b3) << (b4 & 3)) & b5) | 
                            b6) ^ b7) + b8) * b9) / (b10 & 255));
    
    printf("Complex result: %d\n", complex_result);
    
    return (checksum > 1000000) ? 0 : 1;
}
