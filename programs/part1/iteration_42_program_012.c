#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>
#include <x86intrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64-byte integers */
static uint64_t test_v64qi_blend(void) {
    alignas(64) int8_t a[64], b[64], result[64];
    uint64_t checksum = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        a[i] = (int8_t)(i * 2);
        b[i] = (int8_t)(i * 3 + 1);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    /* Generate dynamic mask using comparison */
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 64; i++) {
        checksum += (uint64_t)result[i];
    }
    
    return checksum;
}

/* V32HImode: 32-halfword integers */
static uint64_t test_v32hi_blend(void) {
    alignas(64) int16_t a[32], b[32], result[32];
    uint64_t checksum = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (int16_t)(i * 100);
        b[i] = (int16_t)(i * 150 + 50);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    for (int i = 0; i < 32; i++) {
        checksum += (uint64_t)result[i];
    }
    
    return checksum;
}

/* V32HFmode: 32-half-precision floats */
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) {
    alignas(64) _Float16 a[32], b[32], result[32];
    uint64_t checksum = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.0f + 0.5f);
    }
    
    __m512h va = _mm512_load_ph((const __m512h*)a);
    __m512h vb = _mm512_load_ph((const __m512h*)b);
    
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph((__m512h*)result, vresult);
    
    for (int i = 0; i < 32; i++) {
        checksum += (uint64_t)(result[i] * 1000);
    }
    
    return checksum;
}
#endif

/* V32BFmode: 32-bfloat16 floats */
#ifdef __AVX512BF16__
static uint64_t test_v32bf_blend(void) {
    alignas(64) __bf16 a[32], b[32], result[32];
    uint64_t checksum = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (__bf16)(i * 1.2f);
        b[i] = (__bf16)(i * 1.8f + 0.3f);
    }
    
    /* For bfloat16, we need to use integer blend since there's no direct bfloat16 blend */
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    /* Create mask based on comparison of converted values */
    __m512 va_f32 = _mm512_cvtpbh_ps(_mm512_castsi512_si256(va));
    __m512 vb_f32 = _mm512_cvtpbh_ps(_mm512_castsi512_si256(vb));
    __mmask16 mask32 = _mm512_cmp_ps_mask(va_f32, vb_f32, _CMP_GT_OQ);
    
    /* Expand mask from 16-bit to 32-bit for 32 elements */
    __mmask32 mask = _cvtu32_mask32(_cvtmask16_u32(mask32));
    mask = mask | (mask << 16);
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    for (int i = 0; i < 32; i++) {
        checksum += (uint64_t)(result[i] * 1000);
    }
    
    return checksum;
}
#endif

/* V16SImode: 16-dword integers */
static uint64_t test_v16si_blend(void) {
    alignas(64) int32_t a[16], b[16], result[16];
    uint64_t checksum = 0;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 1500 + 500;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    for (int i = 0; i < 16; i++) {
        checksum += (uint64_t)result[i];
    }
    
    return checksum;
}

/* V8DImode: 8-qword integers */
static uint64_t test_v8di_blend(void) {
    alignas(64) int64_t a[8], b[8], result[8];
    uint64_t checksum = 0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = i * 15000LL + 5000LL;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_NE);
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)result[i];
    }
    
    return checksum;
}

/* V8DFmode: 8-double-precision floats */
static uint64_t test_v8df_blend(void) {
    alignas(64) double a[8], b[8], result[8];
    uint64_t checksum = 0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.0 + 0.5;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LE_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_store_pd(result, vresult);
    
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)(result[i] * 1000);
    }
    
    return checksum;
}

/* V16SFmode: 16-single-precision floats */
static uint64_t test_v16sf_blend(void) {
    alignas(64) float a[16], b[16], result[16];
    uint64_t checksum = 0;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1.1f;
        b[i] = i * 1.8f + 0.3f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GE_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_store_ps(result, vresult);
    
    for (int i = 0; i < 16; i++) {
        checksum += (uint64_t)(result[i] * 1000);
    }
    
    return checksum;
}

/* Mixed data types in loop structure */
static uint64_t test_mixed_blends(void) {
    const int N = 1024;
    alignas(64) int8_t data8[N];
    alignas(64) int16_t data16[N/2];
    alignas(64) int32_t data32[N/4];
    alignas(64) float dataf[N/4];
    alignas(64) double datad[N/8];
    
    uint64_t checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) data8[i] = (int8_t)(i % 256);
    for (int i = 0; i < N/2; i++) data16[i] = (int16_t)(i * 3);
    for (int i = 0; i < N/4; i++) data32[i] = i * 100;
    for (int i = 0; i < N/4; i++) dataf[i] = i * 1.5f;
    for (int i = 0; i < N/8; i++) datad[i] = i * 2.5;
    
    /* Process in vectorized chunks */
    for (int i = 0; i < N/64; i++) {
        __m512i va8 = _mm512_load_si512((const __m512i*)(data8 + i*64));
        __m512i vb8 = _mm512_load_si512((const __m512i*)(data8 + (i+1)*64 % N));
        __mmask64 mask8 = _mm512_cmp_epi8_mask(va8, vb8, _MM_CMPINT_GT);
        __m512i vr8 = _mm512_mask_blend_epi8(mask8, va8, vb8);
        _mm512_store_si512((__m512i*)(data8 + i*64), vr8);
    }
    
    for (int i = 0; i < N/2/32; i++) {
        __m512i va16 = _mm512_load_si512((const __m512i*)(data16 + i*32));
        __m512i vb16 = _mm512_load_si512((const __m512i*)(data16 + (i+1)*32 % (N/2)));
        __mmask32 mask16 = _mm512_cmp_epi16_mask(va16, vb16, _MM_CMPINT_LT);
        __m512i vr16 = _mm512_mask_blend_epi16(mask16, va16, vb16);
        _mm512_store_si512((__m512i*)(data16 + i*32), vr16);
    }
    
    for (int i = 0; i < N/4/16; i++) {
        __m512i va32 = _mm512_load_si512((const __m512i*)(data32 + i*16));
        __m512i vb32 = _mm512_load_si512((const __m512i*)(data32 + (i+1)*16 % (N/4)));
        __mmask16 mask32 = _mm512_cmp_epi32_mask(va32, vb32, _MM_CMPINT_EQ);
        __m512i vr32 = _mm512_mask_blend_epi32(mask32, va32, vb32);
        _mm512_store_si512((__m512i*)(data32 + i*16), vr32);
    }
    
    for (int i = 0; i < N/4/16; i++) {
        __m512 vaf = _mm512_load_ps(dataf + i*16);
        __m512 vbf = _mm512_load_ps(dataf + (i+1)*16 % (N/4));
        __mmask16 maskf = _mm512_cmp_ps_mask(vaf, vbf, _CMP_GE_OQ);
        __m512 vrf = _mm512_mask_blend_ps(maskf, vaf, vbf);
        _mm512_store_ps(dataf + i*16, vrf);
    }
    
    for (int i = 0; i < N/8/8; i++) {
        __m512d vad = _mm512_load_pd(datad + i*8);
        __m512d vbd = _mm512_load_pd(datad + (i+1)*8 % (N/8));
        __mmask8 maskd = _mm512_cmp_pd_mask(vad, vbd, _CMP_LE_OQ);
        __m512d vrd = _mm512_mask_blend_pd(maskd, vad, vbd);
        _mm512_store_pd(datad + i*8, vrd);
    }
    
    /* Compute final checksum */
    for (int i = 0; i < N; i++) checksum += data8[i];
    for (int i = 0; i < N/2; i++) checksum += data16[i];
    for (int i = 0; i < N/4; i++) checksum += data32[i];
    for (int i = 0; i < N/4; i++) checksum += (uint64_t)(dataf[i] * 100);
    for (int i = 0; i < N/8; i++) checksum += (uint64_t)(datad[i] * 100);
    
    return checksum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* Scalar fallback implementations */
static uint64_t scalar_test_v64qi_blend(void) {
    int8_t a[64], b[64], result[64];
    uint64_t checksum = 0;
    
    for (int i = 0; i < 64; i++) {
        a[i] = (int8_t)(i * 2);
        b[i] = (int8_t)(i * 3 + 1);
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
        checksum += (uint64_t)result[i];
    }
    
    return checksum;
}

static uint64_t scalar_test_v32hi_blend(void) {
    int16_t a[32], b[32], result[32];
    uint64_t checksum = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (int16_t)(i * 100);
        b[i] = (int16_t)(i * 150 + 50);
        result[i] = (a[i] < b[i]) ? a[i] : b[i];
        checksum += (uint64_t)result[i];
    }
    
    return checksum;
}

/* ... similar scalar fallbacks for other modes ... */

int main(void) {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    total_checksum += test_v64qi_blend();
    printf("V64QImode blend test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode blend test completed\n");
    
#ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend();
    printf("V32HFmode blend test completed\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += test_v32bf_blend();
    printf("V32BFmode blend test completed\n");
#endif
    
    total_checksum += test_v16si_blend();
    printf("V16SImode blend test completed\n");
    
    total_checksum += test_v8di_blend();
    printf("V8DImode blend test completed\n");
    
    total_checksum += test_v8df_blend();
    printf("V8DFmode blend test completed\n");
    
    total_checksum += test_v16sf_blend();
    printf("V16SFmode blend test completed\n");
    
    total_checksum += test_mixed_blends();
    printf("Mixed mode blend test completed\n");
    
#else
    printf("AVX-512BW not supported. Running scalar tests...\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
#endif
#else
    printf("AVX-512 not supported. Running scalar tests...\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    return (int)(total_checksum % 256);
}
