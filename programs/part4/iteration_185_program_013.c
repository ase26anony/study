#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>

#ifdef __SSE__
#include <xmmintrin.h>
#endif

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#ifdef __AVX__
#include <avxintrin.h>
#endif

/* Force assembly output by using inline asm with vector operands */
#define FORCE_ASM_OUTPUT(vec) \
    __asm__ __volatile__("" : : "x"(vec) : "memory")

/* Function to prevent dead code elimination */
__attribute__((noinline)) 
int use_result(__m128 v) {
    int mask = _mm_movemask_ps(v);
    return mask & 0xF;
}

__attribute__((noinline))
int use_result_pd(__m128d v) {
    int mask = _mm_movemask_pd(v);
    return mask & 0x3;
}

#ifdef __AVX__
__attribute__((noinline))
int use_result_avx(__m256 v) {
    int mask = _mm256_movemask_ps(v);
    return mask & 0xFF;
}

__attribute__((noinline))
int use_result_avx_pd(__m256d v) {
    int mask = _mm256_movemask_pd(v);
    return mask & 0xF;
}
#endif

/* Test all condition codes from the uncovered block */
void test_sse_condition_codes(void) {
    printf("Testing SSE condition codes...\n");
    
    /* Create vectors with various values including NaN */
    __m128 vec1 = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 vec2 = _mm_setr_ps(1.0f, 3.0f, 3.0f, NAN);
    __m128 vec3 = _mm_setr_ps(0.0f, -0.0f, INFINITY, -INFINITY);
    __m128 vec4 = _mm_setr_ps(5.0f, 5.0f, 5.0f, 5.0f);
    
    __m128 result;
    int final_result = 0;
    
    /* Test each condition code explicitly */
    
    /* 1. UNORDERED (_CMP_UNORD_Q) - unordered (NaN) */
    result = _mm_cmp_ps(vec1, vec2, _CMP_UNORD_Q);
    final_result += use_result(result);
    FORCE_ASM_OUTPUT(result);
    
    /* 2. ORDERED (_CMP_ORD_Q) - ordered (not NaN) */
    result = _mm_cmp_ps(vec3, vec4, _CMP_ORD_Q);
    final_result += use_result(result);
    FORCE_ASM_OUTPUT(result);
    
    /* 3. UNEQ (_CMP_UNEQ_UQ) - unordered or equal */
    result = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    final_result += use_result(result);
    FORCE_ASM_OUTPUT(result);
    
    /* 4. UNGE (_CMP_NGE_UQ) - not greater than or equal (unordered) */
    result = _mm_cmp_ps(vec1, vec4, _CMP_NGE_UQ);
    final_result += use_result(result);
    FORCE_ASM_OUTPUT(result);
    
    /* 5. UNGT (_CMP_NGT_UQ) - not greater than (unordered) */
    result = _mm_cmp_ps(vec3, vec4, _CMP_NGT_UQ);
    final_result += use_result(result);
    FORCE_ASM_OUTPUT(result);
    
    /* 6. UNLE (_CMP_ULE_UQ) - unordered or less than or equal */
    result = _mm_cmp_ps(vec2, vec3, _CMP_ULE_UQ);
    final_result += use_result(result);
    FORCE_ASM_OUTPUT(result);
    
    /* 7. UNLT (_CMP_ULT_UQ) - unordered or less than */
    result = _mm_cmp_ps(vec1, vec4, _CMP_ULT_UQ);
    final_result += use_result(result);
    FORCE_ASM_OUTPUT(result);
    
    /* 8. LTGT (_CMP_NEQ_UQ) - less than or greater than (unordered) */
    result = _mm_cmp_ps(vec3, vec4, _CMP_NEQ_UQ);
    final_result += use_result(result);
    FORCE_ASM_OUTPUT(result);
    
    printf("SSE final result: %d\n", final_result);
}

void test_sse2_double_condition_codes(void) {
    printf("Testing SSE2 double precision condition codes...\n");
    
    __m128d dvec1 = _mm_setr_pd(1.0, NAN);
    __m128d dvec2 = _mm_setr_pd(NAN, 2.0);
    __m128d dvec3 = _mm_setr_pd(3.0, 4.0);
    __m128d dvec4 = _mm_setr_pd(INFINITY, -INFINITY);
    
    __m128d result;
    int final_result = 0;
    
    /* Test with double precision */
    result = _mm_cmp_pd(dvec1, dvec2, _CMP_UNORD_Q);
    final_result += use_result_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm_cmp_pd(dvec3, dvec4, _CMP_ORD_Q);
    final_result += use_result_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm_cmp_pd(dvec1, dvec3, _CMP_UNEQ_UQ);
    final_result += use_result_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm_cmp_pd(dvec2, dvec4, _CMP_NGE_UQ);
    final_result += use_result_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm_cmp_pd(dvec3, dvec4, _CMP_NGT_UQ);
    final_result += use_result_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm_cmp_pd(dvec1, dvec2, _CMP_ULE_UQ);
    final_result += use_result_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm_cmp_pd(dvec3, dvec4, _CMP_ULT_UQ);
    final_result += use_result_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm_cmp_pd(dvec1, dvec3, _CMP_NEQ_UQ);
    final_result += use_result_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    printf("SSE2 double final result: %d\n", final_result);
}

#ifdef __AVX__
void test_avx_condition_codes(void) {
    printf("Testing AVX condition codes...\n");
    
    /* Create 256-bit vectors */
    __m256 avx_vec1 = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, NAN, 7.0f, 8.0f);
    __m256 avx_vec2 = _mm256_setr_ps(1.0f, 3.0f, 3.0f, NAN, 5.0f, 6.0f, NAN, 8.0f);
    __m256 avx_vec3 = _mm256_setr_ps(0.0f, -0.0f, INFINITY, -INFINITY, 9.0f, 10.0f, 11.0f, 12.0f);
    
    __m256 result;
    int final_result = 0;
    
    /* Test AVX variants */
    result = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_UNORD_Q);
    final_result += use_result_avx(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm256_cmp_ps(avx_vec1, avx_vec3, _CMP_ORD_Q);
    final_result += use_result_avx(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm256_cmp_ps(avx_vec2, avx_vec3, _CMP_UNEQ_UQ);
    final_result += use_result_avx(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm256_cmp_ps(avx_vec1, avx_vec3, _CMP_NGE_UQ);
    final_result += use_result_avx(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm256_cmp_ps(avx_vec2, avx_vec3, _CMP_NGT_UQ);
    final_result += use_result_avx(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_ULE_UQ);
    final_result += use_result_avx(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm256_cmp_ps(avx_vec2, avx_vec3, _CMP_ULT_UQ);
    final_result += use_result_avx(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm256_cmp_ps(avx_vec1, avx_vec3, _CMP_NEQ_UQ);
    final_result += use_result_avx(result);
    FORCE_ASM_OUTPUT(result);
    
    printf("AVX final result: %d\n", final_result);
}

void test_avx_double_condition_codes(void) {
    printf("Testing AVX double precision condition codes...\n");
    
    __m256d avx_dvec1 = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d avx_dvec2 = _mm256_setr_pd(NAN, 2.0, 3.0, NAN);
    __m256d avx_dvec3 = _mm256_setr_pd(5.0, 6.0, INFINITY, -INFINITY);
    
    __m256d result;
    int final_result = 0;
    
    result = _mm256_cmp_pd(avx_dvec1, avx_dvec2, _CMP_UNORD_Q);
    final_result += use_result_avx_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm256_cmp_pd(avx_dvec1, avx_dvec3, _CMP_ORD_Q);
    final_result += use_result_avx_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm256_cmp_pd(avx_dvec2, avx_dvec3, _CMP_UNEQ_UQ);
    final_result += use_result_avx_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm256_cmp_pd(avx_dvec1, avx_dvec3, _CMP_NGE_UQ);
    final_result += use_result_avx_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm256_cmp_pd(avx_dvec2, avx_dvec3, _CMP_NGT_UQ);
    final_result += use_result_avx_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm256_cmp_pd(avx_dvec1, avx_dvec2, _CMP_ULE_UQ);
    final_result += use_result_avx_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm256_cmp_pd(avx_dvec2, avx_dvec3, _CMP_ULT_UQ);
    final_result += use_result_avx_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    result = _mm256_cmp_pd(avx_dvec1, avx_dvec3, _CMP_NEQ_UQ);
    final_result += use_result_avx_pd(result);
    FORCE_ASM_OUTPUT(result);
    
    printf("AVX double final result: %d\n", final_result);
}
#endif

/* Test scalar comparisons as well */
void test_scalar_condition_codes(void) {
    printf("Testing scalar condition codes...\n");
    
    float f1 = NAN;
    float f2 = 1.0f;
    double d1 = NAN;
    double d2 = 2.0;
    
    __m128 sresult;
    __m128d dresult;
    int final_result = 0;
    
    /* Scalar single precision */
    sresult = _mm_cmp_ss(_mm_set_ss(f1), _mm_set_ss(f2), _CMP_UNORD_Q);
    final_result += _mm_extract_ps(sresult, 0) != 0;
    FORCE_ASM_OUTPUT(sresult);
    
    sresult = _mm_cmp_ss(_mm_set_ss(f2), _mm_set_ss(f1), _CMP_ORD_Q);
    final_result += _mm_extract_ps(sresult, 0) != 0;
    FORCE_ASM_OUTPUT(sresult);
    
    sresult = _mm_cmp_ss(_mm_set_ss(f1), _mm_set_ss(f2), _CMP_UNEQ_UQ);
    final_result += _mm_extract_ps(sresult, 0) != 0;
    FORCE_ASM_OUTPUT(sresult);
    
    /* Scalar double precision */
    dresult = _mm_cmp_sd(_mm_set_sd(d1), _mm_set_sd(d2), _CMP_NGE_UQ);
    final_result += _mm_extract_epi64(_mm_castpd_si128(dresult), 0) != 0;
    FORCE_ASM_OUTPUT(dresult);
    
    dresult = _mm_cmp_sd(_mm_set_sd(d2), _mm_set_sd(d1), _CMP_NGT_UQ);
    final_result += _mm_extract_epi64(_mm_castpd_si128(dresult), 0) != 0;
    FORCE_ASM_OUTPUT(dresult);
    
    printf("Scalar final result: %d\n", final_result);
}

/* Complex expression mixing comparisons with arithmetic */
__attribute__((noinline))
float complex_vector_expression(__m128 a, __m128 b, __m128 c) {
    /* Blend based on comparison results */
    __m128 cmp1 = _mm_cmp_ps(a, b, _CMP_UNORD_Q);
    __m128 cmp2 = _mm_cmp_ps(b, c, _CMP_ORD_Q);
    __m128 cmp3 = _mm_cmp_ps(a, c, _CMP_UNEQ_UQ);
    
    /* Use comparisons to blend values */
    __m128 blended1 = _mm_blendv_ps(a, b, cmp1);
    __m128 blended2 = _mm_blendv_ps(b, c, cmp2);
    __m128 blended3 = _mm_blendv_ps(blended1, blended2, cmp3);
    
    /* Add some arithmetic */
    __m128 result = _mm_add_ps(blended3, _mm_mul_ps(a, b));
    
    /* Extract and return a scalar */
    float res_arr[4];
    _mm_storeu_ps(res_arr, result);
    return res_arr[0] + res_arr[1] + res_arr[2] + res_arr[3];
}

int main(void) {
    printf("Testing x86 condition code printing logic...\n");
    
    /* Test SSE condition codes */
    test_sse_condition_codes();
    
    /* Test SSE2 double precision */
    test_sse2_double_condition_codes();
    
    /* Test scalar comparisons */
    test_scalar_condition_codes();
    
    /* Test complex expression */
    __m128 vec_a = _mm_setr_ps(1.0f, NAN, 3.0f, 4.0f);
    __m128 vec_b = _mm_setr_ps(NAN, 2.0f, 3.0f, NAN);
    __m128 vec_c = _mm_setr_ps(5.0f, 6.0f, INFINITY, -INFINITY);
    float complex_result = complex_vector_expression(vec_a, vec_b, vec_c);
    printf("Complex expression result: %f\n", complex_result);
    
#ifdef __AVX__
    /* Check if AVX is supported at runtime */
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported, testing AVX condition codes...\n");
        test_avx_condition_codes();
        test_avx_double_condition_codes();
    } else {
        printf("AVX not supported at runtime\n");
    }
#else
    printf("AVX not compiled in\n");
#endif
    
    return 0;
}
