#include <stdio.h>
#include <stdint.h>

/* Strategy 1: Multi-Operand Builtin Functions */
static inline int64_t custom_10_op(int64_t a, int64_t b, int64_t c, int64_t d,
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
        "add %0, %0, %9"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    return result;
}

static inline int64_t custom_11_op(int64_t a, int64_t b, int64_t c, int64_t d,
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
        "add %0, %0, %10"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j), "r"(k)
        : "cc"
    );
    return result;
}

/* Strategy 2: Vector Operations with Many Elements */
typedef int32_t v10si __attribute__((vector_size(40)));  /* 10 elements */
typedef int32_t v11si __attribute__((vector_size(44)));  /* 11 elements */

/* Strategy 4: Intrinsic-Like Function with Many Arguments */
static inline int64_t __attribute__((always_inline))
multi_op_11(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e,
            int64_t f, int64_t g, int64_t h, int64_t i, int64_t j,
            int64_t k) {
    /* Complex operation that uses all arguments */
    return ((((((((((a * b) + c) - d) ^ e) | f) & g) << (h & 3)) >> (i & 3)) * j) / (k ? k : 1));
}

int main() {
    /* Strategy 5: Mixed-Type Operations */
    volatile int8_t   v0  = 2;    /* char */
    volatile int16_t  v1  = 3;    /* short */
    volatile int32_t  v2  = 5;    /* int */
    volatile int64_t  v3  = 7;    /* long */
    volatile uint8_t  v4  = 11;   /* unsigned char */
    volatile uint16_t v5  = 13;   /* unsigned short */
    volatile uint32_t v6  = 17;   /* unsigned int */
    volatile uint64_t v7  = 19;   /* unsigned long */
    volatile float    v8  = 23.0f;/* float */
    volatile double   v9  = 29.0; /* double */
    volatile int32_t  v10 = 31;   /* extra operand */
    
    int64_t checksum = 0;
    
    for (int iter = 0; iter < 1000; iter++) {
        /* Part 3a: 10-operand inline assembly */
        int64_t asm_result = custom_10_op(
            v0 + iter, v1, v2, v3, v4,
            v5, v6, v7, (int64_t)v8, (int64_t)v9
        );
        checksum ^= asm_result;
        
        /* Part 3b: 11-operand C expression (Strategy 3) */
        int64_t expr_result = (
            ((((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
            v6) ^ v7) + (int64_t)v8) * (int64_t)v9) / v10) + iter
        );
        checksum += expr_result;
        
        /* Part 3c: 11-element vector operation (Strategy 2) */
        v11si vec_a = {v0, v1, v2, v3, v4, v5, v6, v7, 
                      (int32_t)v8, (int32_t)v9, v10};
        v11si vec_b = {iter, iter+1, iter+2, iter+3, iter+4,
                      iter+5, iter+6, iter+7, iter+8, iter+9, iter+10};
        v11si vec_result = vec_a + vec_b;
        
        /* Extract and use vector results */
        for (int i = 0; i < 11; i++) {
            checksum += vec_result[i];
        }
        
        /* Strategy 4: Intrinsic-like function call */
        int64_t func_result = multi_op_11(
            v0, v1, v2, v3, v4, v5, v6, v7,
            (int64_t)v8, (int64_t)v9, v10
        );
        checksum *= (func_result + 1);
        
        /* Strategy 1 alternative: Direct 11-operand inline assembly */
        int64_t direct_asm_result;
        __asm__ volatile (
            "imul %0, %1, %2\n\t"
            "add %0, %0, %3\n\t"
            "sub %0, %0, %4\n\t"
            "xor %0, %0, %5\n\t"
            "or %0, %0, %6\n\t"
            "and %0, %0, %7\n\t"
            "add %0, %0, %8\n\t"
            "imul %0, %0, %9\n\t"
            "add %0, %0, %10"
            : "=r"(direct_asm_result)
            : "r"((int64_t)v0), "r"((int64_t)v1), "r"((int64_t)v2),
              "r"((int64_t)v3), "r"((int64_t)v4), "r"((int64_t)v5),
              "r"((int64_t)v6), "r"((int64_t)v7), "r"((int64_t)v8),
              "r"((int64_t)v9), "r"((int64_t)v10)
            : "cc"
        );
        checksum ^= direct_asm_result;
    }
    
    /* Strategy 5: Conditional branch to prevent dead code elimination */
    if (checksum != 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return (checksum & 1);
}
