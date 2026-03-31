#include <stdio.h>
#include <stdint.h>

/* Strategy 1: Multi-Operand Builtin Functions using inline assembly */
static inline int64_t custom_10op_asm(int64_t a, int64_t b, int64_t c, int64_t d,
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

static inline int64_t custom_11op_asm(int64_t a, int64_t b, int64_t c, int64_t d,
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
typedef int64_t v10si __attribute__((vector_size(10 * sizeof(int64_t))));
typedef int64_t v11si __attribute__((vector_size(11 * sizeof(int64_t))));

/* Strategy 3: Complex expression trees */
static inline int64_t complex_11op_expr(volatile int64_t a, volatile int64_t b,
                                       volatile int64_t c, volatile int64_t d,
                                       volatile int64_t e, volatile int64_t f,
                                       volatile int64_t g, volatile int64_t h,
                                       volatile int64_t i, volatile int64_t j,
                                       volatile int64_t k) {
    /* 11-operand complex expression that's hard to break apart */
    return ((((((((((a + b) * c) - d) << (e & 3)) & f) | g) ^ h) + i) * j) / (k | 1));
}

/* Strategy 4: Intrinsic-like function with many arguments */
__attribute__((always_inline))
static inline double mixed_10op_func(int8_t a, int16_t b, int32_t c, int64_t d,
                                    float e, double f, uint8_t g, uint16_t h,
                                    uint32_t i, uint64_t j) {
    /* Mixed-type operation that might trigger type-specific expansions */
    return (double)a + (double)b + (double)c + (double)d + 
           (double)e + f + (double)g + (double)h + 
           (double)i + (double)j;
}

__attribute__((always_inline))
static inline double mixed_11op_func(int8_t a, int16_t b, int32_t c, int64_t d,
                                    float e, double f, uint8_t g, uint16_t h,
                                    uint32_t i, uint64_t j, long double k) {
    /* 11-operand mixed-type function */
    return (double)a + (double)b + (double)c + (double)d + 
           (double)e + f + (double)g + (double)h + 
           (double)i + (double)j + (double)k;
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
    
    /* Mixed-type variables for Strategy 5 */
    volatile int8_t m1 = 2;
    volatile int16_t m2 = 3;
    volatile int32_t m3 = 5;
    volatile int64_t m4 = 7;
    volatile float m5 = 11.0f;
    volatile double m6 = 13.0;
    volatile uint8_t m7 = 17;
    volatile uint16_t m8 = 19;
    volatile uint32_t m9 = 23;
    volatile uint64_t m10 = 29;
    volatile long double m11 = 31.0L;
    
    int64_t checksum = 0;
    
    /* Run 1000 iterations to ensure execution */
    for (int iter = 0; iter < 1000; iter++) {
        /* Strategy 1: 10-operand inline assembly */
        int64_t asm10_result = custom_10op_asm(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9);
        checksum ^= asm10_result;
        
        /* Strategy 1: 11-operand inline assembly */
        int64_t asm11_result = custom_11op_asm(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum ^= asm11_result;
        
        /* Strategy 3: 11-operand complex expression */
        int64_t expr_result = complex_11op_expr(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum ^= expr_result;
        
        /* Strategy 2: Vector operations */
        v10si vec10_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v10si vec10_result = vec10_a + vec10_b;
        
        v11si vec11_a = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec11_b = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v0};
        v11si vec11_result = vec11_a + vec11_b;
        
        /* Extract results from vectors */
        for (int i = 0; i < 10; i++) {
            checksum ^= vec10_result[i];
        }
        for (int i = 0; i < 11; i++) {
            checksum ^= vec11_result[i];
        }
        
        /* Strategy 4 & 5: Mixed-type functions */
        double mixed10_result = mixed_10op_func(m1, m2, m3, m4, m5, m6, m7, m8, m9, m10);
        checksum ^= (int64_t)mixed10_result;
        
        double mixed11_result = mixed_11op_func(m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11);
        checksum ^= (int64_t)mixed11_result;
        
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
    }
    
    /* Conditional branch depending on checksum */
    if (checksum != 0) {
        printf("Checksum: %lld\n", (long long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return (checksum & 1);
}
