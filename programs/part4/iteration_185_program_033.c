#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>

// Function to force assembly generation and prevent optimization
__attribute__((noinline)) 
int compute_result(__m128 v1, __m128 v2, __m128 v3, __m128 v4) {
    int result = 0;
    
    // Test all condition codes from the uncovered block
    // Using _mm_cmp_ps with explicit condition codes
    
    // 1. UNORDERED (_CMP_UNORD_Q = 3)
    __m128 cmp_unord = _mm_cmp_ps(v1, v2, _CMP_UNORD_Q);
    int mask_unord = _mm_movemask_ps(cmp_unord);
    result += mask_unord;
    
    // 2. ORDERED (_CMP_ORD_Q = 7)
    __m128 cmp_ord = _mm_cmp_ps(v2, v3, _CMP_ORD_Q);
    int mask_ord = _mm_movemask_ps(cmp_ord);
    result += mask_ord * 2;
    
    // 3. UNEQ (_CMP_UNEQ_UQ = 12)
    __m128 cmp_uneq = _mm_cmp_ps(v3, v4, _CMP_UNEQ_UQ);
    int mask_uneq = _mm_movemask_ps(cmp_uneq);
    result += mask_uneq * 3;
    
    // 4. UNGE (_CMP_NGE_UQ = 13)
    __m128 cmp_unge = _mm_cmp_ps(v4, v1, _CMP_NGE_UQ);
    int mask_unge = _mm_movemask_ps(cmp_unge);
    result += mask_unge * 4;
    
    // 5. UNGT (_CMP_NGT_UQ = 14)
    __m128 cmp_ungt = _mm_cmp_ps(v1, v3, _CMP_NGT_UQ);
    int mask_ungt = _mm_movemask_ps(cmp_ungt);
    result += mask_ungt * 5;
    
    // 6. UNLE (_CMP_ULE_UQ = 15)
    __m128 cmp_unle = _mm_cmp_ps(v2, v4, _CMP_ULE_UQ);
    int mask_unle = _mm_movemask_ps(cmp_unle);
    result += mask_unle * 6;
    
    // 7. UNLT (_CMP_ULT_UQ = 16)
    __m128 cmp_unlt = _mm_cmp_ps(v3, v1, _CMP_ULT_UQ);
    int mask_unlt = _mm_movemask_ps(cmp_unlt);
    result += mask_unlt * 7;
    
    // 8. LTGT (_CMP_NEQ_UQ = 4)
    __m128 cmp_ltgt = _mm_cmp_ps(v4, v2, _CMP_NEQ_UQ);
    int mask_ltgt = _mm_movemask_ps(cmp_ltgt);
    result += mask_ltgt * 8;
    
    return result;
}

// Double precision version
__attribute__((noinline))
int compute_result_pd(__m128d v1, __m128d v2, __m128d v3, __m128d v4) {
    int result = 0;
    
    // Test with double precision comparisons
    __m128d cmp_unord = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    int mask_unord = _mm_movemask_pd(cmp_unord);
    result += mask_unord;
    
    __m128d cmp_ord = _mm_cmp_pd(v2, v3, _CMP_ORD_Q);
    int mask_ord = _mm_movemask_pd(cmp_ord);
    result += mask_ord * 2;
    
    __m128d cmp_uneq = _mm_cmp_pd(v3, v4, _CMP_UNEQ_UQ);
    int mask_uneq = _mm_movemask_pd(cmp_uneq);
    result += mask_uneq * 3;
    
    __m128d cmp_unge = _mm_cmp_pd(v4, v1, _CMP_NGE_UQ);
    int mask_unge = _mm_movemask_pd(cmp_unge);
    result += mask_unge * 4;
    
    __m128d cmp_ungt = _mm_cmp_pd(v1, v3, _CMP_NGT_UQ);
    int mask_ungt = _mm_movemask_pd(cmp_ungt);
    result += mask_ungt * 5;
    
    __m128d cmp_unle = _mm_cmp_pd(v2, v4, _CMP_ULE_UQ);
    int mask_unle = _mm_movemask_pd(cmp_unle);
    result += mask_unle * 6;
    
    __m128d cmp_unlt = _mm_cmp_pd(v3, v1, _CMP_ULT_UQ);
    int mask_unlt = _mm_movemask_pd(cmp_unlt);
    result += mask_unlt * 7;
    
    __m128d cmp_ltgt = _mm_cmp_pd(v4, v2, _CMP_NEQ_UQ);
    int mask_ltgt = _mm_movemask_pd(cmp_ltgt);
    result += mask_ltgt * 8;
    
    return result;
}

#ifdef __AVX__
// AVX version for 256-bit vectors
__attribute__((noinline))
int compute_result_avx(__m256 v1, __m256 v2, __m256 v3, __m256 v4) {
    int result = 0;
    
    // Test all condition codes with AVX
    __m256 cmp_unord = _mm256_cmp_ps(v1, v2, _CMP_UNORD_Q);
    int mask_unord = _mm256_movemask_ps(cmp_unord);
    result += mask_unord;
    
    __m256 cmp_ord = _mm256_cmp_ps(v2, v3, _CMP_ORD_Q);
    int mask_ord = _mm256_movemask_ps(cmp_ord);
    result += mask_ord * 2;
    
    __m256 cmp_uneq = _mm256_cmp_ps(v3, v4, _CMP_UNEQ_UQ);
    int mask_uneq = _mm256_movemask_ps(cmp_uneq);
    result += mask_uneq * 3;
    
    __m256 cmp_unge = _mm256_cmp_ps(v4, v1, _CMP_NGE_UQ);
    int mask_unge = _mm256_movemask_ps(cmp_unge);
    result += mask_unge * 4;
    
    __m256 cmp_ungt = _mm256_cmp_ps(v1, v3, _CMP_NGT_UQ);
    int mask_ungt = _mm256_movemask_ps(cmp_ungt);
    result += mask_ungt * 5;
    
    __m256 cmp_unle = _mm256_cmp_ps(v2, v4, _CMP_ULE_UQ);
    int mask_unle = _mm256_movemask_ps(cmp_unle);
    result += mask_unle * 6;
    
    __m256 cmp_unlt = _mm256_cmp_ps(v3, v1, _CMP_ULT_UQ);
    int mask_unlt = _mm256_movemask_ps(cmp_unlt);
    result += mask_unlt * 7;
    
    __m256 cmp_ltgt = _mm256_cmp_ps(v4, v2, _CMP_NEQ_UQ);
    int mask_ltgt = _mm256_movemask_ps(cmp_ltgt);
    result += mask_ltgt * 8;
    
    return result;
}

// AVX double precision
__attribute__((noinline))
int compute_result_avx_pd(__m256d v1, __m256d v2, __m256d v3, __m256d v4) {
    int result = 0;
    
    __m256d cmp_unord = _mm256_cmp_pd(v1, v2, _CMP_UNORD_Q);
    int mask_unord = _mm256_movemask_pd(cmp_unord);
    result += mask_unord;
    
    __m256d cmp_ord = _mm256_cmp_pd(v2, v3, _CMP_ORD_Q);
    int mask_ord = _mm256_movemask_pd(cmp_ord);
    result += mask_ord * 2;
    
    __m256d cmp_uneq = _mm256_cmp_pd(v3, v4, _CMP_UNEQ_UQ);
    int mask_uneq = _mm256_movemask_pd(cmp_uneq);
    result += mask_uneq * 3;
    
    __m256d cmp_unge = _mm256_cmp_pd(v4, v1, _CMP_NGE_UQ);
    int mask_unge = _mm256_movemask_pd(cmp_unge);
    result += mask_unge * 4;
    
    __m256d cmp_ungt = _mm256_cmp_pd(v1, v3, _CMP_NGT_UQ);
    int mask_ungt = _mm256_movemask_pd(cmp_ungt);
    result += mask_ungt * 5;
    
    __m256d cmp_unle = _mm256_cmp_pd(v2, v4, _CMP_ULE_UQ);
    int mask_unle = _mm256_movemask_pd(cmp_unle);
    result += mask_unle * 6;
    
    __m256d cmp_unlt = _mm256_cmp_pd(v3, v1, _CMP_ULT_UQ);
    int mask_unlt = _mm256_movemask_pd(cmp_unlt);
    result += mask_unlt * 7;
    
    __m256d cmp_ltgt = _mm256_cmp_pd(v4, v2, _CMP_NEQ_UQ);
    int mask_ltgt = _mm256_movemask_pd(cmp_ltgt);
    result += mask_ltgt * 8;
    
    return result;
}
#endif

// Scalar comparisons to test _mm_cmp_ss and _mm_cmp_sd
__attribute__((noinline))
int test_scalar_comparisons(float a, float b, double c, double d) {
    int result = 0;
    
    // Scalar single precision
    __m128 sa = _mm_set_ss(a);
    __m128 sb = _mm_set_ss(b);
    
    __m128 cmp_unord_ss = _mm_cmp_ss(sa, sb, _CMP_UNORD_Q);
    result += _mm_cvtss_si32(cmp_unord_ss);
    
    __m128 cmp_ord_ss = _mm_cmp_ss(sb, sa, _CMP_ORD_Q);
    result += _mm_cvtss_si32(cmp_ord_ss) * 2;
    
    __m128 cmp_uneq_ss = _mm_cmp_ss(sa, sb, _CMP_UNEQ_UQ);
    result += _mm_cvtss_si32(cmp_uneq_ss) * 3;
    
    // Scalar double precision
    __m128d sc = _mm_set_sd(c);
    __m128d sd = _mm_set_sd(d);
    
    __m128d cmp_unord_sd = _mm_cmp_sd(sc, sd, _CMP_UNORD_Q);
    result += _mm_cvtsd_si32(cmp_unord_sd);
    
    __m128d cmp_ord_sd = _mm_cmp_sd(sd, sc, _CMP_ORD_Q);
    result += _mm_cvtsd_si32(cmp_ord_sd) * 2;
    
    __m128d cmp_unge_sd = _mm_cmp_sd(sc, sd, _CMP_NGE_UQ);
    result += _mm_cvtsd_si32(cmp_unge_sd) * 4;
    
    return result;
}

// Complex expression mixing comparisons and arithmetic
__attribute__((noinline))
__m128 complex_vector_expr(__m128 a, __m128 b, __m128 c) {
    // Blend based on comparison results
    __m128 cmp1 = _mm_cmp_ps(a, b, _CMP_UNORD_Q);  // UNORDERED
    __m128 cmp2 = _mm_cmp_ps(b, c, _CMP_ORD_Q);    // ORDERED
    __m128 cmp3 = _mm_cmp_ps(c, a, _CMP_UNEQ_UQ);  // UNEQ
    
    // Use comparison results in arithmetic
    __m128 masked1 = _mm_and_ps(cmp1, a);
    __m128 masked2 = _mm_and_ps(cmp2, b);
    __m128 masked3 = _mm_andnot_ps(cmp3, c);
    
    // Blend vectors based on comparison masks
    __m128 result = _mm_add_ps(masked1, masked2);
    result = _mm_sub_ps(result, masked3);
    
    // Additional comparisons
    __m128 cmp4 = _mm_cmp_ps(a, c, _CMP_NGE_UQ);   // UNGE
    __m128 cmp5 = _mm_cmp_ps(b, a, _CMP_NGT_UQ);   // UNGT
    
    __m128 blended = _mm_blendv_ps(result, a, cmp4);
    blended = _mm_blendv_ps(blended, b, cmp5);
    
    return blended;
}

int main() {
    // Initialize vectors with various values including NaN
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 v2 = _mm_setr_ps(2.0f, 1.0f, 3.0f, NAN);
    __m128 v3 = _mm_setr_ps(NAN, 4.0f, 2.0f, 1.0f);
    __m128 v4 = _mm_setr_ps(4.0f, NAN, 1.0f, 2.0f);
    
    __m128d d1 = _mm_setr_pd(1.0, NAN);
    __m128d d2 = _mm_setr_pd(NAN, 2.0);
    __m128d d3 = _mm_setr_pd(3.0, 4.0);
    __m128d d4 = _mm_setr_pd(4.0, 3.0);
    
    int total_result = 0;
    
    // Test SSE single precision
    total_result += compute_result(v1, v2, v3, v4);
    
    // Test SSE double precision
    total_result += compute_result_pd(d1, d2, d3, d4);
    
    // Test scalar comparisons
    total_result += test_scalar_comparisons(1.0f, NAN, 2.0, NAN);
    
    // Test complex expression
    __m128 complex_result = complex_vector_expr(v1, v2, v3);
    float complex_array[4];
    _mm_storeu_ps(complex_array, complex_result);
    total_result += (int)complex_array[0];
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        // Initialize AVX vectors
        __m256 av1 = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, NAN, 8.0f);
        __m256 av2 = _mm256_setr_ps(2.0f, 1.0f, 3.0f, NAN, 6.0f, 5.0f, 7.0f, NAN);
        __m256 av3 = _mm256_setr_ps(NAN, 4.0f, 2.0f, 1.0f, NAN, 8.0f, 6.0f, 5.0f);
        __m256 av4 = _mm256_setr_ps(4.0f, NAN, 1.0f, 2.0f, 8.0f, NAN, 5.0f, 6.0f);
        
        __m256d ad1 = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
        __m256d ad2 = _mm256_setr_pd(NAN, 2.0, 4.0, 3.0);
        __m256d ad3 = _mm256_setr_pd(3.0, 4.0, NAN, 2.0);
        __m256d ad4 = _mm256_setr_pd(4.0, 3.0, 2.0, NAN);
        
        // Test AVX single precision
        total_result += compute_result_avx(av1, av2, av3, av4);
        
        // Test AVX double precision
        total_result += compute_result_avx_pd(ad1, ad2, ad3, ad4);
    }
#endif
    
    // Force assembly output with inline assembly
    __asm__ __volatile__ (
        "# Vector comparison test marker\n"
        : 
        : "r" (total_result)
        : "memory"
    );
    
    printf("Result: %d\n", total_result);
    
    // Additional test to ensure all condition codes are used
    // by creating a switch-like structure that the compiler
    // might optimize differently
    volatile int test_case = 0;
    __m128 test_vec = _mm_set1_ps(1.0f);
    __m128 ref_vec = _mm_set1_ps(2.0f);
    
    switch (test_case) {
        case 0: {
            __m128 r = _mm_cmp_ps(test_vec, ref_vec, _CMP_UNORD_Q);
            _mm_storeu_ps((float*)&total_result, r);
            break;
        }
        case 1: {
            __m128 r = _mm_cmp_ps(test_vec, ref_vec, _CMP_ORD_Q);
            _mm_storeu_ps((float*)&total_result, r);
            break;
        }
        case 2: {
            __m128 r = _mm_cmp_ps(test_vec, ref_vec, _CMP_UNEQ_UQ);
            _mm_storeu_ps((float*)&total_result, r);
            break;
        }
        case 3: {
            __m128 r = _mm_cmp_ps(test_vec, ref_vec, _CMP_NGE_UQ);
            _mm_storeu_ps((float*)&total_result, r);
            break;
        }
        case 4: {
            __m128 r = _mm_cmp_ps(test_vec, ref_vec, _CMP_NGT_UQ);
            _mm_storeu_ps((float*)&total_result, r);
            break;
        }
        case 5: {
            __m128 r = _mm_cmp_ps(test_vec, ref_vec, _CMP_ULE_UQ);
            _mm_storeu_ps((float*)&total_result, r);
            break;
        }
        case 6: {
            __m128 r = _mm_cmp_ps(test_vec, ref_vec, _CMP_ULT_UQ);
            _mm_storeu_ps((float*)&total_result, r);
            break;
        }
        case 7: {
            __m128 r = _mm_cmp_ps(test_vec, ref_vec, _CMP_NEQ_UQ);
            _mm_storeu_ps((float*)&total_result, r);
            break;
        }
    }
    
    return total_result != 0 ? 0 : 1;
}
