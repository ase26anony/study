#include <stdio.h>
#include <stdint.h>

/* Approach 1: Multi-Operand Builtin Functions using inline assembly */
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

/* Approach 2: Vector Operations with Many Elements */
typedef int32_t v10si __attribute__((vector_size(40)));  /* 10 elements */
typedef int32_t v11si __attribute__((vector_size(44)));  /* 11 elements */

/* Approach 3: Complex Expression Trees */
#define COMPLEX_10OP_EXPR(a,b,c,d,e,f,g,h,i,j) \
    ((((((((((a + b) * c) - d) << (e & 3)) & f) | g) ^ h) + i) * j) / 13)

#define COMPLEX_11OP_EXPR(a,b,c,d,e,f,g,h,i,j,k) \
    (((((((((((a + b) * c) - d) << (e & 3)) & f) | g) ^ h) + i) * j) / 13) + k)

/* Approach 4: Intrinsic-Like Function with Many Arguments */
static inline int64_t __attribute__((always_inline))
intrinsic_11op(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e,
               int64_t f, int64_t g, int64_t h, int64_t i, int64_t j,
               int64_t k) {
    /* Complex operation that uses all arguments */
    return ((a ^ b) & (c | d)) + ((e << 2) & (f >> 1)) * 
           ((g + h) - (i * j)) / (k + 1);
}

/* Approach 5: Mixed-Type Operations */
static inline double mixed_type_10op(volatile char a, volatile short b,
                                     volatile int c, volatile long d,
                                     volatile float e, volatile double f,
                                     volatile int8_t g, volatile int16_t h,
                                     volatile int32_t i, volatile int64_t j) {
    /* Mix of operations with different types */
    return (double)(a + b + c + d) + (double)e + f + 
           (double)(g * h) / (double)(i + j);
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
    volatile char c1 = 2;
    volatile short s1 = 3;
    volatile int i1 = 5;
    volatile long l1 = 7;
    volatile float f1 = 11.0f;
    volatile double d1 = 13.0;
    volatile int8_t i8 = 17;
    volatile int16_t i16 = 19;
    volatile int32_t i32 = 23;
    volatile int64_t i64 = 29;
    
    /* Vector variables */
    v10si vec10 = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    v10si vec10_add = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    v11si vec11 = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31};
    v11si vec11_add = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    
    int64_t checksum = 0;
    
    /* Run 1000 iterations to ensure execution */
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int64_t asm10_result = custom_10op_asm(v0, v1, v2, v3, v4,
                                               v5, v6, v7, v8, v9);
        checksum ^= asm10_result;
        
        /* 2. 11-operand inline assembly */
        int64_t asm11_result = custom_11op_asm(v0, v1, v2, v3, v4, v5,
                                               v6, v7, v8, v9, v10);
        checksum += asm11_result;
        
        /* 3. Complex 10-operand expression tree */
        int64_t expr10_result = COMPLEX_10OP_EXPR(v0, v1, v2, v3, v4,
                                                  v5, v6, v7, v8, v9);
        checksum ^= expr10_result;
        
        /* 4. Complex 11-operand expression tree */
        int64_t expr11_result = COMPLEX_11OP_EXPR(v0, v1, v2, v3, v4, v5,
                                                  v6, v7, v8, v9, v10);
        checksum += expr11_result;
        
        /* 5. 11-operand intrinsic-like function */
        int64_t intrinsic_result = intrinsic_11op(v0, v1, v2, v3, v4, v5,
                                                  v6, v7, v8, v9, v10);
        checksum ^= intrinsic_result;
        
        /* 6. Mixed-type 10-operand operation */
        double mixed_result = mixed_type_10op(c1, s1, i1, l1, f1,
                                              d1, i8, i16, i32, i64);
        checksum += (int64_t)mixed_result;
        
        /* 7. Vector operations (10 and 11 elements) */
        v10si vec10_result = vec10 + vec10_add;
        v11si vec11_result = vec11 + vec11_add;
        
        /* Extract results from vectors */
        for (int i = 0; i < 10; i++) {
            checksum += vec10_result[i];
        }
        for (int i = 0; i < 11; i++) {
            checksum += vec11_result[i];
        }
        
        /* Modify variables slightly to prevent constant folding */
        v0 += 1; v1 += 2; v2 += 3; v3 += 4; v4 += 5;
        v5 += 6; v6 += 7; v7 += 8; v8 += 9; v9 += 10; v10 += 11;
        
        c1++; s1++; i1++; l1++; f1 += 1.0f; d1 += 1.0;
        i8++; i16++; i32++; i64++;
        
        /* Update vector elements */
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
    if (checksum != 0) {
        printf("Final checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return (checksum & 1);
}
