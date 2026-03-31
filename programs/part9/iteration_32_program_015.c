#include <stdio.h>
#include <stdint.h>

/* Approach 1: Multi-Operand Builtin Functions */
#ifdef __GNUC__
/* Custom inline assembly with 10 operands */
static inline int64_t custom_op10(int64_t a, int64_t b, int64_t c, int64_t d,
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

/* Custom inline assembly with 11 operands */
static inline int64_t custom_op11(int64_t a, int64_t b, int64_t c, int64_t d,
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
#endif

/* Approach 2: Vector Operations with Many Elements */
typedef int32_t v10si __attribute__((vector_size(40)));
typedef int32_t v11si __attribute__((vector_size(44)));

/* Approach 3: Complex Expression Trees */
static inline int64_t complex_expr_10(volatile int64_t a, volatile int64_t b,
                                      volatile int64_t c, volatile int64_t d,
                                      volatile int64_t e, volatile int64_t f,
                                      volatile int64_t g, volatile int64_t h,
                                      volatile int64_t i, volatile int64_t j) {
    /* 10-operand complex expression */
    return (((((((((a + b) * c) - d) << (e & 3)) & f) | g) ^ h) + i) * j) >> 2;
}

static inline int64_t complex_expr_11(volatile int64_t a, volatile int64_t b,
                                      volatile int64_t c, volatile int64_t d,
                                      volatile int64_t e, volatile int64_t f,
                                      volatile int64_t g, volatile int64_t h,
                                      volatile int64_t i, volatile int64_t j,
                                      volatile int64_t k) {
    /* 11-operand complex expression */
    return ((((((((((a + b) * c) - d) << (e & 3)) & f) | g) ^ h) + i) * j) / (k | 1)) & 0xFFFF;
}

/* Approach 4: Intrinsic-Like Function with Many Arguments */
__attribute__((always_inline))
static inline int64_t multi_op_intrinsic(int64_t a, int64_t b, int64_t c,
                                         int64_t d, int64_t e, int64_t f,
                                         int64_t g, int64_t h, int64_t i,
                                         int64_t j, int64_t k) {
    /* Mix of operations to prevent optimization */
    return (a * b) + (c * d) - (e * f) + (g * h) - (i * j) + (k * 3);
}

/* Approach 5: Mixed-Type Operations */
static inline double mixed_type_op(volatile char a, volatile short b,
                                   volatile int c, volatile long d,
                                   volatile float e, volatile double f,
                                   volatile int8_t g, volatile int16_t h,
                                   volatile int32_t i, volatile int64_t j,
                                   volatile double k) {
    /* Mix of types to trigger different expansion paths */
    return ((double)a + (double)b + (double)c + (double)d + 
            (double)e + f + (double)g + (double)h + 
            (double)i + (double)j) * k;
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
    
    /* Mixed type variables */
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
    volatile double d2 = 79.0;
    
    /* Initialize vectors */
    v10si vec10 = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    v11si vec11 = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31};
    v10si vec10_add = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    v11si vec11_add = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    
    int64_t checksum = 0;
    
    /* Run 1000 iterations to ensure execution */
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int64_t asm_result_10 = custom_op10(v0, v1, v2, v3, v4, 
                                           v5, v6, v7, v8, v9);
        
        /* 2. 11-operand inline assembly */
        int64_t asm_result_11 = custom_op11(v0, v1, v2, v3, v4, v5,
                                           v6, v7, v8, v9, v10);
        
        /* 3. 10-operand complex expression */
        int64_t expr_result_10 = complex_expr_10(v0, v1, v2, v3, v4,
                                                v5, v6, v7, v8, v9);
        
        /* 4. 11-operand complex expression */
        int64_t expr_result_11 = complex_expr_11(v0, v1, v2, v3, v4, v5,
                                                v6, v7, v8, v9, v10);
        
        /* 5. 11-operand intrinsic-like function */
        int64_t intrinsic_result = multi_op_intrinsic(v0, v1, v2, v3, v4, v5,
                                                     v6, v7, v8, v9, v10);
        
        /* 6. Mixed-type 11-operand operation */
        double mixed_result = mixed_type_op(c1, s1, i1, l1, f1, d1,
                                           i8, i16, i32, i64, d2);
        
        /* 7. Vector operations (10 and 11 elements) */
        v10si vec_result_10 = vec10 + vec10_add;
        v11si vec_result_11 = vec11 + vec11_add;
        
        /* Extract results from vectors */
        int64_t vec_sum_10 = 0;
        for (int i = 0; i < 10; i++) {
            vec_sum_10 += vec_result_10[i];
        }
        
        int64_t vec_sum_11 = 0;
        for (int i = 0; i < 11; i++) {
            vec_sum_11 += vec_result_11[i];
        }
        
        /* Update checksum with all results */
        checksum += asm_result_10;
        checksum += asm_result_11;
        checksum += expr_result_10;
        checksum += expr_result_11;
        checksum += intrinsic_result;
        checksum += (int64_t)mixed_result;
        checksum += vec_sum_10;
        checksum += vec_sum_11;
        
        /* Modify variables slightly each iteration */
        v0 += 1;
        v1 += 2;
        v2 += 3;
        v3 += 4;
        v4 += 5;
        v5 += 6;
        v6 += 7;
        v7 += 8;
        v8 += 9;
        v9 += 10;
        v10 += 11;
        
        /* Update vectors */
        for (int i = 0; i < 10; i++) {
            vec10[i] += 1;
            vec10_add[i] += 1;
        }
        for (int i = 0; i < 11; i++) {
            vec11[i] += 1;
            vec11_add[i] += 1;
        }
    }
    
    /* Conditional branch depending on checksum */
    if (checksum > 0) {
        printf("Checksum: %ld\n", checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return (checksum > 0) ? 0 : 1;
}
