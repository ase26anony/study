#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64-byte integers */
static uint64_t test_v64qi_blend(void) {
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv64qi
    __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
    
    // Use result in computation to prevent elimination
    __m512i sum_vec = _mm512_sad_epu8(result, _mm512_setzero_si512());
    uint64_t sum = _mm512_reduce_add_epi64(sum_vec);
    
    return sum;
}

/* V32HImode: 32-halfword integers */
static uint64_t test_v32hi_blend(void) {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate dynamic mask
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    // Compute sum of absolute differences
    __m512i abs_diff = _mm512_abs_epi16(_mm512_sub_epi16(result, va));
    __m512i sum_vec = _mm512_sad_epu16(abs_diff, _mm512_setzero_si512());
    uint64_t sum = _mm512_reduce_add_epi64(sum_vec);
    
    return sum;
}

#ifdef __AVX512FP16__
/* V32HFmode: 32-half-precision floats */
static float test_v32hf_blend(void) {
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h va = _mm512_load_ph((const __m512h*)a);
    __m512h vb = _mm512_load_ph((const __m512h*)b);
    
    // Generate mask using comparison
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    // Compute sum
    __m512h sum_vec = _mm512_add_ph(result, va);
    float sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    
    return sum;
}
#endif

/* V32BFmode: 32-bfloat16 (emulated with epi16) */
static float test_v32bf_blend(void) {
    unsigned short a[32] __attribute__((aligned(64)));
    unsigned short b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (unsigned short)(i * 0x4000);  // Simple bfloat16 pattern
        b[i] = (unsigned short)(i * 0x4040);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_NE);
    
    // This should trigger gen_avx512bw_blendmv32bf
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    // Compute simple checksum
    uint64_t sum = 0;
    unsigned short* r = (unsigned short*)&result;
    for (int i = 0; i < 32; i++) {
        sum += r[i];
    }
    
    return (float)sum;
}

/* V16SImode: 16-dword integers */
static uint64_t test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 13;
        b[i] = i * 17;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate dynamic mask
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    // Compute horizontal sum
    __m512i sum_vec = _mm512_add_epi32(result, _mm512_set1_epi32(1));
    uint64_t sum = _mm512_reduce_add_epi32(sum_vec);
    
    return sum;
}

/* V8DImode: 8-qword integers */
static uint64_t test_v8di_blend(void) {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 23LL;
        b[i] = i * 29LL;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate dynamic mask
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_LE);
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    // Compute sum
    __m512i sum_vec = _mm512_add_epi64(result, _mm512_set1_epi64(1));
    uint64_t sum = _mm512_reduce_add_epi64(sum_vec);
    
    return sum;
}

/* V8DFmode: 8-double-precision floats */
static double test_v8df_blend(void) {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.1;
        b[i] = i * 2.2;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    // Generate dynamic mask
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d result = _mm512_mask_blend_pd(mask, va, vb);
    
    // Compute sum
    __m512d sum_vec = _mm512_add_pd(result, _mm512_set1_pd(1.0));
    double sum = _mm512_reduce_add_pd(sum_vec);
    
    return sum;
}

/* V16SFmode: 16-single-precision floats */
static float test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1.5f;
        b[i] = i * 3.5f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    // Generate dynamic mask
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GE_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    // Compute sum
    __m512 sum_vec = _mm512_add_ps(result, _mm512_set1_ps(1.0f));
    float sum = _mm512_reduce_add_ps(sum_vec);
    
    return sum;
}

/* Mixed data types in loop structure */
static uint64_t test_mixed_blends(int iterations) {
    uint64_t total_sum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        // Test different blend types in sequence
        total_sum += test_v64qi_blend();
        total_sum += test_v32hi_blend();
        total_sum += test_v16si_blend();
        total_sum += test_v8di_blend();
        
        // Floating point blends
        total_sum += (uint64_t)test_v16sf_blend();
        total_sum += (uint64_t)test_v8df_blend();
        total_sum += (uint64_t)test_v32bf_blend();
        
#ifdef __AVX512FP16__
        total_sum += (uint64_t)test_v32hf_blend();
#endif
    }
    
    return total_sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* Scalar fallback implementations */
static uint64_t scalar_test_v64qi_blend(void) {
    char a[64], b[64];
    char result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    uint64_t mask = 0;
    for (int i = 0; i < 64; i++) {
        mask |= ((uint64_t)(a[i] > b[i]) << i);
    }
    
    for (int i = 0; i < 64; i++) {
        result[i] = (mask & (1ULL << i)) ? b[i] : a[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (unsigned char)result[i];
    }
    
    return sum;
}

static uint64_t scalar_test_v32hi_blend(void) {
    short a[32], b[32];
    short result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
    }
    
    unsigned mask = 0;
    for (int i = 0; i < 32; i++) {
        mask |= ((unsigned)(a[i] < b[i]) << i);
    }
    
    for (int i = 0; i < 32; i++) {
        result[i] = (mask & (1U << i)) ? b[i] : a[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (unsigned short)result[i];
    }
    
    return sum;
}

/* Main driver function */
int main(void) {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    // Run mixed tests with multiple iterations
    total_checksum = test_mixed_blends(3);
    
    // Also run individual tests to ensure each gets compiled
    volatile uint64_t v64qi_result = test_v64qi_blend();
    volatile uint64_t v32hi_result = test_v32hi_blend();
    volatile uint64_t v16si_result = test_v16si_blend();
    volatile uint64_t v8di_result = test_v8di_blend();
    volatile float v16sf_result = test_v16sf_blend();
    volatile double v8df_result = test_v8df_blend();
    volatile float v32bf_result = test_v32bf_blend();
    
#ifdef __AVX512FP16__
    volatile float v32hf_result = test_v32hf_blend();
#endif
    
    // Add individual results to checksum
    total_checksum += v64qi_result + v32hi_result + v16si_result + v8di_result;
    total_checksum += (uint64_t)v16sf_result + (uint64_t)v8df_result + (uint64_t)v32bf_result;
    
#ifdef __AVX512FP16__
    total_checksum += (uint64_t)v32hf_result;
#endif
    
#else
    printf("AVX-512BW not supported. Running scalar tests...\n");
    total_checksum = scalar_test_v64qi_blend() + scalar_test_v32hi_blend();
#endif
#else
    printf("AVX-512 not supported. Running minimal scalar tests...\n");
    total_checksum = scalar_test_v64qi_blend() + scalar_test_v32hi_blend();
#endif
    
    printf("Final checksum: %lu\n", total_checksum);
    
    // Return non-zero to indicate success with data
    return (total_checksum != 0) ? 0 : 1;
}
