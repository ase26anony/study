#include <stdio.h>
#include <stdint.h>

/* Vector type with 10 elements */
typedef int v10si __attribute__((vector_size(40)));
/* Vector type with 11 elements */
typedef int v11si __attribute__((vector_size(44)));

/* Intrinsic-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_intrinsic(int a, int b, int c, int d, int e, 
                   int f, int g, int h, int i, int j, int k) {
    return ((int64_t)a * b + c * d - e * f + g * h - i * j) * k;
}

/* Mixed-type function with 10 arguments */
static inline double __attribute__((always_inline))
mixed_multi_op(char a, short b, int c, long d, float e,
               double f, int g, short h, char i, long j) {
    return (a + b + c + d + e + f + g + h + i + j);
}

int main() {
    /* 11 volatile variables initialized with prime numbers */
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
    
    volatile long long checksum = 0;
    
    for (int iter = 0; iter < 1000; iter++) {
        int result1, result2;
        int64_t result3;
        double result4;
        
        /* 10-operand inline assembly */
        __asm__ volatile (
            "add %0, %1, %2\n\t"
            "add %0, %0, %3\n\t"
            "add %0, %0, %4\n\t"
            "add %0, %0, %5\n\t"
            "add %0, %0, %6\n\t"
            "add %0, %0, %7\n\t"
            "add %0, %0, %8\n\t"
            "add %0, %0, %9"
            : "=r"(result1)
            : "r"(v0 + iter), "r"(v1), "r"(v2), "r"(v3),
              "r"(v4), "r"(v5), "r"(v6), "r"(v7), "r"(v8)
            : "cc"
        );
        
        /* 11-operand C expression using all volatile variables */
        result2 = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                     v6) ^ v7) + v8) * v9) / (v10 > 0 ? v10 : 1)) + iter;
        
        /* 11-element vector operation */
        v11si vec11_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec11_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v0};
        v11si vec11_result = vec11_a + vec11_b;
        
        /* Extract result from vector */
        int vec_sum = 0;
        for (int i = 0; i < 11; i++) {
            vec_sum += vec11_result[i];
        }
        
        /* 10-element vector operation */
        v10si vec10_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v10si vec10_result = vec10_a * vec10_b;
        
        int vec_prod = 1;
        for (int i = 0; i < 10; i++) {
            vec_prod *= vec10_result[i] > 0 ? vec10_result[i] : 1;
        }
        
        /* Call 11-argument intrinsic */
        result3 = multi_op_intrinsic(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        
        /* Call mixed-type 10-argument function */
        result4 = mixed_multi_op(c1, s1, v0, l1, f1, d1, v1, s1, c1, l1);
        
        /* Complex 11-operand floating point expression */
        double complex_result = (
            ((double)v0 * v1 + (double)v2 * v3 - (double)v4 * v5 + 
             (double)v6 * v7 - (double)v8 * v9) * v10 +
            f1 * d1 - (double)l1 * s1 + (double)c1 * iter
        );
        
        /* Update checksum with all results */
        checksum += result1 + result2 + vec_sum + vec_prod + 
                   result3 + (int64_t)result4 + (int64_t)complex_result;
        
        /* Modify variables slightly each iteration */
        v0 = (v0 * 3 + 1) & 0xFF;
        v1 = (v1 * 5 + 1) & 0xFF;
        v2 = (v2 * 7 + 1) & 0xFF;
    }
    
    /* Prevent dead code elimination */
    if (checksum != 0) {
        printf("Checksum: %lld\n", checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    /* Additional complex expression tree outside loop */
    volatile int a = 59, b = 61, c = 67, d = 71, e = 73;
    volatile int f = 79, g = 83, h = 89, i = 97, j = 101, k = 103;
    
    int final_expr = (((((((((a + b) * c) - d) << (e & 3)) & f) | g) ^ h) + i) * j) / k;
    printf("Final expression result: %d\n", final_expr);
    
    /* 11-operand inline assembly with mixed types */
    long long big_result;
    __asm__ volatile (
        "imul %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "sub %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "imul %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "sub %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "imul %0, %0, %10"
        : "=r"(big_result)
        : "r"((long long)a), "r"((long long)b), "r"((long long)c),
          "r"((long long)d), "r"((long long)e), "r"((long long)f),
          "r"((long long)g), "r"((long long)h), "r"((long long)i),
          "r"((long long)j)
        : "cc"
    );
    
    printf("Big result: %lld\n", big_result);
    
    return checksum > 0 ? 0 : 1;
}
