#include <stdio.h>
#include <stdint.h>

/* Custom inline assembly functions with 10 and 11 operands */
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
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    return result;
}

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
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i), "r"(j), "r"(k)
        : "cc"
    );
    return result;
}

/* Vector types with 10 and 11 elements */
typedef int32_t v10si __attribute__((vector_size(40)));
typedef int32_t v11si __attribute__((vector_size(44)));

/* Complex expression with 11 operands */
static inline int64_t complex_expr11(volatile int64_t a, volatile int64_t b,
                                     volatile int64_t c, volatile int64_t d,
                                     volatile int64_t e, volatile int64_t f,
                                     volatile int64_t g, volatile int64_t h,
                                     volatile int64_t i, volatile int64_t j,
                                     volatile int64_t k) {
    return ((((((((((a + b) * c) - d) << (e & 3)) & f) | g) ^ h) + i) * j) / (k | 1));
}

/* Intrinsic-like function with 10 arguments */
static inline __attribute__((always_inline))
int64_t intrinsic_op10(volatile int8_t a, volatile int16_t b, volatile int32_t c,
                       volatile int64_t d, volatile uint8_t e, volatile uint16_t f,
                       volatile uint32_t g, volatile uint64_t h,
                       volatile float i, volatile double j) {
    return (int64_t)(a + b + c + d + e + f + g + h + (int64_t)i + (int64_t)j);
}

int main() {
    /* Declare 11 volatile variables with distinct prime numbers */
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
    volatile int8_t c1 = 37;
    volatile int16_t s1 = 41;
    volatile int32_t i1 = 43;
    volatile uint8_t uc1 = 47;
    volatile uint16_t us1 = 53;
    volatile uint32_t ui1 = 59;
    volatile uint64_t ul1 = 61;
    volatile float f1 = 67.0f;
    volatile double d1 = 71.0;
    
    /* Vector variables */
    v10si vec10 = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    v10si vec10b = {31, 37, 41, 43, 47, 53, 59, 61, 67, 71};
    
    v11si vec11 = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31};
    v11si vec11b = {37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79};
    
    int64_t checksum = 0;
    
    /* Main loop */
    for (int iter = 0; iter < 1000; iter++) {
        /* 10-operand inline assembly */
        int64_t asm_result = custom_op10(v0 + iter, v1, v2, v3, v4, v5, v6, v7, v8, v9);
        checksum ^= asm_result;
        
        /* 11-operand inline assembly */
        int64_t asm_result11 = custom_op11(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10 + iter);
        checksum += asm_result11;
        
        /* 11-operand complex expression */
        int64_t expr_result = complex_expr11(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum *= (expr_result | 1);
        
        /* 10-operand intrinsic-like function with mixed types */
        int64_t intrinsic_result = intrinsic_op10(c1, s1, i1, v0, uc1, us1, ui1, ul1, f1, d1);
        checksum -= intrinsic_result;
        
        /* Vector operations */
        v10si vec_result10 = vec10 + vec10b;
        v11si vec_result11 = vec11 + vec11b;
        
        /* Extract results from vectors */
        int64_t vec_sum = 0;
        for (int i = 0; i < 10; i++) {
            vec_sum += vec_result10[i];
        }
        for (int i = 0; i < 11; i++) {
            vec_sum += vec_result11[i];
        }
        checksum ^= vec_sum;
        
        /* Update variables to prevent constant propagation */
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
        
        c1 += 12;
        s1 += 13;
        i1 += 14;
        uc1 += 15;
        us1 += 16;
        ui1 += 17;
        ul1 += 18;
        f1 += 19.0f;
        d1 += 20.0;
        
        /* Update vector elements */
        for (int i = 0; i < 10; i++) {
            vec10[i] += i;
            vec10b[i] += i + 1;
        }
        for (int i = 0; i < 11; i++) {
            vec11[i] += i;
            vec11b[i] += i + 1;
        }
    }
    
    /* Conditional branch based on checksum */
    if (checksum > 0) {
        printf("Positive checksum: %ld\n", (long)checksum);
    } else if (checksum < 0) {
        printf("Negative checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum\n");
    }
    
    return (checksum != 0) ? 0 : 1;
}
