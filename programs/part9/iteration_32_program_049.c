#include <stdio.h>
#include <stdint.h>

/* Vector types for 10 and 11 elements */
typedef int v10si __attribute__((vector_size(40)));  /* 10 * 4 bytes */
typedef int v11si __attribute__((vector_size(44)));  /* 11 * 4 bytes */

/* Custom inline function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_11(int a, int b, int c, int d, int e, int f, int g, 
            int h, int i, int j, int k) {
    return ((int64_t)a * b + c - d) << (e & 3) | (f ^ g) + (h * i) - (j / (k + 1));
}

/* Custom inline function with 10 arguments */
static inline int __attribute__((always_inline))
multi_op_10(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    return (((((((((a + b) * c) - d) << (e & 3)) & f) | g) ^ h) + i) * j) >> 2;
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
    
    /* Additional volatile variables for mixed types */
    volatile char c1 = 127;
    volatile short s1 = 32767;
    volatile long l1 = 65537L;
    volatile float f1 = 3.14159f;
    volatile double d1 = 2.71828;
    
    /* Results accumulators */
    int64_t asm_result = 0;
    int64_t expr_result = 0;
    int64_t vec_result = 0;
    int64_t func_result = 0;
    int64_t mixed_result = 0;
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int out_asm;
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
            : "=r"(out_asm)
            : "r"(v0 + iter), "r"(v1), "r"(v2), "r"(v3), 
              "r"(v4), "r"(v5), "r"(v6), "r"(v7), "r"(v8)
            : "cc"
        );
        asm_result += out_asm;
        
        /* 2. 11-operand complex expression */
        int out_expr = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                          v6) ^ v7) + v8) * v9) / (v10 + 1)) + iter;
        expr_result += out_expr;
        
        /* 3. Vector operations with 10 and 11 elements */
        v10si vec10 = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_add = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        v10si vec10_result = vec10 + vec10_add;
        
        v11si vec11 = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec11_add = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        v11si vec11_result = vec11 + vec11_add;
        
        /* Extract results from vectors */
        int vec_sum = 0;
        for (int i = 0; i < 10; i++) vec_sum += vec10_result[i];
        for (int i = 0; i < 11; i++) vec_sum += vec11_result[i];
        vec_result += vec_sum;
        
        /* 4. Intrinsic-like function with 11 arguments */
        int64_t func_out = multi_op_11(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        func_result += func_out;
        
        /* 5. Mixed-type operations */
        int mixed_out = multi_op_10(
            (int)c1 + iter,
            (int)s1,
            v2,
            (int)l1,
            v4,
            (int)f1,
            v6,
            v7,
            (int)d1,
            v9
        );
        mixed_result += mixed_out;
        
        /* 6. Additional 11-operand inline assembly */
        int out_asm11;
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
            : "=r"(out_asm11)
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
              "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9),
              "r"(v10)
            : "cc"
        );
        asm_result += out_asm11;
    }
    
    /* Compute final checksum to prevent dead code elimination */
    int64_t checksum = asm_result + expr_result + vec_result + func_result + mixed_result;
    
    /* Conditional branch based on checksum */
    if (checksum > 0) {
        printf("Checksum: %lld\n", (long long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    /* Additional complex expression with exactly 11 operands */
    volatile int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    volatile int a6 = 6, a7 = 7, a8 = 8, a9 = 9, a10 = 10, a11 = 11;
    
    int final_expr = (((((((((a1 * a2) + a3) - a4) & a5) | a6) ^ a7) 
                      << (a8 & 3)) + a9) * a10) / a11;
    
    printf("Final expression result: %d\n", final_expr);
    
    return (checksum > 0) ? 0 : 1;
}
