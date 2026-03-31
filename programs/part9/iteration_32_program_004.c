#include <stdio.h>
#include <stdint.h>

/* Custom inline assembly functions with 10 and 11 operands */
static inline int64_t custom_10op(int64_t a, int64_t b, int64_t c, int64_t d,
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
    );
    return result + j; /* 10th operand handled outside asm */
}

static inline int64_t custom_11op(int64_t a, int64_t b, int64_t c, int64_t d,
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
    );
    return result + k; /* 11th operand handled outside asm */
}

/* 11-element vector type */
typedef int64_t v11si __attribute__((vector_size(88)));

/* Intrinsic-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
multi_op_intrinsic(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e,
                   int64_t f, int64_t g, int64_t h, int64_t i, int64_t j,
                   int64_t k) {
    return ((((((((((a * b) + c) - d) ^ e) | f) & g) << (h & 3)) >> (i & 3)) 
             * j) / (k ? k : 1));
}

/* Mixed-type function with 10 arguments */
static inline double __attribute__((always_inline))
mixed_type_op(int8_t a, int16_t b, int32_t c, int64_t d,
              float e, double f, uint8_t g, uint16_t h,
              uint32_t i, uint64_t j) {
    return (double)a + (double)b + (double)c + (double)d +
           (double)e + f + (double)g + (double)h +
           (double)i + (double)j;
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
    
    /* Additional mixed-type variables */
    volatile int8_t c1 = 37;
    volatile int16_t s1 = 41;
    volatile int32_t i1 = 43;
    volatile float f1 = 47.0f;
    volatile double d1 = 53.0;
    volatile uint8_t uc1 = 59;
    volatile uint16_t us1 = 61;
    volatile uint32_t ui1 = 67;
    volatile uint64_t ul1 = 71;
    
    int64_t checksum = 0;
    
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int64_t asm_result = custom_10op(v0 + iter, v1, v2, v3, v4, v5, v6, v7, v8, v9);
        checksum ^= asm_result;
        
        /* 2. 11-operand complex expression */
        int64_t expr_result = (((((((((v0 * v1) + v2) - v3) ^ v4) | v5) & v6) 
                               << (v7 & 3)) >> (v8 & 3)) * v9) / (v10 ? v10 : 1);
        checksum += expr_result;
        
        /* 3. 11-element vector operation */
        v11si vec1 = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec2 = {v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0};
        v11si vec_sum = vec1 + vec2;
        
        /* Extract and use all vector elements */
        for (int i = 0; i < 11; i++) {
            checksum += vec_sum[i];
        }
        
        /* 4. 11-operand intrinsic-like function */
        int64_t intrinsic_result = multi_op_intrinsic(
            v0 + iter, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum *= (intrinsic_result + 1);
        
        /* 5. Mixed-type operation with 10 operands */
        double mixed_result = mixed_type_op(
            c1, s1, i1, v0, f1, d1, uc1, us1, ui1, ul1);
        checksum += (int64_t)mixed_result;
        
        /* 6. Direct 11-operand inline assembly attempt */
        int64_t direct_asm_result;
        __asm__ volatile (
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
            : "=r"(direct_asm_result)
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
              "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10)
        );
        checksum ^= direct_asm_result;
    }
    
    /* Prevent dead code elimination */
    if (checksum != 0) {
        printf("Result checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return (checksum & 1);
}
