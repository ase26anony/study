#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
int test_v64qi_blend() {
    __m512i a, b, result;
    __mmask64 mask;
    uint8_t a_arr[64], b_arr[64], res_arr[64];
    int sum = 0;
    
    // Initialize with pattern
    for (int i = 0; i < 64; i++) {
        a_arr[i] = i;
        b_arr[i] = 64 - i;
    }
    
    a = _mm512_loadu_si512((__m512i*)a_arr);
    b = _mm512_loadu_si512((__m512i*)b_arr);
    
    // Generate dynamic mask: select a where a > 32, otherwise b
    mask = _mm512_cmpgt_epi8_mask(a, _mm512_set1_epi8(32));
    
    // This should trigger gen_avx512bw_blendmv64qi
    result = _mm512_mask_blend_epi8(mask, b, a);
    
    _mm512_storeu_si512((__m512i*)res_arr, result);
    
    // Compute checksum to prevent optimization
    for (int i = 0; i < 64; i++) {
        sum += res_arr[i];
    }
    
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
int test_v32hi_blend() {
    __m512i a, b, result;
    __mmask32 mask;
    short a_arr[32], b_arr[32], res_arr[32];
    int sum = 0;
    
    for (int i = 0; i < 32; i++) {
        a_arr[i] = i * 100;
        b_arr[i] = 3200 - i * 100;
    }
    
    a = _mm512_loadu_si512((__m512i*)a_arr);
    b = _mm512_loadu_si512((__m512i*)b_arr);
    
    mask = _mm512_cmpgt_epi16_mask(a, _mm512_set1_epi16(1600));
    
    // This should trigger gen_avx512bw_blendmv32hi
    result = _mm512_mask_blend_epi16(mask, b, a);
    
    _mm512_storeu_si512((__m512i*)res_arr, result);
    
    for (int i = 0; i < 32; i++) {
        sum += res_arr[i];
    }
    
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
float test_v32hf_blend() {
    __m512h a, b, result;
    __mmask32 mask;
    _Float16 a_arr[32], b_arr[32], res_arr[32];
    float sum = 0.0f;
    
    for (int i = 0; i < 32; i++) {
        a_arr[i] = (_Float16)(i * 0.5f);
        b_arr[i] = (_Float16)(16.0f - i * 0.5f);
    }
    
    a = _mm512_loadu_ph(a_arr);
    b = _mm512_loadu_ph(b_arr);
    
    mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(8.0f), _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    result = _mm512_mask_blend_ph(mask, b, a);
    
    _mm512_storeu_ph(res_arr, result);
    
    for (int i = 0; i < 32; i++) {
        sum += (float)res_arr[i];
    }
    
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
float test_v32bf_blend() {
    __m512bh a, b, result;
    __mmask32 mask;
    __bfloat16 a_arr[32], b_arr[32], res_arr[32];
    float sum = 0.0f;
    
    for (int i = 0; i < 32; i++) {
        a_arr[i] = bfloat16_from_float(i * 0.5f);
        b_arr[i] = bfloat16_from_float(16.0f - i * 0.5f);
    }
    
    a = _mm512_loadu_si512((__m512i*)a_arr);
    b = _mm512_loadu_si512((__m512i*)b_arr);
    
    // For bfloat16, we need to use integer blend since there's no direct bfloat16 blend
    // This should still trigger the blend logic
    mask = _mm512_cmp_epi16_mask((__m512i)a, _mm512_set1_epi16(bfloat16_from_float(8.0f)), _MM_CMPINT_GT);
    result = (__m512bh)_mm512_mask_blend_epi16(mask, (__m512i)b, (__m512i)a);
    
    _mm512_storeu_si512((__m512i*)res_arr, (__m512i)result);
    
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(res_arr[i]);
    }
    
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
int test_v16si_blend() {
    __m512i a, b, result;
    __mmask16 mask;
    int a_arr[16], b_arr[16], res_arr[16];
    int sum = 0;
    
    for (int i = 0; i < 16; i++) {
        a_arr[i] = i * 1000;
        b_arr[i] = 16000 - i * 1000;
    }
    
    a = _mm512_loadu_si512((__m512i*)a_arr);
    b = _mm512_loadu_si512((__m512i*)b_arr);
    
    mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(8000));
    
    // This should trigger gen_avx512f_blendmv16si
    result = _mm512_mask_blend_epi32(mask, b, a);
    
    _mm512_storeu_si512((__m512i*)res_arr, result);
    
    for (int i = 0; i < 16; i++) {
        sum += res_arr[i];
    }
    
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
long long test_v8di_blend() {
    __m512i a, b, result;
    __mmask8 mask;
    long long a_arr[8], b_arr[8], res_arr[8];
    long long sum = 0;
    
    for (int i = 0; i < 8; i++) {
        a_arr[i] = i * 10000LL;
        b_arr[i] = 80000LL - i * 10000LL;
    }
    
    a = _mm512_loadu_si512((__m512i*)a_arr);
    b = _mm512_loadu_si512((__m512i*)b_arr);
    
    mask = _mm512_cmpgt_epi64_mask(a, _mm512_set1_epi64(40000LL));
    
    // This should trigger gen_avx512f_blendmv8di
    result = _mm512_mask_blend_epi64(mask, b, a);
    
    _mm512_storeu_si512((__m512i*)res_arr, result);
    
    for (int i = 0; i < 8; i++) {
        sum += res_arr[i];
    }
    
    return sum;
}

/* ==================== V8DFmode (8-double-precision floats) ==================== */
double test_v8df_blend() {
    __m512d a, b, result;
    __mmask8 mask;
    double a_arr[8], b_arr[8], res_arr[8];
    double sum = 0.0;
    
    for (int i = 0; i < 8; i++) {
        a_arr[i] = i * 1.5;
        b_arr[i] = 12.0 - i * 1.5;
    }
    
    a = _mm512_loadu_pd(a_arr);
    b = _mm512_loadu_pd(b_arr);
    
    mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(6.0), _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    result = _mm512_mask_blend_pd(mask, b, a);
    
    _mm512_storeu_pd(res_arr, result);
    
    for (int i = 0; i < 8; i++) {
        sum += res_arr[i];
    }
    
    return sum;
}

/* ==================== V16SFmode (16-single-precision floats) ==================== */
float test_v16sf_blend() {
    __m512 a, b, result;
    __mmask16 mask;
    float a_arr[16], b_arr[16], res_arr[16];
    float sum = 0.0f;
    
    for (int i = 0; i < 16; i++) {
        a_arr[i] = i * 0.25f;
        b_arr[i] = 4.0f - i * 0.25f;
    }
    
    a = _mm512_loadu_ps(a_arr);
    b = _mm512_loadu_ps(b_arr);
    
    mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(2.0f), _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    result = _mm512_mask_blend_ps(mask, b, a);
    
    _mm512_storeu_ps(res_arr, result);
    
    for (int i = 0; i < 16; i++) {
        sum += res_arr[i];
    }
    
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
double test_mixed_blends(int iterations) {
    double total_sum = 0.0;
    
    for (int iter = 0; iter < iterations; iter++) {
        // V16SF
        {
            float a_arr[16], b_arr[16];
            for (int i = 0; i < 16; i++) {
                a_arr[i] = (iter + i) * 0.1f;
                b_arr[i] = (iter - i) * 0.1f;
            }
            __m512 a = _mm512_loadu_ps(a_arr);
            __m512 b = _mm512_loadu_ps(b_arr);
            __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
            __m512 result = _mm512_mask_blend_ps(mask, b, a);
            float res_arr[16];
            _mm512_storeu_ps(res_arr, result);
            for (int i = 0; i < 16; i++) total_sum += res_arr[i];
        }
        
        // V8DF
        {
            double a_arr[8], b_arr[8];
            for (int i = 0; i < 8; i++) {
                a_arr[i] = (iter + i) * 0.2;
                b_arr[i] = (iter - i) * 0.2;
            }
            __m512d a = _mm512_loadu_pd(a_arr);
            __m512d b = _mm512_loadu_pd(b_arr);
            __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
            __m512d result = _mm512_mask_blend_pd(mask, b, a);
            double res_arr[8];
            _mm512_storeu_pd(res_arr, result);
            for (int i = 0; i < 8; i++) total_sum += res_arr[i];
        }
        
        // V16SI
        {
            int a_arr[16], b_arr[16];
            for (int i = 0; i < 16; i++) {
                a_arr[i] = iter * 100 + i;
                b_arr[i] = iter * 100 - i;
            }
            __m512i a = _mm512_loadu_si512((__m512i*)a_arr);
            __m512i b = _mm512_loadu_si512((__m512i*)b_arr);
            __mmask16 mask = _mm512_cmpgt_epi32_mask(a, b);
            __m512i result = _mm512_mask_blend_epi32(mask, b, a);
            int res_arr[16];
            _mm512_storeu_si512((__m512i*)res_arr, result);
            for (int i = 0; i < 16; i++) total_sum += res_arr[i];
        }
    }
    
    return total_sum;
}

#endif // __AVX512BW__
#endif // __AVX512F__

/* ==================== Scalar Fallbacks ==================== */
int scalar_test_v64qi_blend() {
    uint8_t a_arr[64], b_arr[64], res_arr[64];
    int sum = 0;
    
    for (int i = 0; i < 64; i++) {
        a_arr[i] = i;
        b_arr[i] = 64 - i;
        res_arr[i] = (a_arr[i] > 32) ? a_arr[i] : b_arr[i];
        sum += res_arr[i];
    }
    
    return sum;
}

int scalar_test_v32hi_blend() {
    short a_arr[32], b_arr[32], res_arr[32];
    int sum = 0;
    
    for (int i = 0; i < 32; i++) {
        a_arr[i] = i * 100;
        b_arr[i] = 3200 - i * 100;
        res_arr[i] = (a_arr[i] > 1600) ? a_arr[i] : b_arr[i];
        sum += res_arr[i];
    }
    
    return sum;
}

float scalar_test_v16sf_blend() {
    float a_arr[16], b_arr[16], res_arr[16];
    float sum = 0.0f;
    
    for (int i = 0; i < 16; i++) {
        a_arr[i] = i * 0.25f;
        b_arr[i] = 4.0f - i * 0.25f;
        res_arr[i] = (a_arr[i] > 2.0f) ? a_arr[i] : b_arr[i];
        sum += res_arr[i];
    }
    
    return sum;
}

double scalar_test_v8df_blend() {
    double a_arr[8], b_arr[8], res_arr[8];
    double sum = 0.0;
    
    for (int i = 0; i < 8; i++) {
        a_arr[i] = i * 1.5;
        b_arr[i] = 12.0 - i * 1.5;
        res_arr[i] = (a_arr[i] > 6.0) ? a_arr[i] : b_arr[i];
        sum += res_arr[i];
    }
    
    return sum;
}

int scalar_test_v16si_blend() {
    int a_arr[16], b_arr[16], res_arr[16];
    int sum = 0;
    
    for (int i = 0; i < 16; i++) {
        a_arr[i] = i * 1000;
        b_arr[i] = 16000 - i * 1000;
        res_arr[i] = (a_arr[i] > 8000) ? a_arr[i] : b_arr[i];
        sum += res_arr[i];
    }
    
    return sum;
}

long long scalar_test_v8di_blend() {
    long long a_arr[8], b_arr[8], res_arr[8];
    long long sum = 0;
    
    for (int i = 0; i < 8; i++) {
        a_arr[i] = i * 10000LL;
        b_arr[i] = 80000LL - i * 10000LL;
        res_arr[i] = (a_arr[i] > 40000LL) ? a_arr[i] : b_arr[i];
        sum += res_arr[i];
    }
    
    return sum;
}

/* ==================== Main Driver ==================== */
int main() {
    long long total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running optimized tests...\n");
    
    // Test all vector modes
    total_checksum += test_v64qi_blend();
    total_checksum += test_v32hi_blend();
    
#ifdef __AVX512FP16__
    total_checksum += (long long)test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total_checksum += (long long)test_v32bf_blend();
#endif
    
    total_checksum += test_v16si_blend();
    total_checksum += test_v8di_blend();
    total_checksum += (long long)test_v8df_blend();
    total_checksum += (long long)test_v16sf_blend();
    
    // Mixed test with loop
    total_checksum += (long long)test_mixed_blends(10);
    
#else
    printf("AVX-512BW not available. Using scalar fallbacks...\n");
#endif
#else
    printf("AVX-512 not available. Using scalar fallbacks...\n");
#endif

#ifndef __AVX512F__
    // Scalar fallbacks
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
    total_checksum += (long long)scalar_test_v16sf_blend();
    total_checksum += (long long)scalar_test_v8df_blend();
    total_checksum += scalar_test_v16si_blend();
    total_checksum += scalar_test_v8di_blend();
#endif

    printf("Total checksum: %lld\n", total_checksum);
    
    // Return non-zero to indicate success (prevents optimization of entire program)
    return total_checksum != 0 ? 0 : 1;
}
