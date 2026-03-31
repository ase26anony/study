/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types for various architectures */
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
__attribute__((noinline))
static void test_vector_fma_chain(void) {
#ifdef __AVX512F__
    v8df a = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df b = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    v8df c = {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    v8df d = {4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0};
    v8df e = {5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    
    /* Complex FMA chain that might generate many operands */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    a = __builtin_fma(a, b, __builtin_fma(c, d, __builtin_fma(e, a, b)));
    
    volatile v8df sink = a;
    (void)sink;
#endif
}
#endif

/* Strategy 2: Inline assembly with exactly 10/11 operands */
__attribute__((noinline))
static void test_inline_asm_10_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "/* dummy 10-operand asm */\n\t"
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
          "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    volatile int64_t sink = o0;
    (void)sink;
}

__attribute__((noinline))
static void test_inline_asm_11_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10, i11 = 11;
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "/* dummy 11-operand asm */\n\t"
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
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
          "r"(i6), "r"(i7), "r"(i8), "r"(i9), "r"(i10)
        : "cc"
    );
    
    volatile int64_t sink = o0;
    (void)sink;
}

/* Strategy 3: Complex shuffle/permute operations */
#ifdef __AVX512F__
__attribute__((noinline))
static void test_complex_shuffle(void) {
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
                 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f};
    
    /* Complex shuffle with large constant mask - may generate many immediate operands */
    typedef int v16si __attribute__((vector_size(64)));
    v16si mask = {0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23};
    
    v16sf result = __builtin_shufflevector(v1, v2, 
        0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
    
    volatile v16sf sink = result;
    (void)sink;
}
#endif

/* Strategy 4: Target-specific builtins for multi-operand instructions */
#ifdef __AVX512F__
#include <immintrin.h>
__attribute__((noinline))
static void test_avx512_gather(void) {
    __m512i index = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    __mmask16 mask = 0xFFFF;
    float base[64] = {0};
    
    /* AVX-512 gather with many parameters */
    __m512 result = _mm512_mask_i32gather_ps(
        _mm512_setzero_ps(),  // src
        mask,                 // mask
        index,                // index
        (void*)base,          // base
        4                     // scale
    );
    
    volatile __m512 sink = result;
    (void)sink;
}
#endif

/* Strategy 5: Complex constant expressions */
__attribute__((noinline))
static int test_complex_const_expr(void) {
    /* Force compiler to generate RTL for complex constant computation */
    int x = 1, y = 2, z = 3;
    
    /* Complex expression that might not fold completely */
    int result = (x + y) * (z + 4) + (5 * 6) - (7 / 8) + (9 % 10) +
                 (11 << 12) + (13 >> 14) + (15 & 16) + (17 | 18) + (19 ^ 20);
    
    /* Use __builtin_constant_p to potentially prevent folding */
    if (__builtin_constant_p(result)) {
        return result + 21;
    } else {
        return result + 22;
    }
}

/* Strategy 6: Template/generic approach (C++ compatible) */
#ifdef __cplusplus
template<typename T, int N>
__attribute__((noinline))
T template_operation(T a, T b) {
    return a + b + static_cast<T>(N) + 
           static_cast<T>(N+1) + static_cast<T>(N+2) +
           static_cast<T>(N+3) + static_cast<T>(N+4) +
           static_cast<T>(N+5) + static_cast<T>(N+6) +
           static_cast<T>(N+7);
}
#endif

int main(void) {
    int result = 0;
    
    /* Test inline assembly with 10 and 11 operands */
    test_inline_asm_10_operands();
    test_inline_asm_11_operands();
    
#ifdef __FMA__
    test_vector_fma_chain();
#endif
    
#ifdef __AVX512F__
    test_complex_shuffle();
    test_avx512_gather();
#endif
    
    /* Test complex constant expression */
    result += test_complex_const_expr();
    
#ifdef __cplusplus
    /* Test template instantiations with different types */
    result += template_operation<int, 100>(1, 2);
    result += template_operation<float, 200>(1.0f, 2.0f);
#endif
    
    /* Ensure results are used */
    volatile int sink = result;
    
    return sink != 0 ? 0 : 1;
}
