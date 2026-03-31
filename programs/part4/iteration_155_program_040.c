/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Vector types for different architectures */
#ifdef __AVX512F__
typedef double v8df __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
#endif

#ifdef __AVX__
typedef double v4df __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
#endif

#ifdef __SSE2__
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Complex inline assembly with exactly 10 operands */
static void inline_asm_10_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "add %[i1], %[i2]\n\t"
        "add %[i3], %[i4]\n\t"
        "add %[i5], %[i6]\n\t"
        "add %[i7], %[i8]\n\t"
        "mov %[i9], %[o0]"
        : [o0] "=r" (o0)
        : [i1] "r" (i1), [i2] "r" (i2), [i3] "r" (i3),
          [i4] "r" (i4), [i5] "r" (i5), [i6] "r" (i6),
          [i7] "r" (i7), [i8] "r" (i8), [i9] "r" (i9)
        : "cc"
    );
    
    volatile int64_t dummy = o0; /* Prevent dead code elimination */
}

/* Complex inline assembly with exactly 11 operands */
static void inline_asm_11_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "add %[i1], %[i2]\n\t"
        "add %[i3], %[i4]\n\t"
        "add %[i5], %[i6]\n\t"
        "add %[i7], %[i8]\n\t"
        "add %[i9], %[i10]\n\t"
        "mov %[i1], %[o0]"
        : [o0] "=r" (o0)
        : [i1] "r" (i1), [i2] "r" (i2), [i3] "r" (i3),
          [i4] "r" (i4), [i5] "r" (i5), [i6] "r" (i6),
          [i7] "r" (i7), [i8] "r" (i8), [i9] "r" (i9),
          [i10] "r" (i10)
        : "cc"
    );
    
    volatile int64_t dummy = o0; /* Prevent dead code elimination */
}

/* Complex vector operations that may generate many operands */
#ifdef __AVX__
static void complex_vector_operations(void) {
    v4df a = {1.0, 2.0, 3.0, 4.0};
    v4df b = {5.0, 6.0, 7.0, 8.0};
    v4df c = {9.0, 10.0, 11.0, 12.0};
    v4df d = {13.0, 14.0, 15.0, 16.0};
    v4df e = {17.0, 18.0, 19.0, 20.0};
    
    /* Complex expression that might generate many operands */
    v4df result = a + b * c - d / e + a * b - c / d + e;
    
    /* Use shuffle with large mask - potentially many immediate operands */
    v4df shuffled = __builtin_shuffle(result, result, 
        (v4df){3, 2, 1, 0, 7, 6, 5, 4});
    
    volatile double dummy = shuffled[0]; /* Prevent elimination */
}
#endif

/* Complex constant expression with many terms */
static int complex_constant_expression(void) {
    /* Force compiler to handle many constants in one expression */
    int x = 
        1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 +
        11 + 12 + 13 + 14 + 15 + 16 + 17 + 18 + 19 + 20;
    
    /* Use __builtin_constant_p to force RTL generation */
    if (__builtin_constant_p(x)) {
        return x + 100;
    } else {
        return x - 100;
    }
}

/* Template-like approach using macros for different types */
#define DEFINE_COMPLEX_OP(TYPE, NAME) \
    static TYPE NAME(TYPE a, TYPE b, TYPE c, TYPE d, TYPE e) { \
        return ((a + b) * (c - d) / e) + \
               ((a - b) * (c + d) / e) + \
               ((a * b) + (c * d) - e) + \
               ((a / b) - (c / d) + e); \
    }

DEFINE_COMPLEX_OP(double, complex_double_op)
DEFINE_COMPLEX_OP(float, complex_float_op)

/* Test AVX-512 gather instructions if available */
#ifdef __AVX512F__
static void test_avx512_gather(void) {
    v16sf index = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    float base[64] = {0};
    v16sf mask = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    
    /* This builtin has many parameters: source, index, scale, base, mask */
    v16sf result = __builtin_ia32_gatherpfd512(mask, (const void *)base, 
                                               index, 1, 1);
    
    volatile float dummy = result[0];
}
#endif

/* Main function that exercises all patterns */
int main(void) {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Test inline assembly patterns */
    inline_asm_10_operands();
    inline_asm_11_operands();
    
    /* Test complex constant expressions */
    int const_result = complex_constant_expression();
    printf("Constant expression result: %d\n", const_result);
    
    /* Test complex floating point operations */
    double d_result = complex_double_op(1.0, 2.0, 3.0, 4.0, 5.0);
    float f_result = complex_float_op(1.0f, 2.0f, 3.0f, 4.0f, 5.0f);
    printf("Double op result: %f\n", d_result);
    printf("Float op result: %f\n", f_result);
    
#ifdef __AVX__
    /* Test vector operations if AVX is available */
    complex_vector_operations();
    printf("AVX vector operations completed\n");
#endif
    
#ifdef __AVX512F__
    /* Test AVX-512 operations if available */
    test_avx512_gather();
    printf("AVX-512 operations completed\n");
#endif
    
    /* Force use of all results to prevent optimization */
    volatile int use_results = const_result + (int)d_result + (int)f_result;
    
    return 0;
}
