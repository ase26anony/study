#include <stdio.h>
#include <stdint.h>

/* Vector types for 10 and 11 elements */
typedef int v10si __attribute__((vector_size(40)));  /* 10 * 4 bytes */
typedef int v11si __attribute__((vector_size(44)));  /* 11 * 4 bytes */

/* Custom builtin-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_11(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e,
            int64_t f, int64_t g, int64_t h, int64_t i, int64_t j,
            int64_t k) {
    /* Complex expression that uses all 11 operands */
    return ((((((((((a + b) * c) - d) << (e & 0x3F)) & f) | g) ^ h) + i) * j) / (k | 1));
}

/* Function with 10 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_10(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e,
            int64_t f, int64_t g, int64_t h, int64_t i, int64_t j) {
    return (((((((((a ^ b) + c) * d) - e) & f) | g) << (h & 0x3F)) ^ i) + j);
}

int main() {
    /* Declare 11 volatile variables with distinct prime numbers */
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
    
    /* Mixed type variables */
    volatile char c1 = 37;
    volatile short s1 = 41;
    volatile int i1 = 43;
    volatile long l1 = 47;
    volatile float f1 = 53.0f;
    volatile double d1 = 59.0;
    
    /* Results accumulator */
    int64_t checksum = 0;
    
    /* Loop to ensure multiple executions */
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
            "add %0, %0, %9\n\t"
            "add %0, %0, %10"
            : "=r"(asm_result)
            : "r"(v0 + iter), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
              "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9)
            : "cc"
        );
        checksum ^= asm_result;
        
        /* 2. 11-operand C expression using all volatile variables */
        int64_t expr_result = ((((((((((v0 + v1) * v2) - v3) << (v4 & 0x3F)) & v5) | 
                                 v6) ^ v7) + v8) * v9) / (v10 | 1));
        checksum += expr_result;
        
        /* 3. 11-operand inline assembly */
        int64_t asm_result_11;
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
            "add %0, %0, %10\n\t"
            "sub %0, %0, %11"
            : "=r"(asm_result_11)
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
              "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9),
              "r"(v10)
            : "cc"
        );
        checksum ^= asm_result_11;
        
        /* 4. Vector operations with 10 elements */
        v10si vec10_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v10si vec10_result = vec10_a + vec10_b;
        
        /* Extract result from vector */
        int* vec10_ptr = (int*)&vec10_result;
        for (int i = 0; i < 10; i++) {
            checksum += vec10_ptr[i];
        }
        
        /* 5. Call 11-argument function */
        int64_t func_result = multi_op_11(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum += func_result;
        
        /* 6. Mixed-type 11-operand expression */
        double mixed_result = (double)(c1 + s1 + i1 + l1) + f1 + d1 + 
                             (double)v0 + (double)v1 + (double)v2 + 
                             (double)v3 + (double)v4;
        checksum += (int64_t)mixed_result;
        
        /* 7. Another 10-operand expression with different operation chain */
        int64_t chain_result = v0 * v1 + v2 * v3 - v4 * v5 + v6 * v7 - v8 * v9 + v10;
        checksum ^= chain_result;
        
        /* 8. Call 10-argument function */
        int64_t func10_result = multi_op_10(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum += func10_result;
        
        /* Modify variables slightly each iteration */
        v0 += 1;
        v1 += 2;
        v2 += 3;
    }
    
    /* Prevent dead code elimination */
    if (checksum != 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return (checksum & 1);
}
