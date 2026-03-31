/* optabs_coverage.c - Program to trigger 10/11 operand expansion in optabs.cc */

#include <stdio.h>
#include <stdint.h>

/* Strategy 1: Multi-Operand Builtin Functions using inline assembly */
static inline int64_t custom_op10(int64_t a, int64_t b, int64_t c, int64_t d,
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

static inline int64_t custom_op11(int64_t a, int64_t b, int64_t c, int64_t d,
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
static inline int64_t complex_expr11(volatile int64_t a, volatile int64_t b,
                                     volatile int64_t c, volatile int64_t d,
                                     volatile int64_t e, volatile int64_t f,
                                     volatile int64_t g, volatile int64_t h,
                                     volatile int64_t i, volatile int64_t j,
                                     volatile int64_t k) {
    /* 11-operand complex expression that cannot be easily broken up */
    return ((((((((((a + b) * c) - d) << (e & 3)) & f) | g) ^ h) + i) * j) / (k | 1));
}

/* Strategy 4: Intrinsic-Like Function with Many Arguments */
__attribute__((always_inline))
static inline double mixed_op11(volatile char a, volatile short b, 
                                volatile int c, volatile long d,
                                volatile float e, volatile double f,
                                volatile int8_t g, volatile int16_t h,
                                volatile int32_t i, volatile int64_t j,
                                volatile double k) {
    /* Mixed-type 11-operand operation */
    return (double)a + (double)b + (double)c + (double)d + 
           (double)e + f + (double)g + (double)h + 
           (double)i + (double)j + k;
}

/* Strategy 5: Custom multi-operand builtin simulation */
#define CUSTOM_BUILTIN_10(a,b,c,d,e,f,g,h,i,j) \
    __builtin_choose_expr(__builtin_constant_p(a), \
        (a)+(b)+(c)+(d)+(e)+(f)+(g)+(h)+(i)+(j), \
        custom_op10(a,b,c,d,e,f,g,h,i,j))

#define CUSTOM_BUILTIN_11(a,b,c,d,e,f,g,h,i,j,k) \
    __builtin_choose_expr(__builtin_constant_p(a), \
        (a)+(b)+(c)+(d)+(e)+(f)+(g)+(h)+(i)+(j)+(k), \
        custom_op11(a,b,c,d,e,f,g,h,i,j,k))

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
    
    /* Mixed type variables for Strategy 5 */
    volatile char cv = 2;
    volatile short sv = 3;
    volatile int iv = 5;
    volatile long lv = 7;
    volatile float fv = 11.0f;
    volatile double dv = 13.0;
    volatile int8_t i8v = 17;
    volatile int16_t i16v = 19;
    volatile int32_t i32v = 23;
    volatile int64_t i64v = 29;
    volatile double dv2 = 31.0;
    
    /* Vector initialization for Strategy 2 */
    v10si vec10 = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    v11si vec11 = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31};
    v10si vec10_add = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    v11si vec11_add = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    
    int64_t checksum = 0;
    
    /* Run 1000 iterations to ensure execution */
    for (int iter = 0; iter < 1000; iter++) {
        /* Strategy 1a: 10-operand inline assembly */
        int64_t result10 = custom_op10(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9);
        checksum ^= result10;
        
        /* Strategy 1b: 11-operand inline assembly */
        int64_t result11 = custom_op11(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum += result11;
        
        /* Strategy 2: Vector operations */
        v10si vec_result10 = vec10 + vec10_add;
        v11si vec_result11 = vec11 + vec11_add;
        
        /* Extract results from vectors */
        int32_t vec_sum = 0;
        for (int i = 0; i < 10; i++) vec_sum += vec_result10[i];
        for (int i = 0; i < 11; i++) vec_sum += vec_result11[i];
        checksum += vec_sum;
        
        /* Strategy 3: Complex 11-operand expression */
        int64_t expr_result = complex_expr11(v0, v1, v2, v3, v4, v5, 
                                            v6, v7, v8, v9, v10);
        checksum *= (expr_result | 1);
        
        /* Strategy 4: Intrinsic-like function with 11 mixed-type args */
        double mixed_result = mixed_op11(cv, sv, iv, lv, fv, dv,
                                        i8v, i16v, i32v, i64v, dv2);
        checksum += (int64_t)mixed_result;
        
        /* Strategy 5: Custom builtin macros */
        int64_t builtin10 = CUSTOM_BUILTIN_10(v0, v1, v2, v3, v4, 
                                             v5, v6, v7, v8, v9);
        int64_t builtin11 = CUSTOM_BUILTIN_11(v0, v1, v2, v3, v4,
                                             v5, v6, v7, v8, v9, v10);
        checksum += builtin10 + builtin11;
        
        /* Modify variables slightly each iteration */
        v0 += 1; v1 += 2; v2 += 3; v3 += 4; v4 += 5;
        v5 += 6; v6 += 7; v7 += 8; v8 += 9; v9 += 10; v10 += 11;
        
        /* Update vectors */
        for (int i = 0; i < 10; i++) vec10[i] += i;
        for (int i = 0; i < 11; i++) vec11[i] += i;
    }
    
    /* Conditional branch based on checksum */
    if (checksum > 0) {
        printf("Checksum (positive case): %ld\n", checksum);
    } else if (checksum < 0) {
        printf("Checksum (negative case): %ld\n", checksum);
    } else {
        printf("Checksum (zero case): %ld\n", checksum);
    }
    
    return (checksum != 0) ? 0 : 1;
}
