/* test_condition_codes.c - Trigger x86 condition code printing in output_operand */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>

/* SSE/AVX comparison condition codes matching the uncovered cases */
#define USE_UNORDERED   _CMP_UNORD_Q    /* UNORDERED */
#define USE_ORDERED     _CMP_ORD_Q      /* ORDERED */
#define USE_UNEQ        _CMP_UNEQ_UQ    /* UNEQ */
#define USE_UNGE        _CMP_NGE_UQ     /* UNGE -> nlt */
#define USE_UNGT        _CMP_NGT_UQ     /* UNGT -> nle */
#define USE_UNLE        _CMP_ULE_UQ     /* UNLE */
#define USE_UNLT        _CMP_ULT_UQ     /* UNLT */
#define USE_LTGT        _CMP_NEQ_UQ     /* LTGT -> une */

/* Force assembly output by using inline asm with vector operands */
static inline void force_asm_output(__m128 v, const char* name) {
    __asm__ __volatile__("# Vector operand %0 named %1" : : "x"(v), "i"(name));
}

static inline void force_asm_output_256(__m256 v, const char* name) {
    __asm__ __volatile__("# AVX operand %0 named %1" : : "x"(v), "i"(name));
}

/* Test all condition codes with SSE (128-bit) */
void test_sse_condition_codes(void) {
    /* Initialize vectors with various values including NaN */
    __m128 vec1 = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 vec2 = _mm_setr_ps(1.0f, 3.0f, 5.0f, NAN);
    __m128 vec3 = _mm_setr_ps(0.0f, -0.0f, INFINITY, -INFINITY);
    __m128 vec4 = _mm_set1_ps(2.5f);
    
    __m128 results[8];
    int masks[8];
    
    /* Test each condition code from the uncovered block */
    results[0] = _mm_cmp_ps(vec1, vec2, USE_UNORDERED);  /* Should generate "unord" */
    results[1] = _mm_cmp_ps(vec1, vec3, USE_ORDERED);    /* Should generate "ord" */
    results[2] = _mm_cmp_ps(vec2, vec4, USE_UNEQ);       /* Should generate "ueq" */
    results[3] = _mm_cmp_ps(vec3, vec1, USE_UNGE);       /* Should generate "nlt" */
    results[4] = _mm_cmp_ps(vec4, vec2, USE_UNGT);       /* Should generate "nle" */
    results[5] = _mm_cmp_ps(vec1, vec4, USE_UNLE);       /* Should generate "ule" */
    results[6] = _mm_cmp_ps(vec3, vec2, USE_UNLT);       /* Should generate "ult" */
    results[7] = _mm_cmp_ps(vec4, vec3, USE_LTGT);       /* Should generate "une" */
    
    /* Extract masks to prevent dead code elimination */
    for (int i = 0; i < 8; i++) {
        masks[i] = _mm_movemask_ps(results[i]);
    }
    
    /* Use results in control flow */
    __m128 blended = _mm_blendv_ps(vec1, vec2, results[0]);
    blended = _mm_add_ps(blended, _mm_and_ps(results[1], vec3));
    
    /* Force assembly output of comparison results */
    force_asm_output(results[0], "unord_result");
    force_asm_output(results[1], "ord_result");
    force_asm_output(results[2], "ueq_result");
    force_asm_output(results[3], "nlt_result");
    force_asm_output(results[4], "nle_result");
    force_asm_output(results[5], "ule_result");
    force_asm_output(results[6], "ult_result");
    force_asm_output(results[7], "une_result");
    
    /* Use masks in conditional logic */
    int final_mask = 0;
    for (int i = 0; i < 8; i++) {
        if (masks[i] != 0) {
            final_mask |= (1 << i);
        }
    }
    
    /* Prevent optimization */
    __asm__ __volatile__("" : : "r"(final_mask));
}

/* Test with double precision (SSE2) */
void test_sse2_double_condition_codes(void) {
    __m128d dvec1 = _mm_setr_pd(1.0, NAN);
    __m128d dvec2 = _mm_setr_pd(NAN, 2.0);
    __m128d dvec3 = _mm_setr_pd(3.0, INFINITY);
    
    __m128d dresults[8];
    
    /* Test all condition codes with double precision */
    dresults[0] = _mm_cmp_pd(dvec1, dvec2, USE_UNORDERED);
    dresults[1] = _mm_cmp_pd(dvec1, dvec3, USE_ORDERED);
    dresults[2] = _mm_cmp_pd(dvec2, dvec3, USE_UNEQ);
    dresults[3] = _mm_cmp_pd(dvec3, dvec1, USE_UNGE);
    dresults[4] = _mm_cmp_pd(dvec1, dvec2, USE_UNGT);
    dresults[5] = _mm_cmp_pd(dvec2, dvec1, USE_UNLE);
    dresults[6] = _mm_cmp_pd(dvec3, dvec2, USE_UNLT);
    dresults[7] = _mm_cmp_pd(dvec1, dvec3, USE_LTGT);
    
    /* Mix with arithmetic */
    __m128d sum = _mm_add_pd(dresults[0], dresults[1]);
    sum = _mm_mul_pd(sum, _mm_or_pd(dresults[2], dresults[3]));
    
    /* Force assembly output */
    __asm__ __volatile__("# Double comparison results" : : "x"(dresults[0]), "x"(dresults[4]));
}

/* Test scalar comparisons (SSE) */
void test_scalar_condition_codes(void) {
    __m128 svec1 = _mm_set_ss(NAN);
    __m128 svec2 = _mm_set_ss(1.0f);
    __m128 svec3 = _mm_set_ss(2.0f);
    
    /* Scalar comparisons should also trigger condition code printing */
    __m128 sresult1 = _mm_cmp_ss(svec1, svec2, USE_UNORDERED);
    __m128 sresult2 = _mm_cmp_ss(svec2, svec3, USE_UNEQ);
    __m128 sresult3 = _mm_cmp_ss(svec3, svec1, USE_ORDERED);
    
    /* Use in arithmetic to prevent elimination */
    __m128 blended_scalar = _mm_blendv_ss(svec1, svec2, sresult1);
    
    __asm__ __volatile__("# Scalar comparison" : : "x"(sresult1), "x"(sresult2));
}

#ifdef __AVX__
/* Test AVX (256-bit) condition codes */
void test_avx_condition_codes(void) {
    __m256 avx_vec1 = _mm256_setr_ps(1.0f, NAN, 3.0f, 4.0f, 5.0f, INFINITY, -INFINITY, 0.0f);
    __m256 avx_vec2 = _mm256_setr_ps(NAN, 2.0f, 3.0f, NAN, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 avx_vec3 = _mm256_set1_ps(2.5f);
    
    __m256 avx_results[8];
    
    /* AVX comparisons with all condition codes */
    avx_results[0] = _mm256_cmp_ps(avx_vec1, avx_vec2, USE_UNORDERED);
    avx_results[1] = _mm256_cmp_ps(avx_vec1, avx_vec3, USE_ORDERED);
    avx_results[2] = _mm256_cmp_ps(avx_vec2, avx_vec3, USE_UNEQ);
    avx_results[3] = _mm256_cmp_ps(avx_vec3, avx_vec1, USE_UNGE);
    avx_results[4] = _mm256_cmp_ps(avx_vec1, avx_vec2, USE_UNGT);
    avx_results[5] = _mm256_cmp_ps(avx_vec2, avx_vec1, USE_UNLE);
    avx_results[6] = _mm256_cmp_ps(avx_vec3, avx_vec2, USE_UNLT);
    avx_results[7] = _mm256_cmp_ps(avx_vec1, avx_vec3, USE_LTGT);
    
    /* Complex expression with blending and arithmetic */
    __m256 avx_blended = _mm256_blendv_ps(avx_vec1, avx_vec2, avx_results[0]);
    avx_blended = _mm256_add_ps(avx_blended, _mm256_and_ps(avx_results[1], avx_vec3));
    avx_blended = _mm256_mul_ps(avx_blended, _mm256_or_ps(avx_results[2], avx_results[3]));
    
    /* Force AVX assembly output */
    force_asm_output_256(avx_results[0], "avx_unord");
    force_asm_output_256(avx_results[1], "avx_ord");
    force_asm_output_256(avx_results[4], "avx_nle");
    
    /* Extract and use masks */
    int avx_mask = _mm256_movemask_ps(avx_results[0]);
    if (avx_mask) {
        __asm__ __volatile__("# AVX mask used" : : "r"(avx_mask));
    }
}

/* Test AVX double precision */
void test_avx_double_condition_codes(void) {
    __m256d avx_dvec1 = _mm256_setr_pd(1.0, NAN, INFINITY, -INFINITY);
    __m256d avx_dvec2 = _mm256_setr_pd(NAN, 2.0, 3.0, NAN);
    
    __m256d avx_dresults[4];
    
    avx_dresults[0] = _mm256_cmp_pd(avx_dvec1, avx_dvec2, USE_UNORDERED);
    avx_dresults[1] = _mm256_cmp_pd(avx_dvec1, avx_dvec2, USE_ORDERED);
    avx_dresults[2] = _mm256_cmp_pd(avx_dvec1, avx_dvec2, USE_UNEQ);
    avx_dresults[3] = _mm256_cmp_pd(avx_dvec1, avx_dvec2, USE_LTGT);
    
    __asm__ __volatile__("# AVX double comparisons" : : "x"(avx_dresults[0]), "x"(avx_dresults[3]));
}
#endif

/* Main function with runtime feature detection */
int main(void) {
    printf("Testing x86 condition code printing...\n");
    
    /* Always test SSE paths */
    test_sse_condition_codes();
    test_sse2_double_condition_codes();
    test_scalar_condition_codes();
    
    /* Test AVX if supported at compile time */
#ifdef __AVX__
    printf("AVX supported - testing AVX condition codes\n");
    test_avx_condition_codes();
    test_avx_double_condition_codes();
#else
    printf("AVX not supported at compile time\n");
#endif
    
    /* Runtime check for AVX (example) */
    int avx_supported = 0;
#ifdef __x86_64__
    unsigned int eax, ebx, ecx, edx;
    eax = 1;
    __asm__ __volatile__(
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "0"(eax)
    );
    avx_supported = (ecx & (1 << 28)) != 0;
#endif
    
    printf("Condition code test complete. AVX runtime support: %s\n", 
           avx_supported ? "yes" : "no");
    
    /* Return value based on some comparison result to prevent optimization */
    __m128 test_vec = _mm_set1_ps(1.0f);
    __m128 cmp_result = _mm_cmp_ps(test_vec, test_vec, USE_UNEQ);
    int mask = _mm_movemask_ps(cmp_result);
    
    return mask == 0xF ? 0 : 1;
}
