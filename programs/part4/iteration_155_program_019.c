/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Strategy 1: Vector operations with FMA chaining */
#ifdef __AVX512F__
typedef double v8df __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));

static v8df vector_fma_chain(v8df a, v8df b, v8df c, v8df d, v8df e) {
    /* Complex FMA chain that might generate multi-operand RTL */
    return __builtin_fma(a, b, __builtin_fma(c, d, e));
}

static v16sf vector_shuffle_test(v16sf a, v16sf b) {
    /* Large shuffle with constant mask - may generate many immediate operands */
    const int mask[16] = {0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23};
    return __builtin_shuffle(a, b, mask);
}
#endif

/* Strategy 2: Inline assembly with many operands */
static void inline_asm_10_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
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
        : [out] "=r" (o0)
        : [in1] "r" (i1), [in2] "r" (i2), [in3] "r" (i3),
          [in4] "r" (i4), [in5] "r" (i5), [in6] "r" (i6),
          [in7] "r" (i7), [in8] "r" (i8), [in9] "r" (i9)
        : "cc"
    );
    
    /* Use result to prevent optimization */
    volatile int64_t dummy = o0;
    (void)dummy;
}

static void inline_asm_11_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "mov %[out], %[in1]\n\t"
        "add %[out], %[out], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]\n\t"
        "add %[out], %[out], %[in10]"
        : [out] "=r" (o0)
        : [in1] "r" (i1), [in2] "r" (i2), [in3] "r" (i3),
          [in4] "r" (i4), [in5] "r" (i5), [in6] "r" (i6),
          [in7] "r" (i7), [in8] "r" (i8), [in9] "r" (i9),
          [in10] "r" (i10)
        : "cc"
    );
    
    volatile int64_t dummy = o0;
    (void)dummy;
}

/* Strategy 3: Complex constant expression */
static int complex_constant_expr(void) {
    /* Force compiler to handle large constant expression */
    int result = 
        __builtin_constant_p(1) ? (
            1 * 2 + 3 * 4 + 5 * 6 + 7 * 8 + 9 * 10 +
            11 * 12 + 13 * 14 + 15 * 16 + 17 * 18 + 19 * 20
        ) : 0;
    
    /* Another complex expression with 11 terms */
    int result2 = 
        1 << 1 | 2 << 2 | 3 << 3 | 4 << 4 | 5 << 5 |
        6 << 6 | 7 << 7 | 8 << 8 | 9 << 9 | 10 << 10 | 11 << 11;
    
    return result + result2;
}

/* Strategy 4: Template/generic approach (C++ compatible) */
#ifdef __cplusplus
template<typename T, int N>
T template_accumulate(T a) {
    /* Generate different instantiations with many operations */
    return a + N + (N-1) + (N-2) + (N-3) + (N-4) + 
           (N-5) + (N-6) + (N-7) + (N-8) + (N-9);
}

/* Instantiate with multiple types and constants */
static void test_templates(void) {
    int r1 = template_accumulate<int, 10>(5);
    int r2 = template_accumulate<int, 11>(5);
    int r3 = template_accumulate<int, 12>(5);
    
    volatile int dummy = r1 + r2 + r3;
    (void)dummy;
}
#endif

/* Strategy 5: Target-specific builtins */
#ifdef __AVX512F__
#include <immintrin.h>
static void avx512_gather_test(void) {
    /* AVX-512 gather can have many operands */
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xFFFF;
    float base[64] = {0};
    
    __m512 result = _mm512_mask_i32gather_ps(
        _mm512_setzero_ps(),  // src
        mask,                 // mask
        index,                // index
        (void*)base,          // base
        4                     // scale
    );
    
    /* Use result */
    volatile __m512 dummy = result;
    (void)dummy;
}
#endif

/* Main test driver */
int main(void) {
    printf("Testing 10/11 operand expansion paths...\n");
    
    /* Test inline assembly paths */
    inline_asm_10_operands();
    inline_asm_11_operands();
    
    /* Test complex constant expressions */
    int const_result = complex_constant_expr();
    volatile int dummy_const = const_result;
    (void)dummy_const;
    
#ifdef __AVX512F__
    /* Test vector operations if AVX-512 available */
    v8df v1 = {0}, v2 = {1}, v3 = {2}, v4 = {3}, v5 = {4};
    v8df vresult = vector_fma_chain(v1, v2, v3, v4, v5);
    
    v16sf sv1 = {0}, sv2 = {1};
    v16sf svresult = vector_shuffle_test(sv1, sv2);
    
    /* Test AVX-512 gather */
    avx512_gather_test();
    
    volatile v8df dummy_v = vresult;
    volatile v16sf dummy_sv = svresult;
    (void)dummy_v;
    (void)dummy_sv;
#endif

#ifdef __cplusplus
    /* Test template instantiations in C++ mode */
    test_templates();
#endif
    
    printf("Test completed.\n");
    return 0;
}
