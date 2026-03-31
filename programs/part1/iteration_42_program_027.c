#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>
#include <x86intrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ========== V64QImode (64-byte integers) ========== */
static int test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    __m512i va, vb, vresult;
    __mmask64 mask;
    
    // Initialize with pattern
    for (int i = 0; i < 64; i++) {
        a[i] = i;
        b[i] = 64 - i;
    }
    
    va = _mm512_loadu_si512((__m512i*)a);
    vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask: elements where a[i] > 32
    mask = _mm512_cmpgt_epi8_mask(va, _mm512_set1_epi8(32));
    
    // Blend based on mask
    vresult = _mm512_mask_blend_epi8(mask, va, vb);
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    // Compute checksum
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V32HImode (32-halfword integers) ========== */
static int test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    __m512i va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = i * 100;
        b[i] = 3200 - i * 100;
    }
    
    va = _mm512_loadu_si512((__m512i*)a);
    vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate mask: elements where a[i] > 1600
    mask = _mm512_cmpgt_epi16_mask(va, _mm512_set1_epi16(1600));
    
    vresult = _mm512_mask_blend_epi16(mask, va, vb);
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V32HFmode (32-half-precision floats) ========== */
#ifdef __AVX512FP16__
static float test_v32hf_blend(void) {
    _Float16 a[32], b[32], result[32];
    __m512h va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = i * 0.5f;
        b[i] = 16.0f - i * 0.5f;
    }
    
    va = _mm512_loadu_ph(a);
    vb = _mm512_loadu_ph(b);
    
    // Generate mask: elements where a[i] > 8.0
    mask = _mm512_cmp_ph_mask(va, _mm512_set1_ph(8.0f), _CMP_GT_OQ);
    
    vresult = _mm512_mask_blend_ph(mask, va, vb);
    _mm512_storeu_ph(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}
#endif

/* ========== V32BFmode (32-bfloat16) ========== */
#ifdef __AVX512BF16__
static float test_v32bf_blend(void) {
    __bfloat16 a[32], b[32], result[32];
    __m512bh va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float(i * 0.5f);
        b[i] = bfloat16_from_float(16.0f - i * 0.5f);
    }
    
    va = _mm512_loadu_si512((__m512i*)a);
    vb = _mm512_loadu_si512((__m512i*)b);
    
    // For bfloat16, we need to use integer blend since there's no direct bfloat16 blend
    // This should still trigger the V32BFmode case
    mask = 0xAAAAAAAA; // Alternating pattern
    
    vresult = _mm512_mask_blend_epi16(mask, va, vb);
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(result[i]);
    }
    return sum;
}
#endif

/* ========== V16SFmode (16-single-precision floats) ========== */
static float test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    __m512 va, vb, vresult;
    __mmask16 mask;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1.5f;
        b[i] = 24.0f - i * 1.5f;
    }
    
    va = _mm512_loadu_ps(a);
    vb = _mm512_loadu_ps(b);
    
    // Generate mask: elements where a[i] > 12.0
    mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(12.0f), _CMP_GT_OQ);
    
    vresult = _mm512_mask_blend_ps(mask, va, vb);
    _mm512_storeu_ps(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V8DFmode (8-double-precision floats) ========== */
static double test_v8df_blend(void) {
    double a[8], b[8], result[8];
    __m512d va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 2.5;
        b[i] = 20.0 - i * 2.5;
    }
    
    va = _mm512_loadu_pd(a);
    vb = _mm512_loadu_pd(b);
    
    // Generate mask: elements where a[i] > 10.0
    mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(10.0), _CMP_GT_OQ);
    
    vresult = _mm512_mask_blend_pd(mask, va, vb);
    _mm512_storeu_pd(result, vresult);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V16SImode (16-dword integers) ========== */
static int test_v16si_blend(void) {
    int a[16], b[16], result[16];
    __m512i va, vb, vresult;
    __mmask16 mask;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = 16000 - i * 1000;
    }
    
    va = _mm512_loadu_si512((__m512i*)a);
    vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate mask: elements where a[i] > 8000
    mask = _mm512_cmpgt_epi32_mask(va, _mm512_set1_epi32(8000));
    
    vresult = _mm512_mask_blend_epi32(mask, va, vb);
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V8DImode (8-qword integers) ========== */
static long long test_v8di_blend(void) {
    long long a[8], b[8], result[8];
    __m512i va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = 80000LL - i * 10000LL;
    }
    
    va = _mm512_loadu_si512((__m512i*)a);
    vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate mask: elements where a[i] > 40000
    mask = _mm512_cmpgt_epi64_mask(va, _mm512_set1_epi64(40000));
    
    vresult = _mm512_mask_blend_epi64(mask, va, vb);
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== Mixed data types in loop ========== */
static int test_mixed_blend_loop(void) {
    const int N = 1024;
    int checksum = 0;
    
    // Process different data types in separate loops
    {
        // V64QImode
        char data1[N], data2[N], result[N];
        for (int i = 0; i < N; i += 64) {
            for (int j = 0; j < 64; j++) {
                data1[i + j] = (i + j) & 0xFF;
                data2[i + j] = ~data1[i + j];
            }
            
            __m512i v1 = _mm512_loadu_si512((__m512i*)(data1 + i));
            __m512i v2 = _mm512_loadu_si512((__m512i*)(data2 + i));
            __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, _mm512_set1_epi8(128));
            __m512i vresult = _mm512_mask_blend_epi8(mask, v1, v2);
            _mm512_storeu_si512((__m512i*)(result + i), vresult);
            
            for (int j = 0; j < 64; j++) {
                checksum += result[i + j];
            }
        }
    }
    
    {
        // V32HImode
        short data1[N], data2[N], result[N];
        for (int i = 0; i < N; i += 32) {
            for (int j = 0; j < 32; j++) {
                data1[i + j] = (i + j) * 10;
                data2[i + j] = -data1[i + j];
            }
            
            __m512i v1 = _mm512_loadu_si512((__m512i*)(data1 + i));
            __m512i v2 = _mm512_loadu_si512((__m512i*)(data2 + i));
            __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, _mm512_set1_epi16(N/2));
            __m512i vresult = _mm512_mask_blend_epi16(mask, v1, v2);
            _mm512_storeu_si512((__m512i*)(result + i), vresult);
            
            for (int j = 0; j < 32; j++) {
                checksum += result[i + j];
            }
        }
    }
    
    return checksum;
}

#endif // __AVX512BW__
#endif // __AVX512F__

/* ========== Scalar fallback implementations ========== */
static int scalar_test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = i;
        b[i] = 64 - i;
    }
    
    for (int i = 0; i < 64; i++) {
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
        a[i] = i * 100;
        b[i] = 3200 - i * 100;
    }
    
    for (int i = 0; i < 32; i++) {
        result[i] = (a[i] > 1600) ? b[i] : a[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== Main driver ========== */
int main(void) {
    int total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 supported, using vectorized blend operations\n");
    
    total_checksum += test_v64qi_blend();
    printf("V64QImode blend checksum: %d\n", test_v64qi_blend());
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode blend checksum: %d\n", test_v32hi_blend());
    
#ifdef __AVX512FP16__
    total_checksum += (int)test_v32hf_blend();
    printf("V32HFmode blend checksum: %f\n", test_v32hf_blend());
#endif
    
#ifdef __AVX512BF16__
    total_checksum += (int)test_v32bf_blend();
    printf("V32BFmode blend checksum: %f\n", test_v32bf_blend());
#endif
    
    total_checksum += (int)test_v16sf_blend();
    printf("V16SFmode blend checksum: %f\n", test_v16sf_blend());
    
    total_checksum += (int)test_v8df_blend();
    printf("V8DFmode blend checksum: %f\n", test_v8df_blend());
    
    total_checksum += test_v16si_blend();
    printf("V16SImode blend checksum: %d\n", test_v16si_blend());
    
    total_checksum += (int)test_v8di_blend();
    printf("V8DImode blend checksum: %lld\n", test_v8di_blend());
    
    total_checksum += test_mixed_blend_loop();
    printf("Mixed blend loop checksum: %d\n", test_mixed_blend_loop());
    
#else
    printf("AVX-512BW not supported, using scalar fallback\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
#endif
#else
    printf("AVX-512 not supported, using scalar fallback\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
#endif
    
    printf("Total checksum: %d\n", total_checksum);
    return total_checksum != 0 ? 0 : 1;
}
