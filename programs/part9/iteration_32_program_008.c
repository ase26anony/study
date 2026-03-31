#include <stdio.h>
#include <stdint.h>

/* Strategy 1: Multi-Operand Builtin Functions */
static inline int64_t custom_10_op(int64_t a, int64_t b, int64_t c, int64_t d,
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

static inline int64_t custom_11_op(int64_t a, int64_t b, int64_t c, int64_t d,
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

/* Strategy 4: Intrinsic-Like Function with Many Arguments */
__attribute__((always_inline)) 
static inline int64_t multi_op_11(int64_t a, int64_t b, int64_t c, int64_t d,
                                  int64_t e, int64_t f, int64_t g, int64_t h,
                                  int64_t i, int64_t j, int64_t k) {
    /* Complex operation that uses all 11 arguments */
    return ((((((((((a + b) * c) - d) << (e & 3)) & f) | g) ^ h) + i) * j) / (k ? k : 1);
}

/* Strategy 5: Mixed-Type Operations */
static inline double mixed_type_op(int8_t a, int16_t b, int32_t c, int64_t d,
                                   float e, double f, uint8_t g, uint16_t h,
                                   uint32_t i, uint64_t j, long double k) {
    /* Mix of types to trigger different expansion paths */
    return (double)a + (double)b + (double)c + (double)d + 
           (double)e + f + (double)g + (double)h + 
           (double)i + (double)j + (double)k;
}

int main() {
    /* Strategy: Use volatile to prevent optimization */
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
    volatile long double mv10 = 31.0L;
    
    int64_t checksum = 0;
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int64_t result1 = custom_10_op(v0 + iter, v1, v2, v3, v4, 
                                       v5, v6, v7, v8, v9);
        
        /* 2. 11-operand inline assembly */
        int64_t result2 = custom_11_op(v0, v1, v2, v3, v4, v5, 
                                       v6, v7, v8, v9, v10 + iter);
        
        /* 3. Complex 11-operand expression (Strategy 3) */
        int64_t result3 = (((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                            v6) ^ v7) + v8) * v9) / (v10 ? v10 : 1);
        
        /* 4. 11-argument function call */
        int64_t result4 = multi_op_11(v0, v1, v2, v3, v4, v5, 
                                      v6, v7, v8, v9, v10);
        
        /* 5. Mixed-type operation */
        double result5 = mixed_type_op(mv0, mv1, mv2, mv3, mv4, mv5,
                                       mv6, mv7, mv8, mv9, mv10);
        
        /* 6. Vector operations (Strategy 2) */
        v10si vec10_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v10si vec10_result = vec10_a + vec10_b;
        
        v11si vec11_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec11_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v0};
        v11si vec11_result = vec11_a + vec11_b;
        
        /* Extract results from vectors */
        int64_t vec_sum = 0;
        for (int i = 0; i < 10; i++) {
            vec_sum += vec10_result[i];
        }
        for (int i = 0; i < 11; i++) {
            vec_sum += vec11_result[i];
        }
        
        /* Update checksum with all results */
        checksum += result1 + result2 + result3 + result4 + 
                   (int64_t)result5 + vec_sum;
        
        /* Modify volatile variables slightly */
        v0 += 1; v1 += 2; v2 += 3; v3 += 4; v4 += 5;
        v5 += 6; v6 += 7; v7 += 8; v8 += 9; v9 += 10; v10 += 11;
    }
    
    /* Conditional branch to ensure all code paths are reachable */
    if (checksum != 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return 0;
}
