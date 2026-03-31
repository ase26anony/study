#include <stdio.h>
#include <stdint.h>

/* Custom inline assembly operations with 10 and 11 operands */
static inline int64_t custom_op10(int64_t a, int64_t b, int64_t c, int64_t d,
                                  int64_t e, int64_t f, int64_t g, int64_t h,
                                  int64_t i, int64_t j) {
    int64_t result;
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc"
    );
    return result + j; /* Total of 10 operands used */
}

static inline int64_t custom_op11(int64_t a, int64_t b, int64_t c, int64_t d,
                                  int64_t e, int64_t f, int64_t g, int64_t h,
                                  int64_t i, int64_t j, int64_t k) {
    int64_t result;
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
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    return result + k; /* Total of 11 operands used */
}

/* Vector types with 10 and 11 elements */
typedef int32_t v10si __attribute__((vector_size(40)));  /* 10 * 4 bytes */
typedef int32_t v11si __attribute__((vector_size(44)));  /* 11 * 4 bytes */

/* Intrinsic-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_intrinsic(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e,
                   int64_t f, int64_t g, int64_t h, int64_t i, int64_t j,
                   int64_t k) {
    return ((((((((((a ^ b) + c) * d) - e) & f) | g) ^ h) + i) * j) / (k ? k : 1));
}

/* Mixed-type operation function */
static inline double __attribute__((always_inline))
mixed_type_op(int8_t a, int16_t b, int32_t c, int64_t d,
              float e, double f, uint8_t g, uint16_t h,
              uint32_t i, uint64_t j, int32_t k) {
    return (double)a + (double)b + (double)c + (double)d +
           (double)e + f + (double)g + (double)h +
           (double)i + (double)j + (double)k;
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
    volatile int8_t mv0 = 2;
    volatile int16_t mv1 = 3;
    volatile int32_t mv2 = 5;
    volatile int64_t mv3 = 7;
    volatile float mv4 = 11.0f;
    volatile double mv5 = 13.0;
    volatile uint8_t mv6 = 17;
    volatile uint16_t mv7 = 19;
    volatile uint32_t mv8 = 23;
    volatile uint64_t mv9 = 29;
    volatile int32_t mv10 = 31;
    
    int64_t checksum = 0;
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int64_t asm_result = custom_op10(v0 + iter, v1, v2, v3, v4, v5, v6, v7, v8, v9);
        checksum ^= asm_result;
        
        /* 2. 11-operand C expression using all volatile variables */
        int64_t expr_result = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                                  v6) ^ v7) + v8) * v9) / (v10 ? v10 : 1)) + iter;
        checksum += expr_result;
        
        /* 3. 11-operand inline assembly */
        int64_t asm_result11 = custom_op11(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum |= asm_result11;
        
        /* 4. Vector operations with 10 and 11 elements */
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
            checksum ^= vec11_result[i];
        }
        
        /* 5. Intrinsic-like function with 11 arguments */
        int64_t intrinsic_result = multi_op_intrinsic(
            v0 + iter, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum *= (intrinsic_result + 1);
        
        /* 6. Mixed-type operation with 11 arguments */
        double mixed_result = mixed_type_op(
            mv0, mv1, mv2, mv3, mv4, mv5, mv6, mv7, mv8, mv9, mv10);
        checksum += (int64_t)mixed_result;
        
        /* Complex expression tree with 11 operands */
        int64_t complex_expr = 
            ((((((((((v0 * v1) + v2) - v3) & v4) | v5) ^ v6) + v7) * v8) / 
             (v9 ? v9 : 1)) << (v10 & 3)) + iter;
        checksum -= complex_expr;
    }
    
    /* Conditional branch to ensure all code paths are reachable */
    if (checksum != 0) {
        printf("Checksum: %ld\n", checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    /* Additional test with varying operand counts */
    {
        /* Force expansion with exactly 10 scalar operands */
        int64_t temp = v0;
        temp = temp + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        
        /* Force expansion with exactly 11 scalar operands */
        int64_t temp2 = v0;
        temp2 = temp2 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        
        printf("Additional results: %ld, %ld\n", temp, temp2);
    }
    
    return checksum != 0 ? 0 : 1;
}
