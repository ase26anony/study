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
    return ((int64_t)a * b + c - d) << (e & 3) | (f ^ g) + (h * i) - (j / (k ? k : 1));
}

/* 10-argument inline function with mixed types */
static inline float __attribute__((always_inline))
mixed_op_10(float a, double b, int c, short d, char e, 
            float f, double g, int h, short i, char j) {
    return a + (float)b + c + d + e + f + (float)g + h + i + j;
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
    
    /* Additional volatile variables for mixed types */
    volatile float f0 = 2.5f;
    volatile double d0 = 3.14159;
    volatile short s0 = 42;
    volatile char c0 = 7;
    
    volatile float f1 = 1.618f;
    volatile double d1 = 2.71828;
    volatile short s1 = 99;
    volatile char c1 = 13;
    
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
            : "cc"
        );
        checksum += asm_result;
        
        /* 2. 11-operand inline assembly */
        int asm_result2;
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
            : "=r"(asm_result2)
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
              "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9),
              "r"(v10 + iter)
            : "cc"
        );
        checksum += asm_result2;
        
        /* 3. Complex 11-operand C expression */
        int expr_result = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                              v6) ^ v7) + v8) * v9) / (v10 ? v10 : 1)) + iter;
        checksum += expr_result;
        
        /* 4. 11-argument function call */
        int64_t func_result = multi_op_11(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum += func_result;
        
        /* 5. Mixed-type 10-operand function */
        float float_result = mixed_op_10(f0, d0, v0, s0, c0, f1, d1, v1, s1, c1);
        checksum += (int64_t)float_result;
        
        /* 6. Vector operations */
        v10si vec10_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v10si vec10_result = vec10_a + vec10_b;
        
        v11si vec11_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec11_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v0};
        v11si vec11_result = vec11_a + vec11_b;
        
        /* Extract results from vectors */
        for (int i = 0; i < 10; i++) {
            checksum += vec10_result[i];
        }
        for (int i = 0; i < 11; i++) {
            checksum += vec11_result[i];
        }
        
        /* 7. Another complex expression with 10 operands */
        int expr2_result = (v0 * v1) + (v2 - v3) | (v4 & v5) ^ (v6 << 2) + (v7 >> 1) * (v8 % 9) - v9;
        checksum += expr2_result;
        
        /* 8. 11-operand expression with mixed operations */
        int expr3_result = v0 + (v1 * v2) - (v3 / (v4 ? v4 : 1)) | (v5 & v6) ^ (v7 << (v8 & 3)) + (v9 * v10);
        checksum += expr3_result;
    }
    
    /* Prevent dead code elimination */
    if (checksum != 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return (checksum & 1); /* Return 0 or 1 based on checksum */
}
