#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#ifdef __AVX__
#include <avxintrin.h>
#endif

/* Force assembly generation by using volatile and preventing optimization */
static volatile int global_counter = 0;

/* Function to prevent dead code elimination */
__attribute__((noinline)) 
void use_result(__m128 v) {
    float f[4];
    _mm_storeu_ps(f, v);
    global_counter += (int)f[0] + (int)f[1] + (int)f[2] + (int)f[3];
}

#ifdef __AVX__
__attribute__((noinline))
void use_result_avx(__m256 v) {
    float f[8];
    _mm256_storeu_ps(f, v);
    for (int i = 0; i < 8; i++) {
        global_counter += (int)f[i];
    }
}
#endif

/* Test all SSE condition codes */
void test_sse_condition_codes(void) {
    /* Create vectors with mixed values including NaN */
    __m128 vec1 = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 vec2 = _mm_setr_ps(1.0f, 3.0f, 3.0f, NAN);
    __m128 vec3 = _mm_setr_ps(0.0f, INFINITY, -INFINITY, 5.0f);
    __m128 vec4 = _mm_setr_ps(0.0f, INFINITY, INFINITY, 5.0f);
    
    __m128 result;
    
    /* Test each condition code from the uncovered block */
    
    /* 1. UNORDERED - _CMP_UNORD_Q */
    result = _mm_cmp_ps(vec1, vec2, _CMP_UNORD_Q);
    use_result(result);
    
    /* 2. ORDERED - _CMP_ORD_Q */
    result = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    use_result(result);
    
    /* 3. UNEQ - _CMP_UNEQ_UQ */
    result = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    use_result(result);
    
    /* 4. UNGE - _CMP_NGE_UQ */
    result = _mm_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    use_result(result);
    
    /* 5. UNGT - _CMP_NGT_UQ */
    result = _mm_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    use_result(result);
    
    /* 6. UNLE - _CMP_ULE_UQ */
    result = _mm_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    use_result(result);
    
    /* 7. UNLT - _CMP_ULT_UQ */
    result = _mm_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    use_result(result);
    
    /* 8. LTGT - _CMP_NEQ_UQ */
    result = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    use_result(result);
    
    /* Test scalar versions too */
    __m128 s1 = _mm_set_ss(1.0f);
    __m128 s2 = _mm_set_ss(NAN);
    
    result = _mm_cmp_ss(s1, s2, _CMP_UNORD_Q);
    use_result(result);
    result = _mm_cmp_ss(s1, s2, _CMP_ORD_Q);
    use_result(result);
    
    /* Double precision tests */
    __m128d dvec1 = _mm_setr_pd(1.0, NAN);
    __m128d dvec2 = _mm_setr_pd(2.0, 2.0);
    
    __m128d dresult;
    dresult = _mm_cmp_pd(dvec1, dvec2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)&result, dresult);
    use_result(result);
    
    dresult = _mm_cmp_pd(dvec1, dvec2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)&result, dresult);
    use_result(result);
    
    dresult = _mm_cmp_pd(dvec1, dvec2, _CMP_UNEQ_UQ);
    _mm_storeu_pd((double*)&result, dresult);
    use_result(result);
    
    /* Create control flow based on comparison results */
    int mask = _mm_movemask_ps(_mm_cmp_ps(vec3, vec4, _CMP_UNORD_Q));
    if (mask != 0) {
        global_counter += mask;
    }
    
    /* Blend based on comparison results */
    __m128 blend_result = _mm_blendv_ps(vec1, vec2, 
                                       _mm_cmp_ps(vec1, vec2, _CMP_ULE_UQ));
    use_result(blend_result);
}

#ifdef __AVX__
/* Test AVX variants */
void test_avx_condition_codes(void) {
    /* AVX vectors with mixed values */
    __m256 avx_vec1 = _mm256_setr_ps(1.0f, NAN, 3.0f, 4.0f, 5.0f, NAN, 7.0f, 8.0f);
    __m256 avx_vec2 = _mm256_setr_ps(1.0f, 2.0f, NAN, NAN, 5.0f, 6.0f, 7.0f, 9.0f);
    
    __m256 avx_result;
    
    /* Test all condition codes with AVX */
    avx_result = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_UNORD_Q);
    use_result_avx(avx_result);
    
    avx_result = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_ORD_Q);
    use_result_avx(avx_result);
    
    avx_result = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_UNEQ_UQ);
    use_result_avx(avx_result);
    
    avx_result = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_NGE_UQ);
    use_result_avx(avx_result);
    
    avx_result = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_NGT_UQ);
    use_result_avx(avx_result);
    
    avx_result = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_ULE_UQ);
    use_result_avx(avx_result);
    
    avx_result = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_ULT_UQ);
    use_result_avx(avx_result);
    
    avx_result = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_NEQ_UQ);
    use_result_avx(avx_result);
    
    /* AVX double precision */
    __m256d avx_dvec1 = _mm256_setr_pd(1.0, NAN, 3.0, NAN);
    __m256d avx_dvec2 = _mm256_setr_pd(1.0, 2.0, NAN, 4.0);
    
    __m256d avx_dresult;
    avx_dresult = _mm256_cmp_pd(avx_dvec1, avx_dvec2, _CMP_UNORD_Q);
    float temp[8];
    _mm256_storeu_ps(temp, _mm256_castpd_ps(avx_dresult));
    for (int i = 0; i < 8; i++) global_counter += (int)temp[i];
    
    /* Complex expression with multiple comparisons */
    __m256 cmp1 = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_ORD_Q);
    __m256 cmp2 = _mm256_cmp_ps(avx_vec1, _mm256_set1_ps(0.0f), _CMP_GT_OQ);
    __m256 combined = _mm256_and_ps(cmp1, cmp2);
    use_result_avx(combined);
    
    /* Extract mask and branch */
    int avx_mask = _mm256_movemask_ps(_mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_UNEQ_UQ));
    if (avx_mask & 0x0F) {
        global_counter += 1000;
    }
}
#endif

/* Force inline assembly with vector comparisons */
void test_inline_asm(void) {
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 v2 = _mm_setr_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 result;
    
    /* Inline assembly that should generate condition codes */
    asm volatile (
        "vcmpps %[res], %[v1], %[v2], %{unord%}\n\t"
        : [res] "=x" (result)
        : [v1] "x" (v1), [v2] "x" (v2)
    );
    use_result(result);
    
    asm volatile (
        "vcmpps %[res], %[v1], %[v2], %{ord%}\n\t"
        : [res] "=x" (result)
        : [v1] "x" (v1), [v2] "x" (v2)
    );
    use_result(result);
}

int main(void) {
    printf("Testing x86 condition code generation...\n");
    
    /* Test SSE condition codes */
    test_sse_condition_codes();
    
#ifdef __AVX__
    /* Test AVX if supported at compile time */
    test_avx_condition_codes();
#endif
    
    /* Test inline assembly */
    test_inline_asm();
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed. Check generated assembly for condition codes:\n");
    printf("  unord, ord, ueq, nlt, nle, ule, ult, une\n");
    
    return 0;
}
