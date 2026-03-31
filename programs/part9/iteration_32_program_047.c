#include <stdio.h>
#include <stdint.h>

/* Strategy 1: Multi-operand inline assembly */
static inline int64_t custom_op_10(int64_t a, int64_t b, int64_t c, int64_t d,
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
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    return result;
}

static inline int64_t custom_op_11(int64_t a, int64_t b, int64_t c, int64_t d,
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
        "add %0, %0, %10\n\t"
        "add %0, %0, %11"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f),
          "r"(g), "r"(h), "r"(i), "r"(j), "r"(k)
        : "cc"
    );
    return result;
}

/* Strategy 2: Vector operations */
typedef int32_t v10si __attribute__((vector_size(40)));
typedef int32_t v11si __attribute__((vector_size(44)));

/* Strategy 4: Intrinsic-like function with many arguments */
static inline int64_t __attribute__((always_inline))
multi_op_11(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e,
            int64_t f, int64_t g, int64_t h, int64_t i, int64_t j,
            int64_t k) {
    return ((((((((((a + b) * c) - d) << (e & 3)) & f) | g) ^ h) + i) * j) / (k ? k : 1);
}

/* Strategy 5: Mixed-type operations */
static inline double mixed_op_10(int8_t a, int16_t b, int32_t c, int64_t d,
                                 float e, double f, uint8_t g, uint16_t h,
                                 uint32_t i, uint64_t j) {
    return (double)a + (double)b + (double)c + (double)d + 
           (double)e + f + (double)g + (double)h + 
           (double)i + (double)j;
}

int main() {
    /* Strategy 1: 11 volatile variables with prime numbers */
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
        /* 1a: 10-operand inline assembly */
        int64_t asm_result = custom_op_10(v0 + iter, v1, v2, v3, v4, 
                                          v5, v6, v7, v8, v9);
        
        /* 1b: 11-operand inline assembly */
        int64_t asm_result_11 = custom_op_11(v0, v1, v2, v3, v4, v5,
                                             v6, v7, v8, v9, v10 + iter);
        
        /* 2: 11-operand C expression (Strategy 3) */
        int64_t expr_result = ((((((((((v0 + iter) + v1) * v2) - v3) 
                                 << (v4 & 3)) & v5) | v6) ^ v7) + v8) * v9) / (v10 ? v10 : 1);
        
        /* 3: Vector operations (Strategy 2) */
        v11si vec_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec_b = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        v11si vec_result = vec_a + vec_b + iter;
        
        /* Extract vector result */
        int64_t vec_sum = 0;
        for (int i = 0; i < 11; i++) {
            vec_sum += vec_result[i];
        }
        
        /* 4: Intrinsic-like function call */
        int64_t func_result = multi_op_11(v0, v1, v2, v3, v4, v5,
                                          v6, v7, v8, v9, v10 + iter);
        
        /* 5: Mixed-type operation */
        double mixed_result = mixed_op_10(c1, s1, i1, v0, f1, d1,
                                          uc1, us1, ui1, ul1);
        
        /* Update checksum with all results */
        checksum += asm_result + asm_result_11 + expr_result + 
                   vec_sum + func_result + (int64_t)mixed_result;
        
        /* Modify variables slightly to prevent optimization */
        v0 += 1;
        v1 -= 1;
        v2 ^= iter;
    }
    
    /* Strategy 5: Conditional branch based on checksum */
    if (checksum > 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    /* Additional complex expression to ensure RTL expansion */
    volatile int64_t final_expr = 
        ((((((((((v0 * v1) + v2) - v3) | v4) & v5) ^ v6) << (v7 & 7)) + v8) * v9) / v10) +
        ((((((((((v10 * v9) + v8) - v7) | v6) & v5) ^ v4) << (v3 & 7)) + v2) * v1) / v0);
    
    printf("Final expression: %ld\n", (long)final_expr);
    
    return (checksum > 1000000) ? 0 : 1;
}
