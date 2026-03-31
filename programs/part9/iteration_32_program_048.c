#include <stdio.h>
#include <stdint.h>

/* Strategy 1: Multi-operand inline assembly */
static inline int64_t asm_10_op(int64_t a, int64_t b, int64_t c, int64_t d,
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

static inline int64_t asm_11_op(int64_t a, int64_t b, int64_t c, int64_t d,
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

/* Strategy 2: Vector operations with many elements */
typedef int32_t v10si __attribute__((vector_size(40)));  /* 10 ints */
typedef int32_t v11si __attribute__((vector_size(44)));  /* 11 ints */

/* Strategy 4: Intrinsic-like function with many arguments */
__attribute__((always_inline))
static inline int64_t multi_op_11(int64_t a, int64_t b, int64_t c, int64_t d,
                                  int64_t e, int64_t f, int64_t g, int64_t h,
                                  int64_t i, int64_t j, int64_t k) {
    /* Complex expression that uses all 11 operands */
    return ((((((((((a * b) + c) - d) ^ e) | f) & g) << (h & 3)) >> (i & 3)) 
             * j) / (k ? k : 1)) + (a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j ^ k);
}

/* Strategy 5: Mixed-type operations */
__attribute__((always_inline))
static inline double mixed_types_10(int8_t a, int16_t b, int32_t c, int64_t d,
                                    float e, double f, uint8_t g, uint16_t h,
                                    uint32_t i, uint64_t j) {
    return (double)a + (double)b + (double)c + (double)d + 
           (double)e + f + (double)g + (double)h + (double)i + (double)j;
}

int main() {
    /* Strategy 1: Declare 11 volatile variables with prime numbers */
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
    
    /* Mixed-type variables for Strategy 5 */
    volatile int8_t   m1 = 2;
    volatile int16_t  m2 = 3;
    volatile int32_t  m3 = 5;
    volatile int64_t  m4 = 7;
    volatile float    m5 = 11.0f;
    volatile double   m6 = 13.0;
    volatile uint8_t  m7 = 17;
    volatile uint16_t m8 = 19;
    volatile uint32_t m9 = 23;
    volatile uint64_t m10 = 29;
    
    int64_t checksum = 0;
    
    /* Run 1000 iterations to ensure execution */
    for (int iter = 0; iter < 1000; iter++) {
        /* a) 10-operand inline assembly */
        int64_t asm_result_10 = asm_10_op(v0, v1, v2, v3, v4, 
                                          v5, v6, v7, v8, v9);
        
        /* b) 11-operand inline assembly */
        int64_t asm_result_11 = asm_11_op(v0, v1, v2, v3, v4,
                                          v5, v6, v7, v8, v9, v10);
        
        /* Strategy 3: Complex 11-operand expression tree */
        int64_t expr_result = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                                 v6) ^ v7) + v8) * v9) / (v10 ? v10 : 1)) % 1000;
        
        /* Strategy 4: Intrinsic-like function with 11 arguments */
        int64_t func_result = multi_op_11(v0, v1, v2, v3, v4,
                                          v5, v6, v7, v8, v9, v10);
        
        /* Strategy 5: Mixed-type operations */
        double mixed_result = mixed_types_10(m1, m2, m3, m4, m5,
                                             m6, m7, m8, m9, m10);
        
        /* Strategy 2: Vector operations */
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
        
        /* Update checksum to prevent dead code elimination */
        checksum += asm_result_10 + asm_result_11 + expr_result + 
                   func_result + (int64_t)mixed_result + vec_sum;
        
        /* Modify volatile variables slightly to prevent constant propagation */
        v0 += 1; v1 += 2; v2 += 3; v3 += 4; v4 += 5;
        v5 += 6; v6 += 7; v7 += 8; v8 += 9; v9 += 10; v10 += 11;
        
        m1 += 1; m2 += 2; m3 += 3; m4 += 4; m5 += 1.0f;
        m6 += 2.0; m7 += 3; m8 += 4; m9 += 5; m10 += 6;
    }
    
    /* Conditional branch depending on checksum */
    if (checksum > 0) {
        printf("Checksum: %ld\n", checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return 0;
}
