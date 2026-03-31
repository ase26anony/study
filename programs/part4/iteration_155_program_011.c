/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types for different architectures */
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

/* Strategy 1: Complex vector operations with FMA chaining */
#ifdef __FMA__
static void test_vector_fma_chain(void) {
#ifdef __AVX512F__
    v8df a = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df b = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    v8df c = {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    v8df d = {4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0};
    v8df e = {5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    
    /* Chain multiple FMA operations - may generate RTL with many operands */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    
    /* Use result to prevent optimization */
    volatile v8df *volatile ptr = &a;
    (void)ptr;
#endif
}
#endif

/* Strategy 2: Inline assembly with exactly 10 and 11 operands */
static void test_inline_asm_10_operands(void) {
    int64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int64_t in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    int64_t out1, out2;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "/* 10-operand dummy asm */\n\t"
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0"
        : "=r"(out1)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), 
          "r"(in5), "r"(in6), "r"(in7), "r"(in8), "r"(in9)
        : "cc"
    );
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "/* 11-operand dummy asm */\n\t"
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0\n\t"
        "add %10, %0"
        : "=r"(out2)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), 
          "r"(in5), "r"(in6), "r"(in7), "r"(in8), 
          "r"(in9), "r"(in10)
        : "cc"
    );
    
    /* Use results */
    volatile int64_t *volatile ptr1 = &out1;
    volatile int64_t *volatile ptr2 = &out2;
    (void)ptr1;
    (void)ptr2;
}

/* Strategy 3: Complex shuffle/permute with large masks */
#ifdef __AVX512F__
static void test_vector_shuffle(void) {
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
                 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f};
    
    /* Complex shuffle with many immediate operands in mask */
    __attribute__((unused)) v16sf result = __builtin_shuffle(v1, v2, 
        (int16_t){0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23});
    
    /* Another shuffle with different pattern */
    __attribute__((unused)) v16sf result2 = __builtin_shuffle(v1, v2,
        (int16_t){31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16});
}
#endif

/* Strategy 4: Target-specific builtins for multi-operand instructions */
#ifdef __AVX512F__
static void test_avx512_gather(void) {
    /* AVX-512 gather instructions have many operands */
    int base[64] = {0};
    __m512i vindex = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512i vmask = _mm512_set1_epi32(-1);
    __m512 scale = _mm512_set1_ps(1.0f);
    
    /* This gather operation conceptually has many operands */
    __m512 result = _mm512_i32gather_ps(vindex, base, 4);
    __m512 result2 = _mm512_mask_i32gather_ps(_mm512_setzero_ps(), vmask, 
                                              vindex, base, 4);
    
    /* Use results */
    volatile __m512 *volatile ptr = &result;
    volatile __m512 *volatile ptr2 = &result2;
    (void)ptr;
    (void)ptr2;
}
#endif

/* Strategy 5: Complex constant expressions with many terms */
static int test_complex_const_expr(void) {
    /* Force compiler to handle large constant expression */
    int x = 
        1 * 2 + 3 * 4 + 5 * 6 + 7 * 8 + 9 * 10 +
        11 * 12 + 13 * 14 + 15 * 16 + 17 * 18 + 19 * 20 +
        21 * 22;
    
    /* Use __builtin_constant_p to potentially generate RTL for both paths */
    if (__builtin_constant_p(x)) {
        return x + 1;
    } else {
        return x - 1;
    }
}

/* Strategy 6: Template-like approach using macros */
#define GEN_OPERATION(N) \
    static int operation_##N(int a, int b) { \
        return a + b + (N * 2) + (N / 3) + (N % 5) + \
               (N << 2) + (N >> 1) + (~N) + (N ^ 0xFF) + \
               (N & 0x0F) + (N | 0xF0); \
    }

/* Generate multiple instantiations */
GEN_OPERATION(10)
GEN_OPERATION(11)
GEN_OPERATION(12)
GEN_OPERATION(13)
GEN_OPERATION(14)
GEN_OPERATION(15)

/* Main function that exercises all strategies */
int main(void) {
    int result = 0;
    
    /* Test inline assembly with 10/11 operands */
    test_inline_asm_10_operands();
    
    /* Test vector operations if supported */
#ifdef __FMA__
    test_vector_fma_chain();
#endif
    
#ifdef __AVX512F__
    test_vector_shuffle();
    test_avx512_gather();
#endif
    
    /* Test complex constant expression */
    result += test_complex_const_expr();
    
    /* Test multiple instantiations of macro-generated functions */
    result += operation_10(1, 2);
    result += operation_11(2, 3);
    result += operation_12(3, 4);
    result += operation_13(4, 5);
    result += operation_14(5, 6);
    result += operation_15(6, 7);
    
    /* Additional complex expression that might generate many operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int complex_result = 
        (((((((((a + b) * c) - d) / e) << f) >> g) & h) | i) ^ j) + k;
    
    result += complex_result;
    
    /* Force side effects */
    volatile int *volatile ptr = &result;
    
    return result == 0 ? 0 : 1;
}
