#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64-byte integers */
static int test_v64qi_blend(void) {
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    char result[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 2);
        b[i] = (char)(i * 3);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmpgt_epi8_mask(va, _mm512_set1_epi8(32));
    
    // This should trigger gen_avx512bw_blendmv64qi
    __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    // Compute checksum to prevent optimization
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

/* V32HImode: 32-halfword integers */
static int test_v32hi_blend(void) {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    short result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 10);
        b[i] = (short)(i * 15);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate dynamic mask
    __mmask32 mask = _mm512_cmpgt_epi16_mask(va, _mm512_set1_epi16(160));
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

#ifdef __AVX512FP16__
/* V32HFmode: 32-half-precision floats */
static float test_v32hf_blend(void) {
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    _Float16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)(i * 0.75f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    // Generate mask
    __mmask32 mask = _mm512_cmp_ph_mask(va, _mm512_set1_ph(8.0f), _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    return sum;
}
#endif

#ifdef __AVX512BF16__
/* V32BFmode: 32-bfloat16 floats */
static float test_v32bf_blend(void) {
    __bf16 a[32] __attribute__((aligned(64)));
    __bf16 b[32] __attribute__((aligned(64)));
    __bf16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (__bf16)(i * 0.5f);
        b[i] = (__bf16)(i * 0.75f);
    }
    
    __m512bh va = _mm512_load_si512((__m512i*)a);
    __m512bh vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask using integer comparison since bfloat16 doesn't have direct comparison
    __m512i vai = _mm512_load_si512((__m512i*)a);
    __m512i vbi = _mm512_load_si512((__m512i*)b);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(vai, _mm512_set1_epi16(0x4000)); // 2.0 in bf16
    
    // This should trigger gen_avx512bw_blendmv32bf
    __m512bh vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, (__m512i)vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    return sum;
}
#endif

/* V16SImode: 16-dword integers */
static long long test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    int result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 100;
        b[i] = i * 150;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate dynamic mask
    __mmask16 mask = _mm512_cmpgt_epi32_mask(va, _mm512_set1_epi32(800));
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* V8DImode: 8-qword integers */
static long long test_v8di_blend(void) {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    long long result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1000LL;
        b[i] = i * 1500LL;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate dynamic mask
    __mmask8 mask = _mm512_cmpgt_epi64_mask(va, _mm512_set1_epi64(4000));
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* V8DFmode: 8-double-precision floats */
static double test_v8df_blend(void) {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.25;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    // Generate dynamic mask
    __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(6.0), _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_store_pd(result, vresult);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* V16SFmode: 16-single-precision floats */
static float test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    float result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.75f;
        b[i] = i * 1.125f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    // Generate dynamic mask
    __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(6.0f), _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_store_ps(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* Mixed data types in loop structure */
static long long test_mixed_blend_loop(int iterations) {
    long long total_sum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        // Process different data types in each iteration
        if (iter % 7 == 0) {
            total_sum += test_v64qi_blend();
        } else if (iter % 7 == 1) {
            total_sum += test_v32hi_blend();
        } else if (iter % 7 == 2) {
            total_sum += test_v16si_blend();
        } else if (iter % 7 == 3) {
            total_sum += test_v8di_blend();
        } else if (iter % 7 == 4) {
            total_sum += (long long)test_v16sf_blend();
        } else if (iter % 7 == 5) {
            total_sum += (long long)test_v8df_blend();
#ifdef __AVX512FP16__
        } else if (iter % 7 == 6) {
            total_sum += (long long)test_v32hf_blend();
#endif
        }
    }
    
    return total_sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* Scalar fallback implementations */
static int scalar_test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 2);
        b[i] = (char)(i * 3);
        result[i] = (a[i] > 32) ? b[i] : a[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

static int scalar_test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 10);
        b[i] = (short)(i * 15);
        result[i] = (a[i] > 160) ? b[i] : a[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

static long long scalar_test_v16si_blend(void) {
    int a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 100;
        b[i] = i * 150;
        result[i] = (a[i] > 800) ? b[i] : a[i];
    }
    
    long long sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

static long long scalar_test_v8di_blend(void) {
    long long a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1000LL;
        b[i] = i * 1500LL;
        result[i] = (a[i] > 4000) ? b[i] : a[i];
    }
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

static float scalar_test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.75f;
        b[i] = i * 1.125f;
        result[i] = (a[i] > 6.0f) ? b[i] : a[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

static double scalar_test_v8df_blend(void) {
    double a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.25;
        result[i] = (a[i] > 6.0) ? b[i] : a[i];
    }
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

static long long scalar_test_mixed_blend_loop(int iterations) {
    long long total_sum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        if (iter % 6 == 0) {
            total_sum += scalar_test_v64qi_blend();
        } else if (iter % 6 == 1) {
            total_sum += scalar_test_v32hi_blend();
        } else if (iter % 6 == 2) {
            total_sum += scalar_test_v16si_blend();
        } else if (iter % 6 == 3) {
            total_sum += scalar_test_v8di_blend();
        } else if (iter % 6 == 4) {
            total_sum += (long long)scalar_test_v16sf_blend();
        } else if (iter % 6 == 5) {
            total_sum += (long long)scalar_test_v8df_blend();
        }
    }
    
    return total_sum;
}

int main(void) {
    printf("Testing AVX-512 blend operations...\n");
    
    long long total_result = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("Using AVX-512 optimized implementation\n");
    
    // Test individual blend operations
    total_result += test_v64qi_blend();
    total_result += test_v32hi_blend();
    total_result += test_v16si_blend();
    total_result += test_v8di_blend();
    total_result += (long long)test_v16sf_blend();
    total_result += (long long)test_v8df_blend();
    
#ifdef __AVX512FP16__
    total_result += (long long)test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total_result += (long long)test_v32bf_blend();
#endif
    
    // Test mixed operations in loop
    total_result += test_mixed_blend_loop(10);
#else
    printf("AVX-512BW not available, using scalar fallback\n");
    total_result = scalar_test_mixed_blend_loop(10);
#endif
#else
    printf("AVX-512 not available, using scalar fallback\n");
    total_result = scalar_test_mixed_blend_loop(10);
#endif
    
    printf("Final checksum: %lld\n", total_result);
    
    // Return non-zero to indicate success (simplified)
    return total_result != 0 ? 0 : 1;
}
