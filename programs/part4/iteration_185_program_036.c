/* test_condition_codes.c - Exercise x86 vector comparison condition codes */
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

/* Function to test SSE condition codes */
float test_sse_condition_codes(float a, float b) {
    float result = 0.0f;
    
#ifdef __SSE__
    __m128 vec_a = _mm_set1_ps(a);
    __m128 vec_b = _mm_set1_ps(b);
    __m128 cmp_result;
    int mask;
    
    /* Test all condition codes from the uncovered block */
    
    /* 1. UNORDERED (_CMP_UNORD_Q) - "unord" */
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_UNORD_Q);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 1.0f;
    
    /* 2. ORDERED (_CMP_ORD_Q) - "ord" */
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_ORD_Q);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 2.0f;
    
    /* 3. UNEQ (_CMP_UNEQ_UQ) - "ueq" */
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_UNEQ_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 4.0f;
    
    /* 4. UNGE (_CMP_NGE_UQ) - "nlt" */
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_NGE_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 8.0f;
    
    /* 5. UNGT (_CMP_NGT_UQ) - "nle" */
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_NGT_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 16.0f;
    
    /* 6. UNLE (_CMP_ULE_UQ) - "ule" */
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_ULE_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 32.0f;
    
    /* 7. UNLT (_CMP_ULT_UQ) - "ult" */
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_ULT_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 64.0f;
    
    /* 8. LTGT (_CMP_NEQ_UQ) - "une" */
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_NEQ_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 128.0f;
    
    /* Mix with arithmetic to prevent optimization */
    __m128 add_result = _mm_add_ps(vec_a, cmp_result);
    result += ((float*)&add_result)[0];
#endif
    
    return result;
}

/* Function to test SSE2 double precision condition codes */
double test_sse2_condition_codes(double a, double b) {
    double result = 0.0;
    
#ifdef __SSE2__
    __m128d vec_a = _mm_set1_pd(a);
    __m128d vec_b = _mm_set1_pd(b);
    __m128d cmp_result;
    int mask;
    
    /* Test with double precision */
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 1.0;
    
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 2.0;
    
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_UNEQ_UQ);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 4.0;
    
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_NGE_UQ);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 8.0;
    
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_NGT_UQ);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 16.0;
    
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_ULE_UQ);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 32.0;
    
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_ULT_UQ);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 64.0;
    
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_NEQ_UQ);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 128.0;
    
    /* Use blend to create data-dependent operations */
    __m128d blend_result = _mm_blendv_pd(vec_a, vec_b, cmp_result);
    result += ((double*)&blend_result)[0];
#endif
    
    return result;
}

#ifdef __AVX__
/* Function to test AVX condition codes */
float test_avx_condition_codes(float a, float b) {
    float result = 0.0f;
    
    __m256 vec_a = _mm256_set1_ps(a);
    __m256 vec_b = _mm256_set1_ps(b);
    __m256 cmp_result;
    int mask;
    
    /* Test all condition codes with AVX */
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_UNORD_Q);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 256.0f;
    
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_ORD_Q);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 512.0f;
    
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_UNEQ_UQ);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 1024.0f;
    
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_NGE_UQ);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 2048.0f;
    
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_NGT_UQ);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 4096.0f;
    
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_ULE_UQ);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 8192.0f;
    
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_ULT_UQ);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 16384.0f;
    
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_NEQ_UQ);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 32768.0f;
    
    /* Complex expression with arithmetic */
    __m256 add_result = _mm256_add_ps(vec_a, cmp_result);
    __m256 mul_result = _mm256_mul_ps(add_result, vec_b);
    result += ((float*)&mul_result)[0];
    
    return result;
}

/* Test AVX double precision */
double test_avx_pd_condition_codes(double a, double b) {
    double result = 0.0;
    
    __m256d vec_a = _mm256_set1_pd(a);
    __m256d vec_b = _mm256_set1_pd(b);
    __m256d cmp_result;
    int mask;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 256.0;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 512.0;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_UNEQ_UQ);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 1024.0;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_NGE_UQ);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 2048.0;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_NGT_UQ);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 4096.0;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_ULE_UQ);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 8192.0;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_ULT_UQ);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 16384.0;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_NEQ_UQ);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 32768.0;
    
    return result;
}
#endif

/* Test scalar comparisons (SSE) */
float test_scalar_condition_codes(float a, float b) {
    float result = 0.0f;
    
#ifdef __SSE__
    __m128 vec_a = _mm_set_ss(a);
    __m128 vec_b = _mm_set_ss(b);
    __m128 cmp_result;
    
    /* Test scalar comparisons */
    cmp_result = _mm_cmp_ss(vec_a, vec_b, _CMP_UNORD_Q);
    result += ((float*)&cmp_result)[0];
    
    cmp_result = _mm_cmp_ss(vec_a, vec_b, _CMP_ORD_Q);
    result += ((float*)&cmp_result)[0];
    
    cmp_result = _mm_cmp_ss(vec_a, vec_b, _CMP_UNEQ_UQ);
    result += ((float*)&cmp_result)[0];
    
    cmp_result = _mm_cmp_ss(vec_a, vec_b, _CMP_NGE_UQ);
    result += ((float*)&cmp_result)[0];
    
    cmp_result = _mm_cmp_ss(vec_a, vec_b, _CMP_NGT_UQ);
    result += ((float*)&cmp_result)[0];
    
    cmp_result = _mm_cmp_ss(vec_a, vec_b, _CMP_ULE_UQ);
    result += ((float*)&cmp_result)[0];
    
    cmp_result = _mm_cmp_ss(vec_a, vec_b, _CMP_ULT_UQ);
    result += ((float*)&cmp_result)[0];
    
    cmp_result = _mm_cmp_ss(vec_a, vec_b, _CMP_NEQ_UQ);
    result += ((float*)&cmp_result)[0];
#endif
    
    return result;
}

/* Main function with NaN testing */
int main() {
    float float_values[] = {1.0f, 2.0f, 0.0f, -1.0f, INFINITY, -INFINITY, NAN};
    double double_values[] = {1.0, 2.0, 0.0, -1.0, INFINITY, -INFINITY, NAN};
    
    float float_result = 0.0f;
    double double_result = 0.0;
    
    printf("Testing x86 vector comparison condition codes...\n");
    
    /* Test with various value combinations including NaN */
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            /* Test SSE float comparisons */
            float_result += test_sse_condition_codes(float_values[i], float_values[j]);
            
            /* Test SSE2 double comparisons */
            double_result += test_sse2_condition_codes(double_values[i], double_values[j]);
            
            /* Test scalar comparisons */
            float_result += test_scalar_condition_codes(float_values[i], float_values[j]);
            
#ifdef __AVX__
            /* Test AVX if available */
            if (__builtin_cpu_supports("avx")) {
                float_result += test_avx_condition_codes(float_values[i], float_values[j]);
                double_result += test_avx_pd_condition_codes(double_values[i], double_values[j]);
            }
#endif
        }
    }
    
    /* Force assembly output with inline asm */
#ifdef __SSE__
    __m128 test_vec = _mm_set1_ps(1.0f);
    __m128 cmp_vec = _mm_set1_ps(2.0f);
    __m128 asm_result;
    
    /* Inline assembly that should generate condition code strings */
    __asm__ __volatile__ (
        "vcmpeqps %1, %0, %0\n\t"
        "vcmpltps %1, %0, %0\n\t"
        : "+x" (test_vec)
        : "x" (cmp_vec)
        : "cc"
    );
    
    asm_result = test_vec;
    float_result += ((float*)&asm_result)[0];
#endif
    
    printf("Final results: float=%f, double=%lf\n", float_result, double_result);
    
    /* Use results to prevent dead code elimination */
    if (float_result > 1000000.0f || double_result > 1000000.0) {
        printf("Results are large, continuing...\n");
    }
    
    return 0;
}
