#include <stdio.h>
#include <stdint.h>

/* Approach 1: Multi-Operand Builtin Functions */
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

/* Approach 2: Vector Operations with Many Elements */
typedef int32_t v10si __attribute__((vector_size(40)));  /* 10 elements */
typedef int32_t v11si __attribute__((vector_size(44)));  /* 11 elements */

/* Approach 3: Complex Expression Trees */
static inline int64_t complex_11op_expr(volatile int64_t a, volatile int64_t b,
                                        volatile int64_t c, volatile int64_t d,
                                        volatile int64_t e, volatile int64_t f,
                                        volatile int64_t g, volatile int64_t h,
                                        volatile int64_t i, volatile int64_t j,
                                        volatile int64_t k) 
    __attribute__((always_inline));

static inline int64_t complex_11op_expr(volatile int64_t a, volatile int64_t b,
                                        volatile int64_t c, volatile int64_t d,
                                        volatile int64_t e, volatile int64_t f,
                                        volatile int64_t g, volatile int64_t h,
                                        volatile int64_t i, volatile int64_t j,
                                        volatile int64_t k) {
    /* Complex 11-operand expression that cannot be easily broken up */
    return ((((((((((a + b) * c) - d) << (e & 0x3F)) & f) | 
               g) ^ h) + i) * j) / (k ? k : 1));
}

/* Approach 4: Intrinsic-Like Function with Many Arguments */
static inline double mixed_10op(volatile char a, volatile short b, 
                                volatile int c, volatile long d,
                                volatile float e, volatile double f,
                                volatile int8_t g, volatile int16_t h,
                                volatile int32_t i, volatile int64_t j)
    __attribute__((always_inline));

static inline double mixed_10op(volatile char a, volatile short b,
                                volatile int c, volatile long d,
                                volatile float e, volatile double f,
                                volatile int8_t g, volatile int16_t h,
                                volatile int32_t i, volatile int64_t j) {
    /* Mixed-type operation that might trigger type-specific expansions */
    return (double)a + (double)b + (double)c + (double)d + 
           (double)e + f + (double)g + (double)h + (double)i + (double)j;
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
    
    /* Mixed-type variables */
    volatile char c1 = 37;
    volatile short s1 = 41;
    volatile int i1 = 43;
    volatile long l1 = 47;
    volatile float f1 = 53.0f;
    volatile double d1 = 59.0;
    volatile int8_t i8 = 61;
    volatile int16_t i16 = 67;
    volatile int32_t i32 = 71;
    volatile int64_t i64 = 73;
    
    /* Initialize vectors */
    v10si vec10 = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    v10si vec10_add = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    v11si vec11 = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31};
    v11si vec11_add = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    
    int64_t checksum = 0;
    
    /* Run 1000 iterations to ensure execution */
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. Use 10-operand inline assembly */
        int64_t asm_result = custom_10op(v0, v1, v2, v3, v4, 
                                         v5, v6, v7, v8, v9);
        checksum ^= asm_result;
        
        /* 2. Use 11-operand inline assembly */
        int64_t asm11_result = custom_11op(v0, v1, v2, v3, v4,
                                          v5, v6, v7, v8, v9, v10);
        checksum += asm11_result;
        
        /* 3. Complex 11-operand C expression */
        int64_t expr_result = complex_11op_expr(v0, v1, v2, v3, v4,
                                               v5, v6, v7, v8, v9, v10);
        checksum *= (expr_result + 1);
        
        /* 4. Mixed-type 10-operand function */
        double mixed_result = mixed_10op(c1, s1, i1, l1, f1,
                                        d1, i8, i16, i32, i64);
        checksum += (int64_t)mixed_result;
        
        /* 5. Vector operations */
        v10si vec_result10 = vec10 + vec10_add;
        v11si vec_result11 = vec11 + vec11_add;
        
        /* Extract results from vectors */
        for (int i = 0; i < 10; i++) {
            checksum += vec_result10[i];
        }
        for (int i = 0; i < 11; i++) {
            checksum += vec_result11[i];
        }
        
        /* Modify variables slightly to prevent constant propagation */
        v0 += 1; v1 += 2; v2 += 3; v3 += 4; v4 += 5;
        v5 += 6; v6 += 7; v7 += 8; v8 += 9; v9 += 10; v10 += 11;
        
        /* Update vectors */
        for (int i = 0; i < 10; i++) {
            vec10[i] += i;
            vec10_add[i] += 1;
        }
        for (int i = 0; i < 11; i++) {
            vec11[i] += i;
            vec11_add[i] += 1;
        }
    }
    
    /* Conditional branch based on checksum */
    if (checksum > 0) {
        printf("Checksum (positive): %lld\n", (long long)checksum);
    } else if (checksum < 0) {
        printf("Checksum (negative): %lld\n", (long long)checksum);
    } else {
        printf("Checksum (zero): %lld\n", (long long)checksum);
    }
    
    return (checksum != 0) ? 0 : 1;
}
