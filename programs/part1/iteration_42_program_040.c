#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
int test_v64qi_blend() {
    alignas(64) int8_t a[64], b[64], result[64];
    
    // Initialize with alternating patterns
    for (int i = 0; i < 64; i++) {
        a[i] = (int8_t)(i % 3);
        b[i] = (int8_t)((i + 1) % 5);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    // Blend based on mask - this should trigger gen_avx512bw_blendmv64qi
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
int test_v32hi_blend() {
    alignas(64) int16_t a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (int16_t)(i * 2);
        b[i] = (int16_t)(i * 3);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask using bitwise operation for variety
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_EQ);
    
    // Blend - should trigger gen_avx512bw_blendmv32hi
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
float test_v32hf_blend() {
    alignas(64) _Float16 a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)(i * 0.75f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    // Generate mask using comparison
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    // Blend - should trigger gen_avx512bw_blendmv32hf
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
float test_v32bf_blend() {
    alignas(64) __bf16 a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (__bf16)(i * 0.5f);
        b[i] = (__bf16)(i * 0.75f);
    }
    
    // Load as epi16 for bfloat16 emulation
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_GT);
    
    // Blend using epi16 intrinsic - should trigger gen_avx512bw_blendmv32bf
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
int test_v16si_blend() {
    alignas(64) int32_t a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 10;
        b[i] = i * 15;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Complex mask generation
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_LT);
    
    // Blend - should trigger gen_avx512f_blendmv16si
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
long long test_v8di_blend() {
    alignas(64) int64_t a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 100LL;
        b[i] = i * 150LL;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_NEQ);
    
    // Blend - should trigger gen_avx512f_blendmv8di
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V8DFmode (8-double-precision floats) ==================== */
double test_v8df_blend() {
    alignas(64) double a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.5;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LE_OQ);
    
    // Blend - should trigger gen_avx512f_blendmv8df
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_store_pd(result, vresult);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V16SFmode (16-single-precision floats) ==================== */
float test_v16sf_blend() {
    alignas(64) float a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.25f;
        b[i] = i * 0.375f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GE_OQ);
    
    // Blend - should trigger gen_avx512f_blendmv16sf
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_store_ps(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
double test_mixed_blend_loop(int iterations) {
    alignas(64) float farr[16];
    alignas(64) double darr[8];
    alignas(64) int32_t iarr[16];
    alignas(64) int16_t sarr[32];
    
    // Initialize arrays
    for (int i = 0; i < 16; i++) {
        farr[i] = (float)(i + 1);
        iarr[i] = i * 2;
    }
    for (int i = 0; i < 8; i++) {
        darr[i] = (double)(i + 1) * 0.5;
    }
    for (int i = 0; i < 32; i++) {
        sarr[i] = (int16_t)(i * 3);
    }
    
    double total_sum = 0.0;
    
    for (int iter = 0; iter < iterations; iter++) {
        // Process float array
        __m512 vf1 = _mm512_load_ps(farr);
        __m512 vf2 = _mm512_set1_ps(iter * 0.1f);
        __mmask16 fmask = _mm512_cmp_ps_mask(vf1, vf2, _CMP_GT_OQ);
        __m512 vfres = _mm512_mask_blend_ps(fmask, vf1, vf2);
        _mm512_store_ps(farr, vfres);
        
        // Process double array
        __m512d vd1 = _mm512_load_pd(darr);
        __m512d vd2 = _mm512_set1_pd(iter * 0.05);
        __mmask8 dmask = _mm512_cmp_pd_mask(vd1, vd2, _CMP_LT_OQ);
        __m512d vdres = _mm512_mask_blend_pd(dmask, vd1, vd2);
        _mm512_store_pd(darr, vdres);
        
        // Process int32 array
        __m512i vi1 = _mm512_load_si512((__m512i*)iarr);
        __m512i vi2 = _mm512_set1_epi32(iter);
        __mmask16 imask = _mm512_cmp_epi32_mask(vi1, vi2, _MM_CMPINT_EQ);
        __m512i vires = _mm512_mask_blend_epi32(imask, vi1, vi2);
        _mm512_store_si512((__m512i*)iarr, vires);
        
        // Process int16 array
        __m512i vs1 = _mm512_load_si512((__m512i*)sarr);
        __m512i vs2 = _mm512_set1_epi16(iter);
        __mmask32 smask = _mm512_cmp_epi16_mask(vs1, vs2, _MM_CMPINT_GT);
        __m512i vsres = _mm512_mask_blend_epi16(smask, vs1, vs2);
        _mm512_store_si512((__m512i*)sarr, vsres);
        
        // Accumulate results
        for (int i = 0; i < 16; i++) {
            total_sum += farr[i] + iarr[i];
        }
        for (int i = 0; i < 8; i++) {
            total_sum += darr[i];
        }
        for (int i = 0; i < 32; i++) {
            total_sum += sarr[i];
        }
    }
    
    return total_sum;
}

#endif // __AVX512BW__
#endif // __AVX512F__

/* ==================== Scalar Fallbacks ==================== */
int scalar_test_v64qi_blend() {
    int8_t a[64], b[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (int8_t)(i % 3);
        b[i] = (int8_t)((i + 1) % 5);
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

int scalar_test_v32hi_blend() {
    int16_t a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (int16_t)(i * 2);
        b[i] = (int16_t)(i * 3);
        result[i] = (a[i] == b[i]) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

float scalar_test_v16sf_blend() {
    float a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.25f;
        b[i] = i * 0.375f;
        result[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

double scalar_test_v8df_blend() {
    double a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.5;
        result[i] = (a[i] <= b[i]) ? a[i] : b[i];
    }
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== Main Function ==================== */
int main() {
    long long total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    // Test each vector mode individually
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
    
    // Test mixed types in loop
    double mixed_result = test_mixed_blend_loop(10);
    total_checksum += (long long)mixed_result;
    
#else
    printf("AVX-512BW not available. Running scalar tests...\n");
#endif
#else
    printf("AVX-512 not available. Running scalar tests...\n");
#endif

#ifndef __AVX512F__
    // Run scalar fallbacks
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
    total_checksum += (long long)scalar_test_v16sf_blend();
    total_checksum += (long long)scalar_test_v8df_blend();
#endif

    printf("Total checksum: %lld\n", total_checksum);
    return (int)(total_checksum % 256);
}
