#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>
#include <math.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
static int test_v64qi_blend(void) {
    alignas(64) int8_t a[64], b[64], result[64];
    
    // Initialize with alternating patterns
    for (int i = 0; i < 64; i++) {
        a[i] = (int8_t)(i % 3);
        b[i] = (int8_t)((i + 1) % 5);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmpgt_epi8_mask(va, _mm512_set1_epi8(1));
    
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

/* ==================== V32HImode (32-halfword integers) ==================== */
static int test_v32hi_blend(void) {
    alignas(64) int16_t a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (int16_t)(i * 2);
        b[i] = (int16_t)(i * 3);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask: select a[i] where a[i] > 30, otherwise b[i]
    __mmask32 mask = _mm512_cmpgt_epi16_mask(va, _mm512_set1_epi16(30));
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
static int test_v32hf_blend(void) {
    alignas(64) _Float16 a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)(i * 0.75f);
    }
    
    __m512h va = _mm512_load_ph((const __m512h*)a);
    __m512h vb = _mm512_load_ph((const __m512h*)b);
    
    // Compare: a > 8.0
    __mmask32 mask = _mm512_cmp_ph_mask(va, _mm512_set1_ph(8.0f), _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph((__m512h*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)result[i];
    }
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
static int test_v32bf_blend(void) {
    alignas(64) __bf16 a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float((float)(i * 0.5f));
        b[i] = bfloat16_from_float((float)(i * 0.75f));
    }
    
    __m512bh va = _mm512_load_si512((const __m512i*)a);
    __m512bh vb = _mm512_load_si512((const __m512i*)b);
    
    // For bfloat16, we need to use integer blend since there's no direct bfloat16 blend
    // This will still trigger the blend logic for V32BFmode
    __mmask32 mask = 0xAAAAAAAA;  // Alternating pattern
    
    // Use integer blend as workaround for bfloat16
    __m512i vai = _mm512_castps_si512(_mm512_castbh_ps(va));
    __m512i vbi = _mm512_castps_si512(_mm512_castbh_ps(vb));
    __m512i vresulti = _mm512_mask_blend_epi16(mask, vai, vbi);
    
    _mm512_store_si512((__m512i*)result, vresulti);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)bfloat16_to_float(result[i]);
    }
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
static int test_v16si_blend(void) {
    alignas(64) int32_t a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 10;
        b[i] = i * 15;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask using comparison
    __mmask16 mask = _mm512_cmpgt_epi32_mask(va, _mm512_set1_epi32(80));
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
static long long test_v8di_blend(void) {
    alignas(64) int64_t a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 100LL;
        b[i] = i * 150LL;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask: select a where a > 300
    __mmask8 mask = _mm512_cmpgt_epi64_mask(va, _mm512_set1_epi64(300));
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V8DFmode (8-double precision floats) ==================== */
static double test_v8df_blend(void) {
    alignas(64) double a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.5;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    // Generate mask: select a where a > 6.0
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

/* ==================== V16SFmode (16-single precision floats) ==================== */
static float test_v16sf_blend(void) {
    alignas(64) float a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 0.75f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    // Generate mask: select a where a > 4.0
    __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(4.0f), _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_store_ps(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
static int test_mixed_blends(int iterations) {
    alignas(64) float farr[16];
    alignas(64) double darr[8];
    alignas(64) int32_t iarr[16];
    alignas(64) int16_t sarr[32];
    
    // Initialize arrays
    for (int i = 0; i < 16; i++) {
        farr[i] = (float)(i + iterations);
        iarr[i] = i + iterations;
    }
    for (int i = 0; i < 8; i++) {
        darr[i] = (double)(i + iterations);
    }
    for (int i = 0; i < 32; i++) {
        sarr[i] = (int16_t)(i + iterations);
    }
    
    int total = 0;
    
    // Process in a loop to ensure blend instructions are generated
    for (int iter = 0; iter < iterations; iter++) {
        // Float blend
        __m512 vf1 = _mm512_load_ps(farr);
        __m512 vf2 = _mm512_add_ps(vf1, _mm512_set1_ps(1.0f));
        __mmask16 fmask = _mm512_cmp_ps_mask(vf1, _mm512_set1_ps(8.0f), _CMP_GT_OQ);
        __m512 vfres = _mm512_mask_blend_ps(fmask, vf1, vf2);
        _mm512_store_ps(farr, vfres);
        
        // Double blend
        __m512d vd1 = _mm512_load_pd(darr);
        __m512d vd2 = _mm512_add_pd(vd1, _mm512_set1_pd(1.0));
        __mmask8 dmask = _mm512_cmp_pd_mask(vd1, _mm512_set1_pd(4.0), _CMP_GT_OQ);
        __m512d vdres = _mm512_mask_blend_pd(dmask, vd1, vd2);
        _mm512_store_pd(darr, vdres);
        
        // Integer blend
        __m512i vi1 = _mm512_load_si512((const __m512i*)iarr);
        __m512i vi2 = _mm512_add_epi32(vi1, _mm512_set1_epi32(1));
        __mmask16 imask = _mm512_cmpgt_epi32_mask(vi1, _mm512_set1_epi32(8));
        __m512i vires = _mm512_mask_blend_epi32(imask, vi1, vi2);
        _mm512_store_si512((__m512i*)iarr, vires);
        
        // Short blend
        __m512i vs1 = _mm512_load_si512((const __m512i*)sarr);
        __m512i vs2 = _mm512_add_epi16(vs1, _mm512_set1_epi16(1));
        __mmask32 smask = _mm512_cmpgt_epi16_mask(vs1, _mm512_set1_epi16(16));
        __m512i vsres = _mm512_mask_blend_epi16(smask, vs1, vs2);
        _mm512_store_si512((__m512i*)sarr, vsres);
        
        // Update arrays for next iteration
        for (int i = 0; i < 16; i++) {
            farr[i] += 0.1f;
            iarr[i] += 1;
        }
        for (int i = 0; i < 8; i++) {
            darr[i] += 0.1;
        }
        for (int i = 0; i < 32; i++) {
            sarr[i] += 1;
        }
    }
    
    // Compute final checksum
    for (int i = 0; i < 16; i++) {
        total += (int)farr[i] + iarr[i];
    }
    for (int i = 0; i < 8; i++) {
        total += (int)darr[i];
    }
    for (int i = 0; i < 32; i++) {
        total += sarr[i];
    }
    
    return total;
}

#endif  // __AVX512BW__
#endif  // __AVX512F__

/* ==================== Scalar Fallbacks ==================== */
static int scalar_test_v64qi_blend(void) {
    int8_t a[64], b[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (int8_t)(i % 3);
        b[i] = (int8_t)((i + 1) % 5);
        result[i] = (a[i] > 1) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

static int scalar_test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 0.75f;
        result[i] = (a[i] > 4.0f) ? a[i] : b[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return (int)sum;
}

/* ==================== Main Function ==================== */
int main(void) {
    int total_result = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    // Test all vector modes
    total_result += test_v64qi_blend();
    total_result += test_v32hi_blend();
    
#ifdef __AVX512FP16__
    total_result += test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total_result += test_v32bf_blend();
#endif
    
    total_result += test_v16si_blend();
    total_result += (int)test_v8di_blend();
    total_result += (int)test_v8df_blend();
    total_result += (int)test_v16sf_blend();
    
    // Test mixed types in loop
    total_result += test_mixed_blends(10);
    
#else
    printf("AVX-512BW not available. Using scalar fallbacks...\n");
    total_result += scalar_test_v64qi_blend();
    total_result += scalar_test_v16sf_blend();
#endif
#else
    printf("AVX-512 not available. Using scalar fallbacks...\n");
    total_result += scalar_test_v64qi_blend();
    total_result += scalar_test_v16sf_blend();
#endif
    
    printf("Final checksum: %d\n", total_result);
    return total_result;
}
