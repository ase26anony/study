#include <stdio.h>
#include <stdint.h>

/* Vector types for 10 and 11 elements */
typedef int v10si __attribute__((vector_size(40)));
typedef int v11si __attribute__((vector_size(44)));
typedef float v10sf __attribute__((vector_size(40)));
typedef double v11df __attribute__((vector_size(88)));

/* Intrinsic-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_intrinsic(int a, short b, char c, long d, int e, 
                   float f, double g, int h, short i, char j, long k) {
    return (int64_t)((a + b + c + d + e) * (h + i + j + k)) + (int64_t)(f * g);
}

/* Custom inline assembly with 10 operands */
static inline int custom_10_op(int a, int b, int c, int d, int e,
                               int f, int g, int h, int i, int j) {
    int result;
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    return result;
}

/* Custom inline assembly with 11 operands */
static inline int custom_11_op(int a, int b, int c, int d, int e,
                               int f, int g, int h, int i, int j, int k) {
    int result;
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        "add %0, %0, %11"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j), "r"(k)
        : "cc"
    );
    return result;
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
    
    /* Additional variables for vector operations */
    volatile int a0 = 37, a1 = 41, a2 = 43, a3 = 47, a4 = 53;
    volatile int a5 = 59, a6 = 61, a7 = 67, a8 = 71, a9 = 73;
    volatile int a10 = 79;
    
    int64_t checksum = 0;
    
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int asm_result_10 = custom_10_op(v0, v1, v2, v3, v4, v7, v8, v9, a0, a1);
        checksum += asm_result_10;
        
        /* 2. 11-operand inline assembly */
        int asm_result_11 = custom_11_op(v0, v1, v2, v3, v4, v7, v8, v9, v10, a0, a1);
        checksum += asm_result_11;
        
        /* 3. Complex 11-operand expression tree */
        int complex_expr = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v7) | 
                              v8) ^ v9) + a0) * a1) / (a2 + 1)) % (a3 + 1);
        checksum += complex_expr;
        
        /* 4. Intrinsic-like function with 11 mixed-type arguments */
        int64_t intrinsic_result = multi_op_intrinsic(
            v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10
        );
        checksum += intrinsic_result;
        
        /* 5. Vector operations with 10 elements */
        v10si vec10_a = {a0, a1, a2, a3, a4, a5, a6, a7, a8, a9};
        v10si vec10_b = {v0, v1, v2, v3, v4, v7, v8, v9, a0, a1};
        v10si vec10_result = vec10_a + vec10_b;
        
        /* Extract all elements from vector result */
        for (int i = 0; i < 10; i++) {
            checksum += vec10_result[i];
        }
        
        /* 6. Vector operations with 11 elements */
        v11si vec11_a = {a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10};
        v11si vec11_b = {v0, v1, v2, v3, v4, v7, v8, v9, v10, a0, a1};
        v11si vec11_result = vec11_a + vec11_b;
        
        /* Extract all elements from 11-element vector */
        for (int i = 0; i < 11; i++) {
            checksum += vec11_result[i];
        }
        
        /* 7. Mixed floating-point vector operations */
        v10sf vec10_f = {v5, v5*2, v5*3, v5*4, v5*5, v5*6, v5*7, v5*8, v5*9, v5*10};
        v10sf vec10_f_result = vec10_f * 2.0f;
        
        v11df vec11_d = {v6, v6*2, v6*3, v6*4, v6*5, v6*6, v6*7, v6*8, v6*9, v6*10, v6*11};
        v11df vec11_d_result = vec11_d * 2.0;
        
        /* Use results to prevent elimination */
        checksum += (int64_t)vec10_f_result[0] + (int64_t)vec11_d_result[0];
        
        /* Modify volatile variables slightly each iteration */
        v0 += 1;
        v1 += 1;
        v2 += 1;
        v3 += 1;
        v4 += 1;
        v5 += 1.0f;
        v6 += 1.0;
        v7 += 1;
        v8 += 1;
        v9 += 1;
        v10 += 1;
    }
    
    /* Conditional branch based on checksum */
    if (checksum > 0) {
        printf("Checksum: %lld\n", (long long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return (checksum > 0) ? 0 : 1;
}
