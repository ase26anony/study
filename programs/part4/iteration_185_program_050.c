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

/* Function to test SSE condition codes */
float test_sse_conditions(void) {
    __m128 a = _mm_setr_ps(1.0f, 2.0f, NAN, INFINITY);
    __m128 b = _mm_setr_ps(2.0f, 2.0f, 3.0f, INFINITY);
    __m128 c = _mm_setr_ps(0.0f, 0.0f, 0.0f, 0.0f);
    
    /* Test all condition codes from uncovered block */
    __m128 cmp_unord = _mm_cmp_ps(a, b, _CMP_UNORD_Q);    /* UNORDERED */
    __m128 cmp_ord   = _mm_cmp_ps(a, b, _CMP_ORD_Q);      /* ORDERED */
    __m128 cmp_uneq  = _mm_cmp_ps(a, b, _CMP_UNEQ_UQ);    /* UNEQ */
    __m128 cmp_unge  = _mm_cmp_ps(a, b, _CMP_NGE_UQ);     /* UNGE */
    __m128 cmp_ungt  = _mm_cmp_ps(a, b, _CMP_NGT_UQ);     /* UNGT */
    __m128 cmp_unle  = _mm_cmp_ps(a, b, _CMP_ULE_UQ);     /* UNLE */
    __m128 cmp_unlt  = _mm_cmp_ps(a, b, _CMP_ULT_UQ);     /* UNLT */
    __m128 cmp_ltgt  = _mm_cmp_ps(a, b, _CMP_NEQ_UQ);     /* LTGT */
    
    /* Use results to prevent dead code elimination */
    __m128 blend1 = _mm_blendv_ps(a, b, cmp_unord);
    __m128 blend2 = _mm_blendv_ps(blend1, c, cmp_ord);
    __m128 blend3 = _mm_blendv_ps(blend2, a, cmp_uneq);
    __m128 blend4 = _mm_blendv_ps(blend3, b, cmp_unge);
    __m128 blend5 = _mm_blendv_ps(blend4, c, cmp_ungt);
    __m128 blend6 = _mm_blendv_ps(blend5, a, cmp_unle);
    __m128 blend7 = _mm_blendv_ps(blend6, b, cmp_unlt);
    __m128 final  = _mm_blendv_ps(blend7, c, cmp_ltgt);
    
    /* Extract mask and use in control flow */
    int mask_unord = _mm_movemask_ps(cmp_unord);
    int mask_ord   = _mm_movemask_ps(cmp_ord);
    int mask_uneq  = _mm_movemask_ps(cmp_uneq);
    
    float result = 0.0f;
    if (mask_unord & 1) result += 1.0f;
    if (mask_ord & 2)   result += 2.0f;
    if (mask_uneq & 4)  result += 3.0f;
    
    /* Force assembly output with inline asm */
    __asm__ __volatile__("# SSE comparison results: %0" : : "x"(final));
    
    float final_result;
    _mm_store_ss(&final_result, final);
    return final_result + result;
}

/* Double precision version */
double test_sse_double_conditions(void) {
    __m128d a = _mm_setr_pd(1.0, NAN);
    __m128d b = _mm_setr_pd(2.0, 3.0);
    
    __m128d cmp_unord = _mm_cmp_pd(a, b, _CMP_UNORD_Q);
    __m128d cmp_ord   = _mm_cmp_pd(a, b, _CMP_ORD_Q);
    __m128d cmp_uneq  = _mm_cmp_pd(a, b, _CMP_UNEQ_UQ);
    __m128d cmp_unge  = _mm_cmp_pd(a, b, _CMP_NGE_UQ);
    __m128d cmp_ungt  = _mm_cmp_pd(a, b, _CMP_NGT_UQ);
    __m128d cmp_unle  = _mm_cmp_pd(a, b, _CMP_ULE_UQ);
    __m128d cmp_unlt  = _mm_cmp_pd(a, b, _CMP_ULT_UQ);
    __m128d cmp_ltgt  = _mm_cmp_pd(a, b, _CMP_NEQ_UQ);
    
    /* Combine results */
    __m128d blend1 = _mm_blendv_pd(a, b, cmp_unord);
    __m128d blend2 = _mm_blendv_pd(blend1, a, cmp_ord);
    __m128d blend3 = _mm_blendv_pd(blend2, b, cmp_uneq);
    __m128d final  = _mm_add_pd(blend3, cmp_unge);
    
    double result[2];
    _mm_store_pd(result, final);
    return result[0] + result[1];
}

#ifdef __AVX__
/* AVX 256-bit version */
float test_avx_conditions(void) {
    __m256 a = _mm256_setr_ps(1.0f, 2.0f, NAN, INFINITY, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 b = _mm256_setr_ps(2.0f, 2.0f, 3.0f, INFINITY, 5.0f, 7.0f, 7.0f, 9.0f);
    
    __m256 cmp_unord = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);
    __m256 cmp_ord   = _mm256_cmp_ps(a, b, _CMP_ORD_Q);
    __m256 cmp_uneq  = _mm256_cmp_ps(a, b, _CMP_UNEQ_UQ);
    __m256 cmp_unge  = _mm256_cmp_ps(a, b, _CMP_NGE_UQ);
    __m256 cmp_ungt  = _mm256_cmp_ps(a, b, _CMP_NGT_UQ);
    __m256 cmp_unle  = _mm256_cmp_ps(a, b, _CMP_ULE_UQ);
    __m256 cmp_unlt  = _mm256_cmp_ps(a, b, _CMP_ULT_UQ);
    __m256 cmp_ltgt  = _mm256_cmp_ps(a, b, _CMP_NEQ_UQ);
    
    /* Complex expression to force decomposition */
    __m256 t1 = _mm256_and_ps(cmp_unord, a);
    __m256 t2 = _mm256_andnot_ps(cmp_ord, b);
    __m256 t3 = _mm256_add_ps(t1, t2);
    __m256 t4 = _mm256_mul_ps(cmp_uneq, t3);
    __m256 t5 = _mm256_sub_ps(t4, cmp_unge);
    __m256 t6 = _mm256_div_ps(t5, _mm256_add_ps(cmp_ungt, _mm256_set1_ps(1.0f)));
    
    /* Extract masks for control flow */
    int mask_unord = _mm256_movemask_ps(cmp_unord);
    int mask_ltgt  = _mm256_movemask_ps(cmp_ltgt);
    
    float result = 0.0f;
    if (mask_unord) result += 10.0f;
    if (mask_ltgt)  result += 20.0f;
    
    /* Force assembly generation */
    __asm__ __volatile__("# AVX comparison mask: %0 %1" : : "r"(mask_unord), "r"(mask_ltgt));
    
    float final_result[8];
    _mm256_storeu_ps(final_result, t6);
    return final_result[0] + final_result[4] + result;
}

/* AVX double precision */
double test_avx_double_conditions(void) {
    __m256d a = _mm256_setr_pd(1.0, NAN, 3.0, INFINITY);
    __m256d b = _mm256_setr_pd(2.0, 3.0, 3.0, 4.0);
    
    __m256d cmp_unord = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
    __m256d cmp_ord   = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
    __m256d cmp_uneq  = _mm256_cmp_pd(a, b, _CMP_UNEQ_UQ);
    __m256d cmp_unge  = _mm256_cmp_pd(a, b, _CMP_NGE_UQ);
    __m256d cmp_ungt  = _mm256_cmp_pd(a, b, _CMP_NGT_UQ);
    __m256d cmp_unle  = _mm256_cmp_pd(a, b, _CMP_ULE_UQ);
    __m256d cmp_unlt  = _mm256_cmp_pd(a, b, _CMP_ULT_UQ);
    __m256d cmp_ltgt  = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
    
    /* Use in arithmetic to prevent elimination */
    __m256d sum = _mm256_add_pd(cmp_unord, cmp_ord);
    sum = _mm256_add_pd(sum, cmp_uneq);
    sum = _mm256_add_pd(sum, cmp_unge);
    sum = _mm256_add_pd(sum, cmp_ungt);
    sum = _mm256_add_pd(sum, cmp_unle);
    sum = _mm256_add_pd(sum, cmp_unlt);
    sum = _mm256_add_pd(sum, cmp_ltgt);
    
    double result[4];
    _mm256_storeu_pd(result, sum);
    return result[0] + result[1] + result[2] + result[3];
}
#endif

/* Scalar comparisons to test _mm_cmp_ss/sd */
float test_scalar_conditions(void) {
    __m128 a = _mm_set_ss(NAN);
    __m128 b = _mm_set_ss(1.0f);
    
    __m128 cmp_unord = _mm_cmp_ss(a, b, _CMP_UNORD_Q);
    __m128 cmp_ord   = _mm_cmp_ss(a, b, _CMP_ORD_Q);
    __m128 cmp_uneq  = _mm_cmp_ss(a, b, _CMP_UNEQ_UQ);
    
    /* Chain operations */
    __m128 t1 = _mm_add_ss(a, cmp_unord);
    __m128 t2 = _mm_mul_ss(t1, cmp_ord);
    __m128 final = _mm_sub_ss(t2, cmp_uneq);
    
    float result;
    _mm_store_ss(&result, final);
    return result;
}

int main(void) {
    float sse_result = 0.0f;
    double dbl_result = 0.0;
    
    printf("Testing SSE condition codes...\n");
    
    /* Multiple calls to ensure code generation */
    for (int i = 0; i < 3; i++) {
        sse_result += test_sse_conditions();
        dbl_result += test_sse_double_conditions();
        sse_result += test_scalar_conditions();
    }
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        printf("Testing AVX condition codes...\n");
        float avx_result = 0.0f;
        double avx_dbl_result = 0.0;
        
        for (int i = 0; i < 2; i++) {
            avx_result += test_avx_conditions();
            avx_dbl_result += test_avx_double_conditions();
        }
        
        sse_result += avx_result;
        dbl_result += avx_dbl_result;
        
        /* Force conditional assembly generation */
        __asm__ __volatile__("# AVX path executed: %0 %1" 
                           : : "f"(avx_result), "f"(avx_dbl_result));
    }
#endif
    
    /* Use results to prevent optimization */
    printf("Final results: %f, %f\n", sse_result, dbl_result);
    
    /* Additional inline assembly to force condition code printing */
    __m128 test_vec = _mm_set1_ps(1.0f);
    __m128 cmp_result;
    
    __asm__ __volatile__(
        "vcmpeqps %1, %0, %0\n\t"
        "vcmpltps %1, %0, %0\n\t"
        : "+x"(test_vec) : "x"(test_vec)
    );
    
    return (sse_result > 0.0f) ? 0 : 1;
}
