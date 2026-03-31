#include <stdio.h>
#include <stdint.h>

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
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j), "r"(k)
        : "cc"
    );
    return result;
}

/* 11-element vector type */
typedef int64_t v11si __attribute__((vector_size(88)));

/* Always-inline function with 11 arguments */
static inline __attribute__((always_inline)) 
int64_t multi_op_inline(int64_t a, int64_t b, int64_t c, int64_t d,
                        int64_t e, int64_t f, int64_t g, int64_t h,
                        int64_t i, int64_t j, int64_t k) {
    return (((((((((a + b) * c) - d) << (e & 3)) & f) | g) ^ h) + i) * j) / (k ? k : 1);
}

/* Mixed-type function with 10 arguments */
static inline __attribute__((always_inline))
double mixed_multi_op(int a, short b, char c, long d, 
                      float e, double f, int g, short h,
                      char i, long j) {
    return (double)(a + b + c + d) + (double)e + f + (double)(g + h + i + j);
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
    
    /* Additional mixed-type variables */
    volatile int vi = 37;
    volatile short vs = 41;
    volatile char vc = 43;
    volatile long vl = 47;
    volatile float vf = 53.0f;
    volatile double vd = 59.0;
    
    int64_t checksum = 0;
    
    /* Loop to ensure multiple executions */
    for (int iter = 0; iter < 1000; iter++) {
        /* 1. 10-operand inline assembly */
        int64_t asm_result10 = custom_op10(v0, v1, v2, v3, v4, 
                                          v5, v6, v7, v8, v9);
        
        /* 2. 11-operand inline assembly */
        int64_t asm_result11 = custom_op11(v0, v1, v2, v3, v4,
                                          v5, v6, v7, v8, v9, v10);
        
        /* 3. Complex 11-operand expression tree */
        int64_t expr_result = (((((((((v0 + v1) * v2) - v3) << (v4 & 3)) & v5) | 
                                v6) ^ v7) + v8) * v9) / (v10 ? v10 : 1);
        
        /* 4. Always-inline function with 11 arguments */
        int64_t inline_result = multi_op_inline(v0, v1, v2, v3, v4,
                                               v5, v6, v7, v8, v9, v10);
        
        /* 5. Mixed-type operation with 10 arguments */
        double mixed_result = mixed_multi_op(vi, vs, vc, vl, vf, vd,
                                            vi+1, vs+1, vc+1, vl+1);
        
        /* 6. Vector operations with 11 elements */
        v11si vec1 = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10};
        v11si vec2 = {v10, v9, v8, v7, v6, v5, v4, v3, v2, v1, v0};
        v11si vec_result = vec1 + vec2;
        
        /* Extract result from vector */
        int64_t vec_sum = 0;
        for (int i = 0; i < 11; i++) {
            vec_sum += vec_result[i];
        }
        
        /* Update checksum to prevent dead code elimination */
        checksum += asm_result10 + asm_result11 + expr_result + 
                   inline_result + (int64_t)mixed_result + vec_sum;
        
        /* Modify variables slightly each iteration */
        v0 += 1; v1 += 2; v2 += 3; v3 += 4; v4 += 5;
        v5 += 6; v6 += 7; v7 += 8; v8 += 9; v9 += 10;
        v10 += 11;
        vi += 1; vs += 2; vc += 3; vl += 4;
        vf += 0.5f; vd += 0.7;
    }
    
    /* Conditional branch depending on checksum */
    if (checksum > 0) {
        printf("Checksum: %ld\n", (long)checksum);
    } else {
        printf("Zero checksum (unlikely)\n");
    }
    
    return 0;
}
