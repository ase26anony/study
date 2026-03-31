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
typedef int32_t v10si __attribute__((vector_size(40)));  /* 10 ints */
typedef int32_t v11si __attribute__((vector_size(44)));  /* 11 ints */

/* Strategy 3: Complex Expression Trees */
static inline int64_t complex_11op_expr(
    volatile int64_t a, volatile int64_t b, volatile int64_t c,
    volatile int64_t d, volatile int64_t e, volatile int64_t f,
    volatile int64_t g, volatile int64_t h, volatile int64_t i,
    volatile int64_t j, volatile int64_t k) {
    /* Complex expression that uses all 11 operands in one chain */
    return ((((((((((a + b) * c) - d) << (e & 0x3F)) & f) | 
                g) ^ h) + i) * j) / (k ? k : 1));
}

/* Strategy 4: Intrinsic-Like Function with Many Arguments */
__attribute__((always_inline)) 
static inline double mixed_10op(
    char c1, short s1, int i1, long l1, 
    float f1, double d1, char c2, short s2, int i2, long l2) {
    /* Mixed-type operation that might trigger type-specific expansions */
    return (double)(c1 + s1 + i1 + l1) + f1 + d1 + (double)(c2 + s2 + i2 + l2);
}

__attribute__((always_inline)) 
static inline double mixed_11op(
    char c1, short s1, int i1, long l1, float f1, 
    double d1, char c2, short s2, int i2, long l2, float f2) {
    return (double)(c1 + s1 + i1 + l1) + f1 + d1 + 
           (double)(c2 + s2 + i2 + l2) + f2;
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
    
    /* Mixed-type variables for Strategy 5 */
    volatile char c1 = 2, c2 = 3;
    volatile short s1 = 5, s2 = 7;
    volatile int i1 = 11, i2 = 13;
    volatile long l1 = 17, l2 = 19;
    volatile float f1 = 23.0f, f2 = 29.0f;
    volatile double d1 = 31.0;
    
    int64_t checksum = 0;
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++) {
        /* Strategy 1: 10-operand builtin */
        int64_t res1 = custom_10op(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9);
        
        /* Strategy 1: 11-operand builtin */
        int64_t res2 = custom_11op(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        
        /* Strategy 3: Complex 11-operand expression */
        int64_t res3 = complex_11op_expr(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        
        /* Strategy 4 & 5: Mixed-type functions */
        double res4 = mixed_10op(c1, s1, i1, l1, f1, d1, c2, s2, i2, l2);
        double res5 = mixed_11op(c1, s1, i1, l1, f1, d1, c2, s2, i2, l2, f2);
        
        /* Strategy 2: Vector operations */
        v10si vec10a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v10si vec10c = vec10a + vec10b;
        
        v11si vec11a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec11b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v0};
        v11si vec11c = vec11a + vec11b;
        
        /* Extract results from vectors */
        int64_t vec_sum = 0;
        for (int i = 0; i < 10; i++) vec_sum += vec10c[i];
        for (int i = 0; i < 11; i++) vec_sum += vec11c[i];
        
        /* Update checksum to prevent dead code elimination */
        checksum += res1 + res2 + res3 + (int64_t)res4 + (int64_t)res5 + vec_sum;
        
        /* Modify variables slightly each iteration */
        v0 += 1; v1 += 2; v2 += 3; v3 += 4; v4 += 5;
        v5 += 6; v6 += 7; v7 += 8; v8 += 9; v9 += 10; v10 += 11;
    }
    
    /* Conditional branch depending on checksum */
    if (checksum > 0) {
        printf("Checksum: %ld\n", checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return 0;
}
