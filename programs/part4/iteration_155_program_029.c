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

/* Pattern 1: Complex vector operations with chained FMAs */
#ifdef __FMA__
__attribute__((noinline))
static void test_vector_fma_chain(void) {
#ifdef __AVX512F__
    v8df a = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df b = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    v8df c = {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    v8df d = {4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0};
    v8df e = {5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    
    /* Complex FMA chain - may generate RTL with many operands */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    volatile v8df sink = a;
    (void)sink;
#endif
}
#endif

/* Pattern 2: Inline assembly with exactly 10 and 11 operands */
__attribute__((noinline))
static void test_inline_asm_10_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "add %1, %2\n\t"
        "add %3, %4\n\t"
        "add %5, %6\n\t"
        "add %7, %8\n\t"
        "mov %9, %0"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    volatile int64_t sink = o0;
    (void)sink;
}

__attribute__((noinline))
static void test_inline_asm_11_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "add %1, %2\n\t"
        "add %3, %4\n\t"
        "add %5, %6\n\t"
        "add %7, %8\n\t"
        "add %9, %10\n\t"
        "mov %10, %0"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), 
          "r"(i9), "r"(i10)
        : "cc"
    );
    
    volatile int64_t sink = o0;
    (void)sink;
}

/* Pattern 3: Complex shuffle/permute with large masks */
#ifdef __AVX512F__
__attribute__((noinline))
static void test_complex_shuffle(void) {
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
                 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f};
    
    /* Complex shuffle with 16-element mask - may generate many immediate operands */
    int mask[16] = {0, 16, 1, 17, 2, 18, 3, 19, 
                    4, 20, 5, 21, 6, 22, 7, 23};
    
    v16sf result = __builtin_shuffle(v1, v2, 
        mask[0], mask[1], mask[2], mask[3],
        mask[4], mask[5], mask[6], mask[7],
        mask[8], mask[9], mask[10], mask[11],
        mask[12], mask[13], mask[14], mask[15]);
    
    volatile v16sf sink = result;
    (void)sink;
}
#endif

/* Pattern 4: Target-specific builtins for multi-operand instructions */
#ifdef __AVX512F__
#include <immintrin.h>
__attribute__((noinline))
static void test_avx512_gather(void) {
    __m512i index = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7,
                                     8, 9, 10, 11, 12, 13, 14, 15);
    float base[64] = {0};
    __mmask16 mask = 0xFFFF;
    
    /* AVX-512 gather has multiple operands: src, base, index, scale, mask */
    __m512 result = _mm512_mask_i32gather_ps(
        _mm512_setzero_ps(),  // src
        mask,                 // mask
        index,                // index
        base,                 // base pointer
        4                     // scale
    );
    
    volatile __m512 sink = result;
    (void)sink;
}
#endif

/* Pattern 5: Complex constant expression that may not fold immediately */
__attribute__((noinline, const))
static int complex_constant_expr(void) {
    /* Large constant expression - compiler may create RTL with many immediates */
    return (1 << 0) + (2 << 1) + (3 << 2) + (4 << 3) + (5 << 4) +
           (6 << 5) + (7 << 6) + (8 << 7) + (9 << 8) + (10 << 9) +
           (11 << 10);
}

/* Pattern 6: Template-like pattern using macros for C */
#define DEFINE_VECTOR_OP(TYPE, SUFFIX) \
    __attribute__((noinline)) \
    static TYPE vector_op_##SUFFIX(TYPE a, TYPE b, TYPE c, TYPE d) { \
        return (a + b) * (c - d) + (a * b) - (c / d) + \
               (a & b) | (c ^ d) + (a << 2) + (b >> 3); \
    }

DEFINE_VECTOR_OP(int64_t, int64)
DEFINE_VECTOR_OP(uint64_t, uint64)
#ifdef __SSE2__
DEFINE_VECTOR_OP(v2df, v2df)
DEFINE_VECTOR_OP(v4sf, v4sf)
#endif
#ifdef __AVX__
DEFINE_VECTOR_OP(v4df, v4df)
DEFINE_VECTOR_OP(v8sf, v8sf)
#endif

/* Main function that exercises all patterns */
int main(void) {
    int result = 0;
    
    /* Test inline assembly patterns */
    test_inline_asm_10_operands();
    test_inline_asm_11_operands();
    
    /* Test vector operations if supported */
#ifdef __FMA__
    test_vector_fma_chain();
#endif
    
#ifdef __AVX512F__
    test_complex_shuffle();
    test_avx512_gather();
#endif
    
    /* Test complex constant expression */
    result += complex_constant_expr();
    
    /* Test various vector operations */
    result += vector_op_int64(1, 2, 3, 4);
    result += vector_op_uint64(5, 6, 7, 8);
    
#ifdef __SSE2__
    v2df v2a = {1.0, 2.0};
    v2df v2b = {3.0, 4.0};
    v2df v2c = {5.0, 6.0};
    v2df v2d = {7.0, 8.0};
    volatile v2df v2r = vector_op_v2df(v2a, v2b, v2c, v2d);
    (void)v2r;
#endif
    
    /* Force compiler to consider all code paths */
    if (result > 1000) {
        printf("Result: %d\n", result);
    }
    
    return 0;
}
