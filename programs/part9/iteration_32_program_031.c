#include <stdio.h>
#include <stdint.h>

/* Strategy 1: Multi-Operand Builtin Functions */
static inline int64_t custom_10op(int64_t a, int64_t b, int64_t c, int64_t d,
                                  int64_t e, int64_t f, int64_t g, int64_t h,
                                  int64_t i, int64_t j) {
    int64_t result;
    /* 10-operand inline assembly */
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

static inline int64_t custom_11op(int64_t a, int64_t b, int64_t c, int64_t d,
                                  int64_t e, int64_t f, int64_t g, int64_t h,
                                  int64_t i, int64_t j, int64_t k) {
    int64_t result;
    /* 11-operand inline assembly */
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

/* Strategy 2: Vector Operations with Many Elements */
typedef int32_t v10si __attribute__((vector_size(40)));  /* 10 elements */
typedef int32_t v11si __attribute__((vector_size(44)));  /* 11 elements */

/* Strategy 3: Complex Expression Trees */
static inline int64_t complex_11expr(volatile int64_t a, volatile int64_t b,
                                     volatile int64_t c, volatile int64_t d,
                                     volatile int64_t e, volatile int64_t f,
                                     volatile int64_t g, volatile int64_t h,
                                     volatile int64_t i, volatile int64_t j,
                                     volatile int64_t k) {
    /* Complex expression using all 11 variables */
    return ((((((((((a + b) * c) - d) << (e & 3)) & f) | g) ^ h) + i) * j) / (k | 1));
}

/* Strategy 4: Intrinsic-Like Function with Many Arguments */
__attribute__((always_inline)) 
static inline double multi_type_op(int8_t a, int16_t b, int32_t c, int64_t d,
                                   uint8_t e, uint16_t f, uint32_t g, uint64_t h,
                                   float i, double j, long double k) {
    /* Mixed-type operation that might trigger type-specific expansions */
    return (double)a + (double)b + (double)c + (double)d +
           (double)e + (double)f + (double)g + (double)h +
           (double)i + j + (double)k;
}

int main() {
    /* Initialize 11 volatile variables with distinct prime numbers */
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
    
    volatile int64_t checksum = 0;
    
    /* Initialize vectors */
    v10si vec10 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    v10si vec10_add = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    
    v11si vec11 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    v11si vec11_add = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. Use 10-operand inline assembly */
        int64_t asm10_result = custom_10op(v0, v1, v2, v3, v4, 
                                           v5, v6, v7, v8, v9);
        checksum ^= asm10_result;
        
        /* 2. Use 11-operand inline assembly */
        int64_t asm11_result = custom_11op(v0, v1, v2, v3, v4,
                                           v5, v6, v7, v8, v9, v10);
        checksum += asm11_result;
        
        /* 3. Complex 11-operand expression */
        int64_t expr_result = complex_11expr(v0, v1, v2, v3, v4,
                                             v5, v6, v7, v8, v9, v10);
        checksum *= (expr_result | 1);
        
        /* 4. Vector operations */
        v10si vec10_result = vec10 + vec10_add;
        v11si vec11_result = vec11 + vec11_add;
        
        /* Extract results from vectors */
        int32_t vec_sum = 0;
        for (int i = 0; i < 10; i++) {
            vec_sum += vec10_result[i];
        }
        for (int i = 0; i < 11; i++) {
            vec_sum += vec11_result[i];
        }
        checksum += vec_sum;
        
        /* 5. Mixed-type operation */
        double mixed_result = multi_type_op(
            (int8_t)(v0 & 0xFF), (int16_t)(v1 & 0xFFFF),
            (int32_t)(v2 & 0xFFFFFFFF), v3,
            (uint8_t)(v4 & 0xFF), (uint16_t)(v5 & 0xFFFF),
            (uint32_t)(v6 & 0xFFFFFFFF), (uint64_t)v7,
            (float)v8, (double)v9, (long double)v10
        );
        checksum += (int64_t)mixed_result;
        
        /* Modify variables slightly each iteration */
        v0 += 1; v1 += 2; v2 += 3; v3 += 4; v4 += 5;
        v5 += 6; v6 += 7; v7 += 8; v8 += 9; v9 += 10; v10 += 11;
    }
    
    /* Prevent dead code elimination */
    if (checksum != 0) {
        printf("Final checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return (checksum & 1);
}
