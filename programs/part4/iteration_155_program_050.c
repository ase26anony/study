/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Vector types for different architectures */
typedef float v8sf __attribute__((vector_size(32)));  /* 8 floats = 256 bits */
typedef double v4df __attribute__((vector_size(32))); /* 4 doubles = 256 bits */
typedef int v8si __attribute__((vector_size(32)));    /* 8 ints = 256 bits */

/* Strategy 1: Complex vector operations with FMA chaining */
__attribute__((noinline))
v4df vector_fma_chain(v4df a, v4df b, v4df c, v4df d, v4df e) {
    /* Chain multiple FMA operations - may generate RTL with many operands */
    return __builtin_fma(a, b, __builtin_fma(c, d, e));
}

/* Strategy 2: Vector shuffle with large constant mask */
__attribute__((noinline))
v8sf vector_shuffle_complex(v8sf a, v8sf b) {
    /* Shuffle with 8-element constant mask = 8 immediate operands + 2 vectors */
    return __builtin_shuffle(a, b, 
        (v8si){7, 6, 5, 4, 3, 2, 1, 0});
}

/* Strategy 3: Inline assembly with exactly 10 operands */
__attribute__((noinline))
int asm_10_operands(int a, int b, int c, int d, int e, 
                    int f, int g, int h, int i, int j) {
    int result;
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "add %[out], %[in1], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]"
        : [out] "=r" (result)
        : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c),
          [in4] "r" (d), [in5] "r" (e), [in6] "r" (f),
          [in7] "r" (g), [in8] "r" (h), [in9] "r" (i)
        : "cc"
    );
    return result + j; /* 10th input used separately */
}

/* Strategy 4: Inline assembly with exactly 11 operands */
__attribute__((noinline))
int asm_11_operands(int a, int b, int c, int d, int e, int f,
                    int g, int h, int i, int j, int k) {
    int result;
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "mov %[out], #0\n\t"
        "add %[out], %[out], %[in1]\n\t"
        "add %[out], %[out], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]\n\t"
        "add %[out], %[out], %[in10]"
        : [out] "=r" (result)
        : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c),
          [in4] "r" (d), [in5] "r" (e), [in6] "r" (f),
          [in7] "r" (g), [in8] "r" (h), [in9] "r" (i),
          [in10] "r" (j)
        : "cc"
    );
    return result + k; /* 11th input used separately */
}

/* Strategy 5: Complex constant expression that might not fold immediately */
__attribute__((noinline))
int complex_const_expression(void) {
    /* Force compiler to consider all operands before folding */
    if (__builtin_constant_p(0)) {
        return 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    } else {
        volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
        volatile int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
        return a + b + c + d + e + f + g + h + i + j + k;
    }
}

/* Strategy 6: Template-like approach using macros for multiple types */
#define GEN_OPERATION(TYPE, NAME) \
    __attribute__((noinline)) \
    TYPE NAME(TYPE a, TYPE b, TYPE c, TYPE d, TYPE e, \
              TYPE f, TYPE g, TYPE h, TYPE i, TYPE j) { \
        return a + b + c + d + e + f + g + h + i + j; \
    }

GEN_OPERATION(int, int_10_operands)
GEN_OPERATION(float, float_10_operands)
GEN_OPERATION(double, double_10_operands)

/* Strategy 7: AVX-512 style gather operation simulation */
#ifdef __AVX512F__
#include <immintrin.h>
__attribute__((noinline))
__m512i avx512_gather_like(__m512i index, __m512i mask, 
                           const void* base, int scale) {
    /* Simulate a gather with many parameters */
    return _mm512_maskz_expand_epi32(mask, index);
}
#endif

/* Main function that uses all strategies */
int main(void) {
    volatile int result = 0;
    
    /* Test 1: Vector FMA chain */
    v4df v1 = {1.0, 2.0, 3.0, 4.0};
    v4df v2 = {5.0, 6.0, 7.0, 8.0};
    v4df v3 = {9.0, 10.0, 11.0, 12.0};
    v4df v4 = {13.0, 14.0, 15.0, 16.0};
    v4df v5 = {17.0, 18.0, 19.0, 20.0};
    
    v4df vres = vector_fma_chain(v1, v2, v3, v4, v5);
    result += (int)vres[0];
    
    /* Test 2: Vector shuffle */
    v8sf sv1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf sv2 = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v8sf svres = vector_shuffle_complex(sv1, sv2);
    result += (int)svres[0];
    
    /* Test 3: 10-operand assembly */
    result += asm_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test 4: 11-operand assembly */
    result += asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test 5: Complex constant expression */
    result += complex_const_expression();
    
    /* Test 6: Multiple type instantiations */
    result += int_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += (int)float_10_operands(1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                                     6.0f, 7.0f, 8.0f, 9.0f, 10.0f);
    result += (int)double_10_operands(1.0, 2.0, 3.0, 4.0, 5.0,
                                      6.0, 7.0, 8.0, 9.0, 10.0);
    
    /* Test 7: Large immediate operand expression */
    int large_expr = 0;
    for (int i = 0; i < 100; i++) {
        /* This might generate RTL with many operands during unrolling */
        large_expr += i * (i + 1) * (i + 2) * (i + 3) * (i + 4);
    }
    result += large_expr;
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
