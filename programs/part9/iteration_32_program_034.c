#include <stdio.h>
#include <stdint.h>

/* Vector types for 10 and 11 operands */
typedef int v10si __attribute__((vector_size(40)));  /* 10 * 4 bytes */
typedef int v11si __attribute__((vector_size(44)));  /* 11 * 4 bytes */

/* Custom inline function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_11(int a, int b, int c, int d, int e, int f, int g, 
            int h, int i, int j, int k) {
    return ((int64_t)a * b + c - d) << (e & 3) | (f ^ g) + h * i - j / (k + 1);
}

/* Custom inline function with 10 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_10(int a, int b, int c, int d, int e, int f, int g, 
            int h, int i, int j) {
    return ((int64_t)a + b) * c - d + (e << 2) & f | g ^ h + i * j;
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
    
    /* Additional mixed-type variables */
    volatile char c1 = 37;
    volatile short s1 = 41;
    volatile long l1 = 43;
    volatile float f1 = 47.0f;
    volatile double d1 = 53.0;
    
    int64_t checksum = 0;
    
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int64_t asm_result;
        __asm__ volatile (
            "/* Custom 10-operand operation */\n\t"
            "add %[out], %[a], %[b]\n\t"
            "add %[out], %[out], %[c]\n\t"
            "add %[out], %[out], %[d]\n\t"
            "add %[out], %[out], %[e]\n\t"
            "add %[out], %[out], %[f]\n\t"
            "add %[out], %[out], %[g]\n\t"
            "add %[out], %[out], %[h]\n\t"
            "add %[out], %[out], %[i]\n\t"
            "add %[out], %[out], %[j]"
            : [out] "=r" (asm_result)
            : [a] "r" (v0 + iter), [b] "r" (v1), [c] "r" (v2), 
              [d] "r" (v3), [e] "r" (v4), [f] "r" (v5),
              [g] "r" (v6), [h] "r" (v7), [i] "r" (v8), [j] "r" (v9)
            : "cc"
        );
        checksum ^= asm_result;
        
        /* 2. 11-operand complex expression */
        int64_t expr_result = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                                 v6) ^ v7) + v8) * v9) / (v10 + 1)) + 
                             ((c1 * s1) - l1) + (int)(f1 * d1);
        checksum += expr_result;
        
        /* 3. Vector operations with 10 elements */
        v10si vec10_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v10si vec10_result = vec10_a + vec10_b;
        
        /* Extract all elements to force scalarization */
        int vec_sum = 0;
        for (int i = 0; i < 10; i++) {
            vec_sum += vec10_result[i];
        }
        checksum *= (vec_sum + 1);
        
        /* 4. 11-operand vector type */
        v11si vec11_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec11_b = {v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0};
        v11si vec11_result = vec11_a * vec11_b;
        
        /* Force use of all vector elements */
        int vec_prod = 1;
        for (int i = 0; i < 11; i++) {
            vec_prod *= vec11_result[i] & 0xFF;
        }
        checksum += vec_prod;
        
        /* 5. Call 11-argument inline function */
        int64_t func_result = multi_op_11(
            v0 + iter, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10
        );
        checksum ^= func_result;
        
        /* 6. Call 10-argument inline function with mixed types */
        int64_t func_result2 = multi_op_10(
            c1, s1, v2, l1, v4, (int)f1, (int)d1, v7, v8, v9
        );
        checksum += func_result2;
        
        /* 7. Another 11-operand expression with mixed operations */
        double double_result = (
            (double)v0 * v1 + v2 - v3 * v4 / (v5 + 1.0) + 
            v6 * v7 - v8 + v9 * v10 + c1 + s1 + l1 + f1 + d1
        );
        checksum += (int64_t)double_result;
    }
    
    /* Prevent dead code elimination */
    if (checksum != 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return (checksum & 1);
}
