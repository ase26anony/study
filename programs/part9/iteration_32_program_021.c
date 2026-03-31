#include <stdio.h>
#include <stdint.h>

/* Define 10 and 11 operand inline assembly operations */
static inline int64_t custom_op10(int64_t a, int64_t b, int64_t c, int64_t d,
                                  int64_t e, int64_t f, int64_t g, int64_t h,
                                  int64_t i, int64_t j) {
    int64_t result;
    __asm__ volatile (
        "/* 10-operand custom operation */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    return result;
}

static inline int64_t custom_op11(int64_t a, int64_t b, int64_t c, int64_t d,
                                  int64_t e, int64_t f, int64_t g, int64_t h,
                                  int64_t i, int64_t j, int64_t k) {
    int64_t result;
    __asm__ volatile (
        "/* 11-operand custom operation */\n\t"
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
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j), "r"(k)
        : "cc"
    );
    return result;
}

/* Define vector types with 10 and 11 elements */
typedef int32_t v10si __attribute__((vector_size(40)));
typedef int32_t v11si __attribute__((vector_size(44)));

/* 11-argument always_inline function */
static inline int64_t __attribute__((always_inline))
multi_op_inline(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e,
                int64_t f, int64_t g, int64_t h, int64_t i, int64_t j,
                int64_t k) {
    /* Complex expression that uses all 11 operands */
    return ((((((((((a ^ b) + (c * d)) >> (e & 0x1F)) | f) & g) 
                ^ (h << (i & 0x1F))) + j) * k) - a) + b) ^ c;
}

/* Mixed-type 10-operand function */
static inline double __attribute__((always_inline))
mixed_type_op10(int8_t a, int16_t b, int32_t c, int64_t d,
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
    
    /* Initialize vectors */
    v10si vec10 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    v11si vec11 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    v10si vec10_add = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    v11si vec11_add = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    
    int64_t checksum = 0;
    
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int64_t asm_result = custom_op10(v0 + iter, v1, v2, v3, v4,
                                         v5, v6, v7, v8, v9);
        
        /* 2. 11-operand inline assembly */
        int64_t asm_result11 = custom_op11(v0, v1, v2, v3, v4, v5,
                                           v6, v7, v8, v9, v10 + iter);
        
        /* 3. Complex 11-operand expression */
        int64_t expr_result = ((((((((((v0 + v1) * v2) - v3) << (v4 & 0x3)) & v5) 
                                  | v6) ^ v7) + v8) * v9) / (v10 + 1)) + iter;
        
        /* 4. 11-operand always_inline function */
        int64_t inline_result = multi_op_inline(v0, v1, v2, v3, v4, v5,
                                                v6, v7, v8, v9, v10);
        
        /* 5. Mixed-type 10-operand function */
        double mixed_result = mixed_type_op10(mv0, mv1, mv2, mv3, mv4,
                                              mv5, mv6, mv7, mv8, mv9);
        
        /* 6. Vector operations */
        v10si vec_result10 = vec10 + vec10_add;
        v11si vec_result11 = vec11 + vec11_add;
        
        /* Extract results from vectors */
        int64_t vec_sum = 0;
        for (int i = 0; i < 10; i++) {
            vec_sum += vec_result10[i];
        }
        for (int i = 0; i < 11; i++) {
            vec_sum += vec_result11[i];
        }
        
        /* Update checksum with all results */
        checksum ^= asm_result;
        checksum += asm_result11;
        checksum ^= expr_result;
        checksum += inline_result;
        checksum ^= (int64_t)mixed_result;
        checksum += vec_sum;
        
        /* Modify variables to prevent optimization */
        v0 ^= iter;
        v1 += iter;
        vec10_add[iter % 10] = iter;
        vec11_add[iter % 11] = iter;
    }
    
    /* Conditional branch based on checksum */
    if (checksum != 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    /* Additional complex expression using all 11 variables */
    volatile int64_t final_expr = 
        ((((((((((v0 & v1) | (v2 ^ v3)) + (v4 * v5)) - (v6 / (v7 + 1))) 
              << (v8 & 0x7)) & v9) | v10) + v0) - v1) * v2) & v3;
    
    printf("Final expression: %ld\n", (long)final_expr);
    
    return (checksum & 1);
}
