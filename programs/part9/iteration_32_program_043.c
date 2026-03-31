/* Multi-operand test program for optabs.cc coverage */
#include <stdio.h>
#include <stdint.h>

/* Define vector types with 10 and 11 elements */
typedef int v10si __attribute__((vector_size(40)));
typedef int v11si __attribute__((vector_size(44)));
typedef float v10sf __attribute__((vector_size(40)));
typedef double v5d __attribute__((vector_size(40))); /* 5 doubles = 40 bytes */

/* 11-operand inline function */
static inline int64_t __attribute__((always_inline))
multi_op_11(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e,
            int64_t f, int64_t g, int64_t h, int64_t i, int64_t j,
            int64_t k) {
    return (((((((((a + b) * c) - d) ^ e) & f) | g) << (h & 0x3F)) >> (i & 0x3F)) + j) * k;
}

/* 10-operand inline function with mixed types */
static inline float __attribute__((always_inline))
mixed_op_10(float a, double b, int c, short d, char e,
            long f, float g, double h, int i, short j) {
    return (a + (float)b + c + d + e + f + g + (float)h + i + j);
}

int main() {
    /* Declare 11 volatile variables with prime numbers */
    volatile int64_t v0 = 2;
    volatile int64_t v1 = 3;
    volatile int64_t v2 = 5;
    volatile int64_t v3 = 7;
    volatile int64_t v4 = 11;
    volatile int64_t v5 = 13;
    volatile int64_t v6 = 17;
    volatile int64_t v7 = 19;
    volatile int64_t v8 = 23;
    volatile int64_t v9 = 29;
    volatile int64_t v10 = 31;
    
    /* Additional volatile variables for mixed types */
    volatile float f1 = 2.5f;
    volatile double d1 = 3.14159;
    volatile short s1 = 7;
    volatile char c1 = 11;
    volatile long l1 = 101;
    
    int64_t checksum = 0;
    
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int64_t asm_result;
        __asm__ volatile (
            "/* 10-operand dummy operation */\n\t"
            "add %0, %1, %2\n\t"
            "add %0, %0, %3\n\t"
            "add %0, %0, %4\n\t"
            "add %0, %0, %5\n\t"
            "add %0, %0, %6\n\t"
            "add %0, %0, %7\n\t"
            "add %0, %0, %8\n\t"
            "add %0, %0, %9\n\t"
            "add %0, %0, %10"
            : "=r"(asm_result)
            : "r"(v0 + iter), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
              "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9)
            : "cc"
        );
        checksum ^= asm_result;
        
        /* 2. 11-operand C expression using all volatile variables */
        int64_t expr_result = ((((((((((v0 + v1) * v2) - v3) ^ v4) & v5) | 
                                 v6) << (v7 & 0x3F)) >> (v8 & 0x3F)) + v9) * v10) / (iter + 1);
        checksum += expr_result;
        
        /* 3. Vector operations with 10 and 11 elements */
        v10si vec10_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v10si vec10_result = vec10_a + vec10_b;
        
        /* Extract result from vector */
        int64_t vec_sum = 0;
        for (int i = 0; i < 10; i++) {
            vec_sum += vec10_result[i];
        }
        checksum += vec_sum;
        
        /* 4. 11-operand function call */
        int64_t func_result = multi_op_11(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum ^= func_result;
        
        /* 5. Mixed-type 10-operand operation */
        float mixed_result = mixed_op_10(f1, d1, v0, s1, c1, l1, f1 + 1.0f, d1 * 2.0, v1, s1 + 1);
        checksum += (int64_t)mixed_result;
        
        /* 6. Another 11-operand inline assembly with different constraint types */
        int64_t asm_result2;
        __asm__ volatile (
            "/* 11-operand operation */\n\t"
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
            "add %0, %0, %10\n\t"
            "add %0, %0, %11"
            : "=r"(asm_result2)
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
              "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9),
              "r"(v10), "r"(iter)
            : "cc"
        );
        checksum += asm_result2;
        
        /* 7. Complex 11-operand bitwise expression */
        int64_t bitwise_result = (v0 & v1) | (v2 & v3) ^ (v4 & v5) | (v6 & v7) ^ (v8 & v9) | v10;
        bitwise_result = (bitwise_result << (v0 & 7)) >> (v1 & 7);
        bitwise_result = bitwise_result + v2 - v3 * v4 / (v5 + 1) % (v6 + 1);
        checksum ^= bitwise_result;
    }
    
    /* Prevent dead code elimination */
    if (checksum != 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum\n");
    }
    
    return (checksum & 1);
}
