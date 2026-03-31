/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Strategy 1: Complex vector operations with FMA chaining */
#ifdef __AVX512F__
typedef double v8df __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));

static v8df vector_fma_chain(v8df a, v8df b, v8df c, v8df d, v8df e) {
    /* This may generate RTL with many operands when optimized */
    return __builtin_fma(a, b, __builtin_fma(c, d, e));
}
#endif

/* Strategy 2: Large shuffle operations with constant masks */
static __m256i large_shuffle(__m256i a, __m256i b) {
    /* Shuffle with a large constant mask - each element is an immediate operand */
    return _mm256_shuffle_epi8(a, _mm256_set_epi8(
        31, 30, 29, 28, 27, 26, 25, 24,
        23, 22, 21, 20, 19, 18, 17, 16,
        15, 14, 13, 12, 11, 10, 9, 8,
        7, 6, 5, 4, 3, 2, 1, 0
    ));
}

/* Strategy 3: Inline assembly with exactly 10 and 11 operands */
static void asm_10_operands(void) {
    int64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int64_t in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    int64_t out1, out2;
    
    /* 10 operands: 2 outputs + 8 inputs */
    asm volatile (
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0"
        : "=r"(out1), "=r"(out2)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), 
          "r"(in5), "r"(in6), "r"(in7), "r"(in8)
        : "cc"
    );
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "lea (%1,%2), %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0\n\t"
        "add %10, %0"
        : "=r"(out1)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5),
          "r"(in6), "r"(in7), "r"(in8), "r"(in9), "r"(in10)
        : "cc"
    );
}

/* Strategy 4: AVX-512 gather instruction (many operands) */
#ifdef __AVX512F__
static __m512d avx512_gather_test(__m512i index, double* base) {
    __mmask8 mask = 0xFF;
    return _mm512_mask_i32gather_pd(_mm512_setzero_pd(), mask, index, base, 8);
}
#endif

/* Strategy 5: Complex constant expression that might not fold immediately */
static int complex_constant_expr(void) {
    /* Use __builtin_constant_p to potentially prevent early folding */
    int x = 1;
    if (__builtin_constant_p(x)) {
        /* This branch has a complex constant expression */
        return 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    } else {
        /* Force compiler to consider the constant expression */
        volatile int y = 1;
        return y + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    }
}

/* Strategy 6: Template/Generic approach for C++ */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b, T c, T d, T e) {
    /* Complex expression that might generate many operands */
    return (a + b) * (c - d) + (e * N) + (a / b) - (c % d) + (e << 2) + (a >> 1);
}

/* Instantiate with multiple types */
static void test_templates(void) {
    int i = template_operation<int, 10>(1, 2, 3, 4, 5);
    float f = template_operation<float, 20>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f);
    double d = template_operation<double, 30>(1.0, 2.0, 3.0, 4.0, 5.0);
    
    volatile int vi = i;
    volatile float vf = f;
    volatile double vd = d;
    (void)vi; (void)vf; (void)vd;
}
#endif

/* Main function that exercises all patterns */
int main(void) {
    volatile int result = 0;
    
    /* Test inline assembly patterns */
    asm_10_operands();
    
    /* Test vector operations if supported */
#ifdef __AVX2__
    __m256i vec_a = _mm256_set_epi32(1, 2, 3, 4, 5, 6, 7, 8);
    __m256i vec_b = _mm256_set_epi32(8, 7, 6, 5, 4, 3, 2, 1);
    __m256i vec_c = large_shuffle(vec_a, vec_b);
    result += _mm256_extract_epi32(vec_c, 0);
#endif
    
#ifdef __AVX512F__
    /* Test AVX-512 gather */
    double base_array[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    __m512i indices = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7);
    __m512d gathered = avx512_gather_test(indices, base_array);
    result += (int)_mm512_cvtsd_f64(_mm512_extractf64x2_pd(gathered, 0));
    
    /* Test FMA chain */
    v8df v1 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df v2 = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    v8df v3 = {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    v8df v4 = {4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0};
    v8df v5 = {5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    v8df v_result = vector_fma_chain(v1, v2, v3, v4, v5);
    result += (int)v_result[0];
#endif
    
    /* Test complex constant expression */
    result += complex_constant_expr();
    
#ifdef __cplusplus
    /* Test template instantiations */
    test_templates();
#endif
    
    /* Ensure result is used */
    printf("Result: %d\n", result);
    
    return 0;
}
