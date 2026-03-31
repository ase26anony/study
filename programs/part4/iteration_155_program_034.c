/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Force usage of results to prevent optimization */
volatile int use_result;

/* Complex vector operations that may generate many operands */
#ifdef __AVX512F__
__m512 test_avx512_fma(__m512 a, __m512 b, __m512 c, __m512 d, __m512 e) {
    /* Chain multiple FMA operations - may generate complex RTL */
    __m512 t1 = _mm512_fmadd_ps(a, b, c);
    __m512 t2 = _mm512_fmadd_ps(d, e, t1);
    __m512 t3 = _mm512_fnmadd_ps(t1, t2, _mm512_set1_ps(1.0f));
    return _mm512_fmadd_ps(t3, a, _mm512_fmadd_ps(b, c, _mm512_fmadd_ps(d, e, t2)));
}
#endif

#ifdef __AVX2__
__m256 test_avx2_complex(__m256 a, __m256 b, __m256 c, __m256 d) {
    /* Complex expression with many vector operands */
    __m256 t1 = _mm256_add_ps(a, b);
    __m256 t2 = _mm256_mul_ps(c, d);
    __m256 t3 = _mm256_sub_ps(t1, t2);
    __m256 t4 = _mm256_add_ps(_mm256_mul_ps(a, c), _mm256_mul_ps(b, d));
    return _mm256_add_ps(t3, _mm256_add_ps(t4, _mm256_set1_ps(3.14159f)));
}
#endif

/* Inline assembly with exactly 10 operands */
void asm_10_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* Extended asm with 10 total operands (1 output + 9 inputs) */
    asm volatile (
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0"
        : "=r" (o0)
        : "r" (i1), "r" (i2), "r" (i3), "r" (i4), 
          "r" (i5), "r" (i6), "r" (i7), "r" (i8), "r" (i9)
        : "cc"
    );
    
    use_result = (int)o0;
}

/* Inline assembly with exactly 11 operands */
void asm_11_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
                 i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* Extended asm with 11 total operands (1 output + 10 inputs) */
    asm volatile (
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
        : "=r" (o0)
        : "r" (i1), "r" (i2), "r" (i3), "r" (i4), "r" (i5),
          "r" (i6), "r" (i7), "r" (i8), "r" (i9), "r" (i10)
        : "cc"
    );
    
    use_result = (int)o0;
}

/* Complex constant expression that might not fold immediately */
int64_t complex_const_expr(void) {
    /* Large constant expression - may generate RTL with many immediates */
    return 1 + (2 * 3) + (4 << 5) + (6 & 7) + (8 | 9) + 
           (10 ^ 11) + (12 - 13) + (14 / 2) + (15 % 4) + 
           (16 * 17) - 18 + 19;
}

/* Vector shuffle with large constant mask */
#ifdef __SSE2__
__m128i test_shuffle(__m128i a, __m128i b) {
    /* Shuffle with 16-byte mask - each byte is an operand */
    const __m128i mask = _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8,
                                      7, 6, 5, 4, 3, 2, 1, 0);
    return _mm_shuffle_epi8(a, mask);
}
#endif

/* Template/generic approach for C++ */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex template operation that might instantiate differently */
    return a + b + static_cast<T>(N) + 
           static_cast<T>(N+1) + static_cast<T>(N+2) +
           static_cast<T>(N+3) + static_cast<T>(N+4) +
           static_cast<T>(N+5) + static_cast<T>(N+6) +
           static_cast<T>(N+7);
}

void test_templates(void) {
    /* Instantiate with different types to increase coverage chance */
    int r1 = template_operation<int, 10>(1, 2);
    float r2 = template_operation<float, 20>(1.0f, 2.0f);
    double r3 = template_operation<double, 30>(1.0, 2.0);
    
    use_result = r1 + (int)r2 + (int)r3;
}
#endif

/* Main function that exercises all patterns */
int main(void) {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Test inline assembly patterns */
    asm_10_operands();
    asm_11_operands();
    
    /* Test complex constant expression */
    int64_t const_result = complex_const_expr();
    use_result = (int)const_result;
    
#ifdef __SSE2__
    /* Test vector shuffle */
    __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    __m128i shuffle_result = test_shuffle(v1, v2);
    use_result = _mm_extract_epi32(shuffle_result, 0);
#endif

#ifdef __AVX2__
    /* Test AVX2 complex operations */
    __m256 avx2_a = _mm256_set1_ps(1.0f);
    __m256 avx2_b = _mm256_set1_ps(2.0f);
    __m256 avx2_c = _mm256_set1_ps(3.0f);
    __m256 avx2_d = _mm256_set1_ps(4.0f);
    __m256 avx2_result = test_avx2_complex(avx2_a, avx2_b, avx2_c, avx2_d);
    use_result = (int)_mm256_cvtss_f32(_mm256_castps256_ps128(avx2_result));
#endif

#ifdef __AVX512F__
    /* Test AVX-512 FMA chaining */
    __m512 avx512_a = _mm512_set1_ps(1.0f);
    __m512 avx512_b = _mm512_set1_ps(2.0f);
    __m512 avx512_c = _mm512_set1_ps(3.0f);
    __m512 avx512_d = _mm512_set1_ps(4.0f);
    __m512 avx512_e = _mm512_set1_ps(5.0f);
    __m512 avx512_result = test_avx512_fma(avx512_a, avx512_b, avx512_c, 
                                           avx512_d, avx512_e);
    use_result = (int)_mm512_cvtss_f32(avx512_result);
#endif

#ifdef __cplusplus
    /* Test template instantiations */
    test_templates();
#endif
    
    printf("Done. Result marker: %d\n", use_result);
    return 0;
}
