#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
static int test_v64qi_blend(void) {
    char a[64], b[64];
    __m512i va, vb, vresult;
    __mmask64 mask;
    int sum = 0;
    
    // Initialize with alternating pattern
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 2);
        b[i] = (char)(i * 3);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    // Generate dynamic mask: elements where a[i] > 32
    mask = _mm512_cmp_epi8_mask(va, _mm512_set1_epi8(32), _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv64qi
    vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    // Store and compute checksum to prevent optimization
    char result[64];
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
static int test_v32hi_blend(void) {
    short a[32], b[32];
    __m512i va, vb, vresult;
    __mmask32 mask;
    int sum = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 10);
        b[i] = (short)(i * 15);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    // Generate mask: elements where a[i] > 160
    mask = _mm512_cmp_epi16_mask(va, _mm512_set1_epi16(160), _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv32hi
    vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    short result[32];
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
static int test_v32hf_blend(void) {
    _Float16 a[32], b[32];
    __m512h va, vb, vresult;
    __mmask32 mask;
    int sum = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)(i * 0.75f);
    }
    
    va = _mm512_loadu_ph(a);
    vb = _mm512_loadu_ph(b);
    
    // Generate mask: elements where a[i] > 8.0
    mask = _mm512_cmp_ph_mask(va, _mm512_set1_ph(8.0f), _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _Float16 result[32];
    _mm512_storeu_ph(result, vresult);
    
    for (int i = 0; i < 32; i++) {
        sum += (int)(result[i] * 100);
    }
    
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
static int test_v32bf_blend(void) {
    __bfloat16 a[32], b[32];
    __m512bh va, vb, vresult;
    __mmask32 mask;
    int sum = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float((float)i * 0.5f);
        b[i] = bfloat16_from_float((float)i * 0.75f);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    // For bfloat16, we need to use integer blend since there's no direct bfloat16 blend
    // This still exercises the V32BFmode case
    mask = _mm512_cmp_epi16_mask((__m512i)va, _mm512_set1_epi16(0x4000), _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv32bf
    vresult = (__m512bh)_mm512_mask_blend_epi16(mask, (__m512i)va, (__m512i)vb);
    
    __bfloat16 result[32];
    _mm512_storeu_si512((__m512i*)result, (__m512i)vresult);
    
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_uint32(result[i]);
    }
    
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
static int test_v16si_blend(void) {
    int a[16], b[16];
    __m512i va, vb, vresult;
    __mmask16 mask;
    int sum = 0;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 100;
        b[i] = i * 150;
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    // Generate mask: elements where a[i] > 800
    mask = _mm512_cmp_epi32_mask(va, _mm512_set1_epi32(800), _MM_CMPINT_GT);
    
    // This should trigger gen_avx512f_blendmv16si
    vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    int result[16];
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
static long long test_v8di_blend(void) {
    long long a[8], b[8];
    __m512i va, vb, vresult;
    __mmask8 mask;
    long long sum = 0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = (long long)i * 1000LL;
        b[i] = (long long)i * 1500LL;
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    // Generate mask: elements where a[i] > 4000
    mask = _mm512_cmp_epi64_mask(va, _mm512_set1_epi64(4000), _MM_CMPINT_GT);
    
    // This should trigger gen_avx512f_blendmv8di
    vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    long long result[8];
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    return sum;
}

/* ==================== V8DFmode (8-double-precision floats) ==================== */
static double test_v8df_blend(void) {
    double a[8], b[8];
    __m512d va, vb, vresult;
    __mmask8 mask;
    double sum = 0.0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = (double)i * 1.5;
        b[i] = (double)i * 2.5;
    }
    
    va = _mm512_loadu_pd(a);
    vb = _mm512_loadu_pd(b);
    
    // Generate mask: elements where a[i] > 6.0
    mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(6.0), _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    double result[8];
    _mm512_storeu_pd(result, vresult);
    
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    return sum;
}

/* ==================== V16SFmode (16-single-precision floats) ==================== */
static float test_v16sf_blend(void) {
    float a[16], b[16];
    __m512 va, vb, vresult;
    __mmask16 mask;
    float sum = 0.0f;
    
    for (int i = 0; i < 16; i++) {
        a[i] = (float)i * 0.5f;
        b[i] = (float)i * 0.75f;
    }
    
    va = _mm512_loadu_ps(a);
    vb = _mm512_loadu_ps(b);
    
    // Generate mask: elements where a[i] > 4.0
    mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(4.0f), _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    float result[16];
    _mm512_storeu_ps(result, vresult);
    
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
static int test_mixed_blend_loop(void) {
    const int N = 1024;
    int sum = 0;
    
    // Process different data types in loops
    {
        // V16SFmode
        float fa[N], fb[N];
        for (int i = 0; i < N; i++) {
            fa[i] = (float)(i % 32) * 0.1f;
            fb[i] = (float)(i % 32) * 0.2f;
        }
        
        for (int i = 0; i < N; i += 16) {
            __m512 va = _mm512_loadu_ps(&fa[i]);
            __m512 vb = _mm512_loadu_ps(&fb[i]);
            __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(1.5f), _CMP_GT_OQ);
            __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
            
            float result[16];
            _mm512_storeu_ps(result, vresult);
            
            for (int j = 0; j < 16; j++) {
                sum += (int)(result[j] * 100);
            }
        }
    }
    
    {
        // V8DFmode
        double da[N/2], db[N/2];
        for (int i = 0; i < N/2; i++) {
            da[i] = (double)(i % 16) * 0.2;
            db[i] = (double)(i % 16) * 0.3;
        }
        
        for (int i = 0; i < N/2; i += 8) {
            __m512d va = _mm512_loadu_pd(&da[i]);
            __m512d vb = _mm512_loadu_pd(&db[i]);
            __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(2.0), _CMP_GT_OQ);
            __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
            
            double result[8];
            _mm512_storeu_pd(result, vresult);
            
            for (int j = 0; j < 8; j++) {
                sum += (int)(result[j] * 100);
            }
        }
    }
    
    return sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ==================== Scalar Fallback Implementations ==================== */
static int scalar_test_v64qi_blend(void) {
    char a[64], b[64];
    int sum = 0;
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 2);
        b[i] = (char)(i * 3);
    }
    
    for (int i = 0; i < 64; i++) {
        char result = (a[i] > 32) ? b[i] : a[i];
        sum += result;
    }
    
    return sum;
}

static int scalar_test_v32hi_blend(void) {
    short a[32], b[32];
    int sum = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 10);
        b[i] = (short)(i * 15);
    }
    
    for (int i = 0; i < 32; i++) {
        short result = (a[i] > 160) ? b[i] : a[i];
        sum += result;
    }
    
    return sum;
}

static int scalar_test_v16si_blend(void) {
    int a[16], b[16];
    int sum = 0;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 100;
        b[i] = i * 150;
    }
    
    for (int i = 0; i < 16; i++) {
        int result = (a[i] > 800) ? b[i] : a[i];
        sum += result;
    }
    
    return sum;
}

static long long scalar_test_v8di_blend(void) {
    long long a[8], b[8];
    long long sum = 0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = (long long)i * 1000LL;
        b[i] = (long long)i * 1500LL;
    }
    
    for (int i = 0; i < 8; i++) {
        long long result = (a[i] > 4000) ? b[i] : a[i];
        sum += result;
    }
    
    return sum;
}

static float scalar_test_v16sf_blend(void) {
    float a[16], b[16];
    float sum = 0.0f;
    
    for (int i = 0; i < 16; i++) {
        a[i] = (float)i * 0.5f;
        b[i] = (float)i * 0.75f;
    }
    
    for (int i = 0; i < 16; i++) {
        float result = (a[i] > 4.0f) ? b[i] : a[i];
        sum += result;
    }
    
    return sum;
}

static double scalar_test_v8df_blend(void) {
    double a[8], b[8];
    double sum = 0.0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = (double)i * 1.5;
        b[i] = (double)i * 2.5;
    }
    
    for (int i = 0; i < 8; i++) {
        double result = (a[i] > 6.0) ? b[i] : a[i];
        sum += result;
    }
    
    return sum;
}

/* ==================== Main Driver ==================== */
int main(void) {
    int total_sum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running optimized versions...\n");
    
    total_sum += test_v64qi_blend();
    total_sum += test_v32hi_blend();
    
#ifdef __AVX512FP16__
    total_sum += test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total_sum += test_v32bf_blend();
#endif
    
    total_sum += test_v16si_blend();
    total_sum += (int)test_v8di_blend();
    total_sum += (int)test_v8df_blend();
    total_sum += (int)test_v16sf_blend();
    total_sum += test_mixed_blend_loop();
    
#else
    printf("AVX-512BW not available. Running scalar fallbacks...\n");
    goto scalar_fallback;
#endif
#else
    printf("AVX-512 not available. Running scalar fallbacks...\n");
    goto scalar_fallback;
#endif

    printf("Total checksum: %d\n", total_sum);
    return total_sum & 0xFF;  // Return lower 8 bits as exit code

scalar_fallback:
    total_sum += scalar_test_v64qi_blend();
    total_sum += scalar_test_v32hi_blend();
    total_sum += scalar_test_v16si_blend();
    total_sum += (int)scalar_test_v8di_blend();
    total_sum += (int)scalar_test_v8df_blend();
    total_sum += (int)scalar_test_v16sf_blend();
    
    printf("Scalar total checksum: %d\n", total_sum);
    return total_sum & 0xFF;
}
