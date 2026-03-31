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

// Function to test SSE comparisons with all condition codes
float test_sse_comparisons(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(b, c, d, a);
    __m128 vec_nan = _mm_set1_ps(NAN);
    __m128 vec_inf = _mm_set1_ps(INFINITY);
    
    // Results accumulator
    __m128 result = _mm_setzero_ps();
    
    // Test all condition codes from the uncovered block
    // Using volatile to prevent optimization
    volatile __m128 cmp_result;
    
    // 1. UNORDERED (handles NaN comparisons)
    cmp_result = _mm_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(1.0f)));
    
    // 2. ORDERED
    cmp_result = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(2.0f)));
    
    // 3. UNEQ (unordered or equal)
    cmp_result = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(3.0f)));
    
    // 4. UNGE (not less than, unordered)
    cmp_result = _mm_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(4.0f)));
    
    // 5. UNGT (not less than or equal, unordered)
    cmp_result = _mm_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(5.0f)));
    
    // 6. UNLE (unordered or less than or equal)
    cmp_result = _mm_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(6.0f)));
    
    // 7. UNLT (unordered or less than)
    cmp_result = _mm_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(7.0f)));
    
    // 8. LTGT (not equal, unordered)
    cmp_result = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(8.0f)));
    
    // Extract and return sum
    float res_arr[4];
    _mm_store_ps(res_arr, result);
    return res_arr[0] + res_arr[1] + res_arr[2] + res_arr[3];
}

// Double precision version
double test_sse2_comparisons(double a, double b) {
    __m128d vec1 = _mm_set_pd(a, b);
    __m128d vec2 = _mm_set_pd(b, a);
    __m128d vec_nan = _mm_set1_pd(NAN);
    
    __m128d result = _mm_setzero_pd();
    volatile __m128d cmp_result;
    
    // Test with double precision
    cmp_result = _mm_cmp_pd(vec1, vec_nan, _CMP_UNORD_Q);
    result = _mm_add_pd(result, _mm_and_pd(cmp_result, _mm_set1_pd(1.0)));
    
    cmp_result = _mm_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    result = _mm_add_pd(result, _mm_and_pd(cmp_result, _mm_set1_pd(2.0)));
    
    cmp_result = _mm_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    result = _mm_add_pd(result, _mm_and_pd(cmp_result, _mm_set1_pd(3.0)));
    
    cmp_result = _mm_cmp_pd(vec1, vec2, _CMP_NGE_UQ);
    result = _mm_add_pd(result, _mm_and_pd(cmp_result, _mm_set1_pd(4.0)));
    
    // Extract result
    double res_arr[2];
    _mm_store_pd(res_arr, result);
    return res_arr[0] + res_arr[1];
}

#ifdef __AVX__
// AVX version with 256-bit vectors
float test_avx_comparisons(float a, float b, float c, float d,
                          float e, float f, float g, float h) {
    __m256 vec1 = _mm256_set_ps(a, b, c, d, e, f, g, h);
    __m256 vec2 = _mm256_set_ps(b, c, d, e, f, g, h, a);
    __m256 vec_nan = _mm256_set1_ps(NAN);
    
    __m256 result = _mm256_setzero_ps();
    volatile __m256 cmp_result;
    
    // Test all condition codes with AVX
    cmp_result = _mm256_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(1.0f)));
    
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(2.0f)));
    
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(3.0f)));
    
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(4.0f)));
    
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(5.0f)));
    
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(6.0f)));
    
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(7.0f)));
    
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(8.0f)));
    
    // Extract result
    float res_arr[8];
    _mm256_store_ps(res_arr, result);
    float sum = 0;
    for (int i = 0; i < 8; i++) sum += res_arr[i];
    return sum;
}
#endif

// Function using inline assembly to force condition code printing
void force_asm_output(float a, float b) {
    __m128 v1 = _mm_set1_ps(a);
    __m128 v2 = _mm_set1_ps(b);
    __m128 result;
    
    // Use inline assembly with vector comparisons
    // This forces the compiler to generate assembly with condition codes
    asm volatile (
        "vcmpeqps %1, %0, %0\n\t"
        "vcmpltps %1, %0, %0\n\t"
        "vcmpunordps %1, %0, %0\n\t"
        : "+x" (v1)
        : "x" (v2)
        : "cc"
    );
    
    // Store to prevent optimization
    float res[4];
    _mm_store_ps(res, v1);
    printf("ASM result: %f\n", res[0]);
}

// Complex expression with blending based on comparisons
__m128 complex_expression(__m128 a, __m128 b, __m128 c) {
    // Multiple comparisons with different condition codes
    __m128 cmp1 = _mm_cmp_ps(a, b, _CMP_UNORD_Q);    // UNORDERED
    __m128 cmp2 = _mm_cmp_ps(a, c, _CMP_ORD_Q);      // ORDERED
    __m128 cmp3 = _mm_cmp_ps(b, c, _CMP_UNEQ_UQ);    // UNEQ
    __m128 cmp4 = _mm_cmp_ps(c, a, _CMP_NGE_UQ);     // UNGE
    
    // Blend results based on comparisons
    __m128 result = _mm_blendv_ps(a, b, cmp1);
    result = _mm_blendv_ps(result, c, cmp2);
    result = _mm_add_ps(result, _mm_and_ps(cmp3, _mm_set1_ps(1.0f)));
    result = _mm_sub_ps(result, _mm_and_ps(cmp4, _mm_set1_ps(2.0f)));
    
    return result;
}

int main() {
    // Initialize with various values including NaN and Inf
    float values[] = {1.0f, 2.0f, NAN, INFINITY, 0.0f, -1.0f, 3.14f, -2.71f};
    double dvalues[] = {1.0, NAN, INFINITY, -INFINITY};
    
    printf("Testing SSE comparisons...\n");
    float sse_result = 0;
    for (int i = 0; i < 4; i++) {
        sse_result += test_sse_comparisons(
            values[i], values[i+1], values[i+2], values[i+3]
        );
    }
    printf("SSE result: %f\n", sse_result);
    
    printf("Testing SSE2 double comparisons...\n");
    double sse2_result = 0;
    for (int i = 0; i < 2; i++) {
        sse2_result += test_sse2_comparisons(dvalues[i], dvalues[i+1]);
    }
    printf("SSE2 result: %f\n", sse2_result);
    
#ifdef __AVX__
    printf("Testing AVX comparisons...\n");
    float avx_result = test_avx_comparisons(
        values[0], values[1], values[2], values[3],
        values[4], values[5], values[6], values[7]
    );
    printf("AVX result: %f\n", avx_result);
#endif
    
    // Test complex expressions
    printf("Testing complex expressions...\n");
    __m128 a = _mm_set_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 b = _mm_set_ps(NAN, 1.0f, 3.0f, INFINITY);
    __m128 c = _mm_set_ps(0.0f, NAN, INFINITY, -INFINITY);
    
    __m128 complex_result = complex_expression(a, b, c);
    float cres[4];
    _mm_store_ps(cres, complex_result);
    printf("Complex result: %f %f %f %f\n", cres[0], cres[1], cres[2], cres[3]);
    
    // Force assembly output
    force_asm_output(1.0f, 2.0f);
    
    // Use movemask to create control flow
    __m128 cmp = _mm_cmp_ps(a, b, _CMP_UNORD_Q);
    int mask = _mm_movemask_ps(cmp);
    if (mask != 0) {
        printf("NaN detected in comparison! Mask: %d\n", mask);
    }
    
    // Test scalar comparisons too
    float scalar_result = 0;
    for (int i = 0; i < 8; i++) {
        __m128 v1 = _mm_set1_ps(values[i]);
        __m128 v2 = _mm_set1_ps(values[(i+1)%8]);
        
        // Mix different condition codes
        int cond_code = i % 8;
        switch (cond_code) {
            case 0: scalar_result += _mm_cvtss_f32(_mm_cmp_ss(v1, v2, _CMP_UNORD_Q)); break;
            case 1: scalar_result += _mm_cvtss_f32(_mm_cmp_ss(v1, v2, _CMP_ORD_Q)); break;
            case 2: scalar_result += _mm_cvtss_f32(_mm_cmp_ss(v1, v2, _CMP_UNEQ_UQ)); break;
            case 3: scalar_result += _mm_cvtss_f32(_mm_cmp_ss(v1, v2, _CMP_NGE_UQ)); break;
            case 4: scalar_result += _mm_cvtss_f32(_mm_cmp_ss(v1, v2, _CMP_NGT_UQ)); break;
            case 5: scalar_result += _mm_cvtss_f32(_mm_cmp_ss(v1, v2, _CMP_ULE_UQ)); break;
            case 6: scalar_result += _mm_cvtss_f32(_mm_cmp_ss(v1, v2, _CMP_ULT_UQ)); break;
            case 7: scalar_result += _mm_cvtss_f32(_mm_cmp_ss(v1, v2, _CMP_NEQ_UQ)); break;
        }
    }
    printf("Scalar comparison result: %f\n", scalar_result);
    
    return 0;
}
