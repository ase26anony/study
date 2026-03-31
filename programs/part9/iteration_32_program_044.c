#include <stdio.h>
#include <stdint.h>

/* Strategy 1: Multi-operand inline assembly */
static inline int64_t ten_operand_asm(int64_t a, int64_t b, int64_t c, int64_t d,
                                      int64_t e, int64_t f, int64_t g, int64_t h,
                                      int64_t i, int64_t j) {
    int64_t result;
    /* 10-input, 1-output inline assembly */
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
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    return result;
}

static inline int64_t eleven_operand_asm(int64_t a, int64_t b, int64_t c, int64_t d,
                                         int64_t e, int64_t f, int64_t g, int64_t h,
                                         int64_t i, int64_t j, int64_t k) {
    int64_t result;
    /* 11-input, 1-output inline assembly */
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
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j), "r" (k)
        : "cc"
    );
    return result;
}

/* Strategy 2: Vector operations with many elements */
typedef int32_t v10si __attribute__((vector_size(10 * sizeof(int32_t))));
typedef int32_t v11si __attribute__((vector_size(11 * sizeof(int32_t))));

/* Strategy 4: Intrinsic-like function with many arguments */
__attribute__((always_inline))
static inline int64_t eleven_arg_function(int8_t a, int16_t b, int32_t c, int64_t d,
                                          float e, double f, uint8_t g, uint16_t h,
                                          uint32_t i, uint64_t j, intptr_t k) {
    /* Mixed-type operations (Strategy 5) */
    return (int64_t)a + (int64_t)b + (int64_t)c + d + 
           (int64_t)e + (int64_t)f + (int64_t)g + (int64_t)h + 
           (int64_t)i + (int64_t)j + (int64_t)k;
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
    
    /* Additional volatile variables for mixed types */
    volatile int8_t  c1 = 37;
    volatile int16_t s1 = 41;
    volatile int32_t i1 = 43;
    volatile float   f1 = 47.0f;
    volatile double  d1 = 53.0;
    volatile uint8_t uc1 = 59;
    volatile uint16_t us1 = 61;
    volatile uint32_t ui1 = 67;
    volatile uint64_t ul1 = 71;
    volatile intptr_t p1 = 73;
    
    int64_t checksum = 0;
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++) {
        /* a. 10-operand inline assembly */
        int64_t asm_result_10 = ten_operand_asm(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9);
        checksum ^= asm_result_10;
        
        /* b. 11-operand C expression (Strategy 3) */
        /* Complex expression tree with 11 operands */
        int64_t expr_result = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                                 v6) ^ v7) + v8) * v9) / (v10 & 0xFF)) + 
                             ((v0 ^ v1) | (v2 & v3));
        checksum += expr_result;
        
        /* c. 11-operand inline assembly */
        int64_t asm_result_11 = eleven_operand_asm(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v0);
        checksum ^= asm_result_11;
        
        /* d. Vector operations (Strategy 2) */
        v11si vec_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec_b = {v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0};
        v11si vec_result = vec_a + vec_b;
        
        /* Extract all elements to force scalarization */
        for (int i = 0; i < 11; i++) {
            checksum += vec_result[i];
        }
        
        /* e. 11-argument function call (Strategy 4) with mixed types (Strategy 5) */
        int64_t func_result = eleven_arg_function(c1, s1, i1, v0, f1, d1, 
                                                 uc1, us1, ui1, ul1, p1);
        checksum += func_result;
        
        /* Modify volatile variables to prevent constant propagation */
        v0 ^= iter;
        v1 += iter;
        v2 -= iter;
        v3 |= iter;
        v4 &= ~iter;
    }
    
    /* Strategy 5: Conditional branch depending on checksum */
    if (checksum != 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return (checksum & 1);
}
