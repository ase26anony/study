#include <stdio.h>
#include <stdint.h>

/* Vector type with 11 elements */
typedef int v11si __attribute__((vector_size(44)));
typedef int v10si __attribute__((vector_size(40)));

/* Intrinsic-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_11(int a, int b, int c, int d, int e, int f, int g, 
            int h, int i, int j, int k) {
    return ((int64_t)a * b + c - d) << (e & 3) | (f ^ g) + (h * i) - (j / (k ? k : 1));
}

/* Intrinsic-like function with 10 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_10(int a, int b, int c, int d, int e, int f, int g, 
            int h, int i, int j) {
    return ((int64_t)a + b - c) * d ^ (e | f) & (g << 2) + (h - i) * j;
}

int main() {
    /* 11 volatile variables initialized with primes */
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
    volatile char c1 = 127;
    volatile short s1 = 32767;
    volatile long l1 = 65537L;
    volatile float f1 = 3.14159f;
    volatile double d1 = 2.71828;
    
    int64_t checksum = 0;
    
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int64_t asm_result;
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
        checksum ^= asm_result;
        
        /* 2. 11-operand inline assembly */
        int64_t asm_result2;
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
            : "=r"(asm_result2)
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
              "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10)
            : "cc"
        );
        checksum += asm_result2 * 3;
        
        /* 3. Complex 11-operand expression tree */
        int64_t expr_result = (
            ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
            v6) ^ v7) + v8) * v9) / (v10 ? v10 : 1)) + 
            (c1 * s1) - (l1 % 17) + (int)(f1 * 100) + (int)(d1 * 100)
        );
        checksum |= expr_result;
        
        /* 4. Call 11-argument intrinsic-like function */
        int64_t func_result = multi_op_11(
            v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10
        );
        checksum ^= func_result;
        
        /* 5. Call 10-argument intrinsic-like function */
        int64_t func_result2 = multi_op_10(
            v0, v1, v2, v3, v4, v5, v6, v7, v8, v9
        );
        checksum += func_result2;
        
        /* 6. Vector operations with 11 elements */
        v11si vec_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec_b = {v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0};
        v11si vec_result = vec_a + vec_b;
        
        /* Extract all elements from vector */
        int vec_sum = 0;
        for (int i = 0; i < 11; i++) {
            vec_sum += vec_result[i];
        }
        checksum += vec_sum;
        
        /* 7. Vector operations with 10 elements */
        v10si vec10_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_b = {v9, v8, v7, v6, v5, v4, v3, v2, v1, v0};
        v10si vec10_result = vec10_a * vec10_b;
        
        int vec10_sum = 0;
        for (int i = 0; i < 10; i++) {
            vec10_sum += vec10_result[i];
        }
        checksum ^= vec10_sum;
        
        /* 8. Mixed-type 11-operand expression */
        double mixed_result = (
            (double)v0 * v1 + (double)v2 / (v3 ? v3 : 1) - 
            (double)v4 * v5 + (double)v6 / (v7 ? v7 : 1) +
            (double)v8 * v9 - (double)v10 + f1 * d1
        );
        checksum += (int64_t)mixed_result;
    }
    
    /* Prevent dead code elimination */
    if (checksum != 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return checksum != 0 ? 0 : 1;
}
