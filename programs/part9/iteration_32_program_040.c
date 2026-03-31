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

/* Strategy 2: Vector operations with many elements */
typedef int32_t v10si __attribute__((vector_size(40)));  /* 10 elements */
typedef int32_t v11si __attribute__((vector_size(44)));  /* 11 elements */

/* Strategy 3: Complex expression tree with 11 operands */
#define ELEVEN_OPERAND_EXPR(a,b,c,d,e,f,g,h,i,j,k) \
    ((((((((((a + b) * c) - d) << (e & 3)) & f) | g) ^ h) + i) * j) / (k ? k : 1))

/* Strategy 4: Intrinsic-like function with 11 arguments */
static inline int64_t __attribute__((always_inline))
eleven_arg_intrinsic(int64_t a1, int64_t a2, int64_t a3, int64_t a4, int64_t a5,
                     int64_t a6, int64_t a7, int64_t a8, int64_t a9, int64_t a10,
                     int64_t a11) {
    /* Complex operation that can't be easily decomposed */
    return ((a1 * a2) + (a3 << 2) - (a4 >> 1) + (a5 & a6) | 
            (a7 ^ a8) + (a9 * a10) - (a11 << 1));
}

/* Strategy 5: Mixed-type operations */
static inline double mixed_type_10op(int8_t a, int16_t b, int32_t c, int64_t d,
                                     float e, double f, uint8_t g, uint16_t h,
                                     uint32_t i, uint64_t j) {
    /* Mixed-type expression forcing conversions */
    return (double)a + (double)b + (double)c + (double)d + 
           (double)e + f + (double)g + (double)h + (double)i + (double)j;
}

int main() {
    /* Initialize 11 volatile variables with distinct primes */
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
    
    /* Mixed-type variables */
    volatile int8_t  m1 = 37;
    volatile int16_t m2 = 41;
    volatile int32_t m3 = 43;
    volatile float   m4 = 47.0f;
    volatile double  m5 = 53.0;
    volatile uint8_t m6 = 59;
    volatile uint16_t m7 = 61;
    volatile uint32_t m8 = 67;
    volatile uint64_t m9 = 71;
    
    int64_t checksum = 0;
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int64_t asm_result = ten_operand_asm(
            v0 + iter, v1, v2, v3, v4, v5, v6, v7, v8, v9
        );
        checksum ^= asm_result;
        
        /* 2. 11-operand C expression using volatile variables */
        int64_t expr_result = ELEVEN_OPERAND_EXPR(
            v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10
        );
        checksum += expr_result;
        
        /* 3. 11-element vector operation */
        v11si vec_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec_b = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        v11si vec_result = vec_a + vec_b;
        
        /* Extract and use vector elements */
        for (int i = 0; i < 11; i++) {
            checksum += vec_result[i];
        }
        
        /* 4. 11-argument intrinsic call */
        int64_t intrinsic_result = eleven_arg_intrinsic(
            v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10
        );
        checksum *= (intrinsic_result + 1);
        
        /* 5. Mixed-type 10-operand function */
        double mixed_result = mixed_type_10op(
            m1, m2, m3, v0, m4, m5, m6, m7, m8, m9
        );
        checksum += (int64_t)mixed_result;
        
        /* Additional: 11-operand inline assembly variant */
        int64_t asm11_result;
        __asm__ volatile (
            "mov %0, #0\n\t"
            "add %0, %0, %1\n\t"
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
            : "=r" (asm11_result)
            : "r" (v0), "r" (v1), "r" (v2), "r" (v3),
              "r" (v4), "r" (v5), "r" (v6), "r" (v7),
              "r" (v8), "r" (v9), "r" (v10)
            : "cc"
        );
        checksum ^= asm11_result;
    }
    
    /* Prevent dead code elimination */
    if (checksum != 0) {
        printf("Final checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return (checksum & 1);  /* Return 0 or 1 based on checksum */
}
