#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE
#endif

// Function to test SSE comparisons with all condition codes
FORCE_INLINE float test_sse_comparisons(float a, float b, float c, float d) {
    __m128 v1 = _mm_set_ps(a, b, c, d);
    __m128 v2 = _mm_set_ps(d, c, b, a);
    __m128 v_nan = _mm_set1_ps(NAN);
    __m128 v_inf = _mm_set1_ps(INFINITY);
    
    // Store results to prevent optimization
    __m128 results[16];
    int idx = 0;
    
    // Test all condition codes from the uncovered block
    // UNORDERED
    results[idx++] = _mm_cmp_ps(v_nan, v1, _CMP_UNORD_Q);
    
    // ORDERED
    results[idx++] = _mm_cmp_ps(v1, v2, _CMP_ORD_Q);
    
    // UNEQ
    results[idx++] = _mm_cmp_ps(v1, v2, _CMP_UNEQ_UQ);
    
    // UNGE (maps to nlt)
    results[idx++] = _mm_cmp_ps(v1, v_inf, _CMP_NGE_UQ);
    
    // UNGT (maps to nle)
    results[idx++] = _mm_cmp_ps(v_inf, v1, _CMP_NGT_UQ);
    
    // UNLE
    results[idx++] = _mm_cmp_ps(v1, v2, _CMP_ULE_UQ);
    
    // UNLT
    results[idx++] = _mm_cmp_ps(v2, v1, _CMP_ULT_UQ);
    
    // LTGT (maps to une)
    results[idx++] = _mm_cmp_ps(v1, v2, _CMP_NEQ_UQ);
    
    // Also test scalar versions
    __m128 s1 = _mm_set_ss(a);
    __m128 s2 = _mm_set_ss(b);
    
    results[idx++] = _mm_cmp_ss(s1, s2, _CMP_UNORD_Q);
    results[idx++] = _mm_cmp_ss(s2, s1, _CMP_ORD_Q);
    results[idx++] = _mm_cmp_ss(s1, s2, _CMP_UNEQ_UQ);
    
    // Use results in control flow
    int mask = 0;
    for (int i = 0; i < idx; i++) {
        mask |= _mm_movemask_ps(results[i]);
    }
    
    // Blend based on comparison results
    __m128 blended = _mm_blendv_ps(v1, v2, results[0]);
    blended = _mm_blendv_ps(blended, v_nan, results[1]);
    
    // Extract final result
    float res[4];
    _mm_store_ps(res, blended);
    
    return res[0] + res[1] + res[2] + res[3] + (float)mask;
}

// Function to test AVX comparisons when available
#ifdef __AVX__
FORCE_INLINE double test_avx_comparisons(double a, double b, double c, double d) {
    __m256d v1 = _mm256_set_pd(a, b, c, d);
    __m256d v2 = _mm256_set_pd(d, c, b, a);
    __m256d v_nan = _mm256_set1_pd(NAN);
    __m256d v_inf = _mm256_set1_pd(INFINITY);
    
    // Test double precision comparisons
    __m256d results[8];
    
    // UNORDERED
    results[0] = _mm256_cmp_pd(v_nan, v1, _CMP_UNORD_Q);
    
    // ORDERED
    results[1] = _mm256_cmp_pd(v1, v2, _CMP_ORD_Q);
    
    // UNEQ
    results[2] = _mm256_cmp_pd(v1, v2, _CMP_UNEQ_UQ);
    
    // UNGE (nlt)
    results[3] = _mm256_cmp_pd(v1, v_inf, _CMP_NGE_UQ);
    
    // UNGT (nle)
    results[4] = _mm256_cmp_pd(v_inf, v1, _CMP_NGT_UQ);
    
    // UNLE
    results[5] = _mm256_cmp_pd(v1, v2, _CMP_ULE_UQ);
    
    // UNLT
    results[6] = _mm256_cmp_pd(v2, v1, _CMP_ULT_UQ);
    
    // LTGT (une)
    results[7] = _mm256_cmp_pd(v1, v2, _CMP_NEQ_UQ);
    
    // Use results in arithmetic
    __m256d sum = _mm256_setzero_pd();
    for (int i = 0; i < 8; i++) {
        // Convert comparison mask to double (0.0 or 1.0)
        __m256d mask_d = _mm256_and_pd(results[i], _mm256_set1_pd(1.0));
        sum = _mm256_add_pd(sum, mask_d);
    }
    
    // Extract result
    double res[4];
    _mm256_store_pd(res, sum);
    
    return res[0] + res[1] + res[2] + res[3];
}
#endif

// Force assembly output with inline assembly
FORCE_INLINE void force_asm_output(__m128 vec) {
    // Use inline assembly to force the compiler to generate
    // assembly instructions with condition codes
    __asm__ __volatile__ (
        "# Vector operand: %0\n\t"
        : 
        : "x" (vec)
        : 
    );
}

int main() {
    // Initialize with various values including special cases
    float f1 = 1.0f, f2 = 2.0f, f3 = 0.0f, f4 = -1.0f;
    double d1 = 3.14, d2 = 2.71, d3 = 0.0, d4 = -3.14;
    
    printf("Testing SSE comparisons with all condition codes...\n");
    
    // Test SSE comparisons multiple times with different values
    float sse_result = 0.0f;
    for (int i = 0; i < 10; i++) {
        float t = (float)i * 0.1f;
        sse_result += test_sse_comparisons(f1 + t, f2 - t, f3 + t, f4 - t);
    }
    
    printf("SSE result: %f\n", sse_result);
    
#ifdef __AVX__
    printf("Testing AVX comparisons with all condition codes...\n");
    
    // Test AVX comparisons
    double avx_result = 0.0;
    for (int i = 0; i < 10; i++) {
        double t = (double)i * 0.1;
        avx_result += test_avx_comparisons(d1 + t, d2 - t, d3 + t, d4 - t);
    }
    
    printf("AVX result: %f\n", avx_result);
#else
    printf("AVX not available on this platform\n");
#endif
    
    // Create some vectors and force assembly output
    __m128 test_vec = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 cmp_result = _mm_cmp_ps(test_vec, _mm_set1_ps(2.5f), _CMP_UNORD_Q);
    force_asm_output(cmp_result);
    
    // Test all condition codes in a loop to ensure they're all used
    const int cond_codes[] = {
        _CMP_UNORD_Q,  // UNORDERED
        _CMP_ORD_Q,    // ORDERED
        _CMP_UNEQ_UQ,  // UNEQ
        _CMP_NGE_UQ,   // UNGE (nlt)
        _CMP_NGT_UQ,   // UNGT (nle)
        _CMP_ULE_UQ,   // UNLE
        _CMP_ULT_UQ,   // UNLT
        _CMP_NEQ_UQ    // LTGT (une)
    };
    
    const char* cond_names[] = {
        "UNORDERED", "ORDERED", "UNEQ", "UNGE", "UNGT", "UNLE", "UNLT", "LTGT"
    };
    
    __m128 vec_a = _mm_set_ps(1.0f, NAN, INFINITY, -INFINITY);
    __m128 vec_b = _mm_set_ps(NAN, 1.0f, -INFINITY, INFINITY);
    
    float final_result = 0.0f;
    for (int i = 0; i < 8; i++) {
        __m128 cmp = _mm_cmp_ps(vec_a, vec_b, cond_codes[i]);
        int mask = _mm_movemask_ps(cmp);
        
        // Use result in conditional
        if (mask != 0) {
            __m128 selected = _mm_blendv_ps(vec_a, vec_b, cmp);
            float temp[4];
            _mm_store_ps(temp, selected);
            final_result += temp[0] + temp[1] + temp[2] + temp[3];
        }
        
        printf("Condition %s produced mask: 0x%X\n", cond_names[i], mask);
    }
    
    printf("Final blended result: %f\n", final_result);
    
    return 0;
}
