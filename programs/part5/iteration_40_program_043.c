#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef __AVX512BW__
/* V64QImode: 64 x 8-bit integers */
__m512i test_v64qimode_blend() {
    __m512i a = _mm512_set1_epi8(0xAA);
    __m512i b = _mm512_set1_epi8(0x55);
    
    /* Create alternating mask: 0x5555... pattern */
    __mmask64 mask = 0x5555555555555555ULL;
    
    /* This should generate vblendmb */
    return _mm512_mask_blend_epi8(mask, a, b);
}

/* V32HImode: 32 x 16-bit integers */
__m512i test_v32himode_blend() {
    __m512i a = _mm512_set1_epi16(0xAAAA);
    __m512i b = _mm512_set1_epi16(0x5555);
    
    /* Create mask with alternating bits for 32 elements */
    __mmask32 mask = 0x55555555;
    
    /* This should generate vblendmw */
    return _mm512_mask_blend_epi16(mask, a, b);
}
#endif /* __AVX512BW__ */

#ifdef __AVX512FP16__
/* V32HFmode: 32 x half-precision floats */
#if defined(__AVX512FP16__) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
__m512h test_v32hfmode_blend() {
    _Float16 a_vals[32], b_vals[32];
    for (int i = 0; i < 32; i++) {
        a_vals[i] = (_Float16)(i * 1.0f);
        b_vals[i] = (_Float16)(i * 2.0f);
    }
    
    __m512h a = _mm512_loadu_ph(a_vals);
    __m512h b = _mm512_loadu_ph(b_vals);
    
    /* Select even elements from a, odd from b */
    __mmask32 mask = 0xAAAAAAAA;  /* 1010... pattern */
    
    /* This should generate vblendmps for half-precision */
    return _mm512_mask_blend_ph(mask, a, b);
}
#endif

/* V32BFmode: 32 x brain float (bfloat16) */
__m512bh test_v32bfmode_blend() {
    /* Initialize with simple patterns */
    __m512bh a = _mm512_set1_epi16(0x3F80);  /* ~1.0 in BF16 */
    __m512bh b = _mm512_set1_epi16(0x4000);  /* ~2.0 in BF16 */
    
    /* Create alternating mask */
    __mmask32 mask = 0x55555555;
    
    /* Use integer blend for BF16 elements */
    return _mm512_mask_blend_epi16(mask, a, b);
}
#endif /* __AVX512FP16__ */

#ifdef __AVX512F__
/* V16SImode: 16 x 32-bit integers */
__m512i test_v16simode_blend() {
    __m512i a = _mm512_set1_epi32(0xAAAAAAAA);
    __m512i b = _mm512_set1_epi32(0x55555555);
    
    /* Mask for 16 elements */
    __mmask16 mask = 0xAAAA;  /* 1010... pattern */
    
    /* This should generate vblendmd */
    return _mm512_mask_blend_epi32(mask, a, b);
}

/* V8DImode: 8 x 64-bit integers */
__m512i test_v8dimode_blend() {
    __m512i a = _mm512_set1_epi64(0xAAAAAAAAAAAAAAAAULL);
    __m512i b = _mm512_set1_epi64(0x5555555555555555ULL);
    
    /* Mask for 8 elements */
    __mmask8 mask = 0xAA;  /* 10101010 */
    
    /* This should generate vblendmq */
    return _mm512_mask_blend_epi64(mask, a, b);
}

/* V8DFmode: 8 x double-precision floats */
__m512d test_v8dfmode_blend() {
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    
    /* Mask for 8 elements */
    __mmask8 mask = 0x55;  /* 01010101 */
    
    /* This should generate vblendmpd */
    return _mm512_mask_blend_pd(mask, a, b);
}

/* V16SFmode: 16 x single-precision floats */
__m512 test_v16sfmode_blend() {
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    
    /* Mask for 16 elements */
    __mmask16 mask = 0x5555;  /* 0101... pattern */
    
    /* This should generate vblendmps */
    return _mm512_mask_blend_ps(mask, a, b);
}
#endif /* __AVX512F__ */

/* Helper function to prevent dead code elimination */
volatile int g_volatile_sink = 0;

int main() {
    int checksum = 0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode (64x8-bit integers)...\n");
    __m512i v64qi_result = test_v64qimode_blend();
    g_volatile_sink = _mm512_extract_epi32(v64qi_result, 0);
    checksum += g_volatile_sink;
    
    printf("Testing V32HImode (32x16-bit integers)...\n");
    __m512i v32hi_result = test_v32himode_blend();
    g_volatile_sink = _mm512_extract_epi32(v32hi_result, 0);
    checksum += g_volatile_sink;
#endif
    
#ifdef __AVX512FP16__
    printf("Testing V32BFmode (32x bfloat16)...\n");
    __m512bh v32bf_result = test_v32bfmode_blend();
    g_volatile_sink = _mm512_extract_epi32((__m512i)v32bf_result, 0);
    checksum += g_volatile_sink;
    
#if defined(__AVX512FP16__) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
    printf("Testing V32HFmode (32x half-precision)...\n");
    __m512h v32hf_result = test_v32hfmode_blend();
    g_volatile_sink = _mm512_extract_epi32((__m512i)v32hf_result, 0);
    checksum += g_volatile_sink;
#endif
#endif
    
#ifdef __AVX512F__
    printf("Testing V16SImode (16x32-bit integers)...\n");
    __m512i v16si_result = test_v16simode_blend();
    g_volatile_sink = _mm512_extract_epi32(v16si_result, 0);
    checksum += g_volatile_sink;
    
    printf("Testing V8DImode (8x64-bit integers)...\n");
    __m512i v8di_result = test_v8dimode_blend();
    g_volatile_sink = _mm512_extract_epi32(v8di_result, 0);
    checksum += g_volatile_sink;
    
    printf("Testing V8DFmode (8x double-precision)...\n");
    __m512d v8df_result = test_v8dfmode_blend();
    g_volatile_sink = (int)_mm512_cvtsd_f64(v8df_result);
    checksum += g_volatile_sink;
    
    printf("Testing V16SFmode (16x single-precision)...\n");
    __m512 v16sf_result = test_v16sfmode_blend();
    g_volatile_sink = (int)_mm512_cvtss_f32(v16sf_result);
    checksum += g_volatile_sink;
#endif
    
    printf("All tests completed. Checksum: %d\n", checksum);
    
    /* Process arrays to ensure blends are actually used */
    const int ARRAY_SIZE = 1024;
    static int32_t src1[ARRAY_SIZE], src2[ARRAY_SIZE], dst[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src1[i] = i;
        src2[i] = i * 2;
    }
    
    /* Process in AVX-512 chunks */
    for (int i = 0; i < ARRAY_SIZE; i += 16) {
#ifdef __AVX512F__
        __m512i a = _mm512_loadu_si512((__m512i*)&src1[i]);
        __m512i b = _mm512_loadu_si512((__m512i*)&src2[i]);
        
        /* Use comparison to create dynamic mask */
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(ARRAY_SIZE/2));
        
        /* This should trigger vblendmd */
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        _mm512_storeu_si512((__m512i*)&dst[i], result);
#endif
    }
    
    /* Verify some results */
    int verify_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        verify_sum += dst[i];
    }
    printf("Array processing completed. Verify sum: %d\n", verify_sum);
    
    return 0;
}
