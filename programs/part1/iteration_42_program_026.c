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
        a[i] = i;
        b[i] = 64 - i;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Create dynamic mask: elements where a[i] > 32
    __mmask64 mask = _mm512_cmp_epi8_mask(va, _mm512_set1_epi8(32), _MM_CMPINT_GT);
    
    // Blend based on mask: if mask bit=1, take from va, else from vb
    __m512i vresult = _mm512_mask_blend_epi8(mask, vb, va);
    
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
        a[i] = i * 2;
        b[i] = 1000 - i * 3;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Create mask: elements where a[i] > 20
    __mmask32 mask = _mm512_cmp_epi16_mask(va, _mm512_set1_epi16(20), _MM_CMPINT_GT);
    
    __m512i vresult = _mm512_mask_blend_epi16(mask, vb, va);
    
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
        a[i] = i * 0.5f;
        b[i] = 10.0f - i * 0.3f;
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    // Create mask: elements where a[i] > 8.0
    __mmask32 mask = _mm512_cmp_ph_mask(va, _mm512_set1_ph(8.0f), _CMP_GT_OQ);
    
    __m512h vresult = _mm512_mask_blend_ph(mask, vb, va);
    
    _mm512_store_ph(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
float test_v32bf_blend() {
    alignas(64) __bf16 a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        float fa = i * 0.5f;
        float fb = 10.0f - i * 0.3f;
        a[i] = _mm_cvtness_sbh(fa);
        b[i] = _mm_cvtness_sbh(fb);
    }
    
    // For bfloat16, we use epi16 blend since there's no direct bf16 blend intrinsic
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Create mask based on converted float values
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 3 == 0) mask |= (1ULL << i);
    }
    
    __m512i vresult = _mm512_mask_blend_epi16(mask, vb, va);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += _mm_cvtbh_ps(_mm_cvtsi32_si128(result[i]));
    }
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
int test_v16si_blend() {
    alignas(64) int32_t a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 100;
        b[i] = 5000 - i * 200;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Create mask: elements where a[i] > 800
    __mmask16 mask = _mm512_cmp_epi32_mask(va, _mm512_set1_epi32(800), _MM_CMPINT_GT);
    
    __m512i vresult = _mm512_mask_blend_epi32(mask, vb, va);
    
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
        a[i] = i * 1000LL;
        b[i] = 10000LL - i * 500LL;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Create mask: elements where a[i] > 3000
    __mmask8 mask = _mm512_cmp_epi64_mask(va, _mm512_set1_epi64(3000), _MM_CMPINT_GT);
    
    __m512i vresult = _mm512_mask_blend_epi64(mask, vb, va);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V8DFmode (8-double precision floats) ==================== */
double test_v8df_blend() {
    alignas(64) double a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = 10.0 - i * 0.7;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    // Create mask: elements where a[i] > 6.0
    __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(6.0), _CMP_GT_OQ);
    
    __m512d vresult = _mm512_mask_blend_pd(mask, vb, va);
    
    _mm512_store_pd(result, vresult);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V16SFmode (16-single precision floats) ==================== */
float test_v16sf_blend() {
    alignas(64) float a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.8f;
        b[i] = 12.0f - i * 0.4f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    // Create mask: elements where a[i] > 6.0
    __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(6.0f), _CMP_GT_OQ);
    
    __m512 vresult = _mm512_mask_blend_ps(mask, vb, va);
    
    _mm512_store_ps(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
void test_mixed_blends(int iterations) {
    alignas(64) float farr[16];
    alignas(64) double darr[8];
    alignas(64) int32_t iarr[16];
    alignas(64) int16_t sarr[32];
    
    for (int iter = 0; iter < iterations; iter++) {
        // Initialize with iteration-dependent values
        for (int i = 0; i < 16; i++) {
            farr[i] = (iter + i) * 0.1f;
            iarr[i] = (iter + i) * 10;
        }
        for (int i = 0; i < 8; i++) {
            darr[i] = (iter + i) * 0.2;
        }
        for (int i = 0; i < 32; i++) {
            sarr[i] = (iter + i) * 5;
        }
        
        // Process each data type with blend operations
        __m512 fv1 = _mm512_load_ps(farr);
        __m512 fv2 = _mm512_set1_ps(iter * 0.5f);
        __mmask16 fmask = _mm512_cmp_ps_mask(fv1, fv2, _CMP_GT_OQ);
        __m512 fresult = _mm512_mask_blend_ps(fmask, fv2, fv1);
        _mm512_store_ps(farr, fresult);
        
        __m512d dv1 = _mm512_load_pd(darr);
        __m512d dv2 = _mm512_set1_pd(iter * 0.3);
        __mmask8 dmask = _mm512_cmp_pd_mask(dv1, dv2, _CMP_GT_OQ);
        __m512d dresult = _mm512_mask_blend_pd(dmask, dv2, dv1);
        _mm512_store_pd(darr, dresult);
        
        __m512i iv1 = _mm512_load_si512((__m512i*)iarr);
        __m512i iv2 = _mm512_set1_epi32(iter * 20);
        __mmask16 imask = _mm512_cmp_epi32_mask(iv1, iv2, _MM_CMPINT_GT);
        __m512i iresult = _mm512_mask_blend_epi32(imask, iv2, iv1);
        _mm512_store_si512((__m512i*)iarr, iresult);
        
        __m512i sv1 = _mm512_load_si512((__m512i*)sarr);
        __m512i sv2 = _mm512_set1_epi16(iter * 10);
        __mmask32 smask = _mm512_cmp_epi16_mask(sv1, sv2, _MM_CMPINT_GT);
        __m512i sresult = _mm512_mask_blend_epi16(smask, sv2, sv1);
        _mm512_store_si512((__m512i*)sarr, sresult);
    }
}

#endif  // __AVX512BW__
#endif  // __AVX512F__

/* ==================== Scalar Fallbacks ==================== */
int scalar_test_v64qi_blend() {
    int8_t a[64], b[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = i;
        b[i] = 64 - i;
        result[i] = (a[i] > 32) ? a[i] : b[i];
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
        a[i] = i * 2;
        b[i] = 1000 - i * 3;
        result[i] = (a[i] > 20) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== Main Driver ==================== */
int main() {
    int total_result = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    // Run all vector mode tests
    total_result += test_v64qi_blend();
    total_result += test_v32hi_blend();
    total_result += test_v16si_blend();
    total_result += (int)test_v8di_blend();
    total_result += (int)test_v8df_blend();
    total_result += (int)test_v16sf_blend();
    
#ifdef __AVX512FP16__
    total_result += (int)test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total_result += (int)test_v32bf_blend();
#endif
    
    // Run mixed data type test
    test_mixed_blends(10);
    
#else
    printf("AVX-512BW not available. Running scalar tests...\n");
    total_result += scalar_test_v64qi_blend();
    total_result += scalar_test_v32hi_blend();
#endif
#else
    printf("AVX-512 not available. Running scalar tests...\n");
    total_result += scalar_test_v64qi_blend();
    total_result += scalar_test_v32hi_blend();
#endif
    
    printf("Total checksum: %d\n", total_result);
    return total_result;
}
