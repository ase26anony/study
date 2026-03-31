#include <stdio.h>
#include <stdint.h>

/* Define vector types with 10 and 11 elements */
typedef int v10si __attribute__((vector_size(40)));  /* 10 * 4 bytes */
typedef int v11si __attribute__((vector_size(44)));  /* 11 * 4 bytes */

/* Intrinsic-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_11(int a, int b, int c, int d, int e, int f, 
            int g, int h, int i, int j, int k) {
    return ((int64_t)a * b + c - d) << (e & 3) | (f ^ g) + h * i - j / (k + 1);
}

/* Function with 10 arguments */
static inline int __attribute__((always_inline))
multi_op_10(int a, int b, int c, int d, int e, 
            int f, int g, int h, int i, int j) {
    return (((((((a + b) * c - d) << (e & 3)) & f) | g) ^ h) + i) * j;
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
    
    /* Mixed type variables */
    volatile char c1 = 37;
    volatile short s1 = 41;
    volatile long l1 = 43;
    volatile float f1 = 47.0f;
    volatile double d1 = 53.0;
    
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
        
        /* 2. 11-operand C expression using all volatile variables */
        int expr_result = ((((((((((v0 + v1) * v2 - v3) << (v4 & 3)) & v5) | 
                              v6) ^ v7) + v8) * v9 - v10) / (iter + 1)) + 
                          c1 - s1) * (l1 & 0xFF);
        checksum += expr_result;
        
        /* 3. 11-operand builtin-like operation using function */
        int64_t func_result = multi_op_11(
            v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10
        );
        checksum += func_result;
        
        /* 4. 10-operand function call */
        int func10_result = multi_op_10(
            v0, v1, v2, v3, v4, v5, v6, v7, v8, v9
        );
        checksum += func10_result;
        
        /* 5. Vector operations with 10 and 11 elements */
        v10si vec10 = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_add = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        v10si vec10_result = vec10 + vec10_add;
        
        /* Extract all elements from 10-element vector */
        int vec10_sum = 0;
        for (int i = 0; i < 10; i++) {
            vec10_sum += vec10_result[i];
        }
        checksum += vec10_sum;
        
        /* 11-element vector operation */
        v11si vec11 = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec11_add = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        v11si vec11_result = vec11 + vec11_add;
        
        /* Extract all elements from 11-element vector */
        int vec11_sum = 0;
        for (int i = 0; i < 11; i++) {
            vec11_sum += vec11_result[i];
        }
        checksum += vec11_sum;
        
        /* 6. Mixed-type 11-operand expression */
        double mixed_result = (
            (double)v0 * f1 + 
            (double)v1 * d1 - 
            (double)v2 / f1 + 
            (double)v3 * (c1 + 1) + 
            (double)v4 * (s1 - 1) + 
            (double)v5 * (l1 & 0xFF) + 
            (double)v6 * (iter + 1) - 
            (double)v7 / (iter + 2) + 
            (double)v8 * 1.5 + 
            (double)v9 * 2.5 - 
            (double)v10 * 0.5
        );
        checksum += (int64_t)mixed_result;
        
        /* 7. Another 11-operand inline assembly with mixed constraints */
        int64_t asm11_result;
        __asm__ volatile (
            "/* 11-operand dummy operation */\n\t"
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
            : "=r"(asm11_result)
            : "r"((int64_t)v0), "r"((int64_t)v1), "r"((int64_t)v2),
              "r"((int64_t)v3), "r"((int64_t)v4), "r"((int64_t)v5),
              "r"((int64_t)v6), "r"((int64_t)v7), "r"((int64_t)v8),
              "r"((int64_t)v9), "r"((int64_t)v10)
            : "cc"
        );
        checksum += asm11_result;
    }
    
    /* Prevent dead code elimination with conditional branch */
    if (checksum != 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return (checksum & 1);
}
