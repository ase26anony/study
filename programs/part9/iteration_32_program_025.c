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
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
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
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), 
          "r"(i), "r"(j), "r"(k)
        : "cc"
    );
    return result;
}

/* Strategy 2: Vector Operations with Many Elements */
typedef int64_t v10si __attribute__((vector_size(10 * sizeof(int64_t))));
typedef int64_t v11si __attribute__((vector_size(11 * sizeof(int64_t))));

/* Strategy 4: Intrinsic-Like Function with Many Arguments */
static inline int64_t __attribute__((always_inline))
intrinsic_11arg(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e,
                int64_t f, int64_t g, int64_t h, int64_t i, int64_t j,
                int64_t k) {
    /* Complex operation that uses all arguments */
    return ((a ^ b) | (c & d)) + ((e << 2) | (f >> 3)) * 
           ((g + h) - (i * j)) / (k + 1);
}

/* Strategy 5: Mixed-Type Operations */
static inline double mixed_type_10op(int8_t a, int16_t b, int32_t c, int64_t d,
                                     float e, double f, uint8_t g, uint16_t h,
                                     uint32_t i, uint64_t j) {
    /* Mix of operations that may require type conversions */
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
    
    int64_t checksum = 0;
    
    /* Run 1000 iterations */
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. Use 10-operand inline assembly */
        int64_t result1 = custom_10op(v0 + iter, v1, v2, v3, v4, v5, v6, v7, v8, v9);
        checksum ^= result1;
        
        /* 2. Use 11-operand inline assembly */
        int64_t result2 = custom_11op(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10 + iter);
        checksum += result2;
        
        /* 3. Strategy 3: Complex Expression Tree with 11 operands */
        int64_t result3 = ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                              v6) ^ v7) + v8) * v9) / (v10 + 1)) + iter;
        checksum |= result3;
        
        /* 4. Use intrinsic-like function with 11 arguments */
        int64_t result4 = intrinsic_11arg(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum *= (result4 + 1);
        
        /* 5. Use mixed-type function with 10 operands */
        double result5 = mixed_type_10op(mv0, mv1, mv2, mv3, mv4, mv5, mv6, mv7, mv8, mv9);
        checksum += (int64_t)result5;
        
        /* 6. Strategy 2: Vector operations */
        v10si vec10 = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9};
        v10si vec10_add = vec10 + (v10si){iter, iter, iter, iter, iter, 
                                         iter, iter, iter, iter, iter};
        /* Extract and sum all elements */
        int64_t* ptr10 = (int64_t*)&vec10_add;
        int64_t vec_sum = 0;
        for (int i = 0; i < 10; i++) {
            vec_sum += ptr10[i];
        }
        checksum -= vec_sum;
        
        /* 11-element vector */
        v11si vec11 = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec11_add = vec11 + (v11si){iter, iter, iter, iter, iter, 
                                         iter, iter, iter, iter, iter, iter};
        int64_t* ptr11 = (int64_t*)&vec11_add;
        int64_t vec_sum11 = 0;
        for (int i = 0; i < 11; i++) {
            vec_sum11 += ptr11[i];
        }
        checksum ^= vec_sum11;
    }
    
    /* Conditional branch based on checksum */
    if (checksum > 0) {
        printf("Positive checksum: %ld\n", checksum);
    } else if (checksum < 0) {
        printf("Negative checksum: %ld\n", checksum);
    } else {
        printf("Zero checksum\n");
    }
    
    return 0;
}
