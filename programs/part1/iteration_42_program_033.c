#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
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
    
    // Compute checksum to prevent optimization
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
static int test_v32hf_blend(void) {
    _Float16 a[32], b[32], result[32];
    __m512h va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)(16.0f - i * 0.5f);
    }
    
    va = _mm512_loadu_ph(a);
    vb = _mm512_loadu_ph(b);
    
    // Generate mask: elements where a[i] > 8.0
    mask = _mm512_cmp_ph_mask(va, _mm512_set1_ph(8.0f), _CMP_GT_OQ);
    
    vresult = _mm512_mask_blend_ph(mask, va, vb);
    _mm512_storeu_ph(result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)result[i];
    }
    return sum;
}
#endif

/* ========== V32BFmode (32-bfloat16) ========== */
#ifdef __AVX512BF16__
static int test_v32bf_blend(void) {
    __bf16 a[32], b[32], result[32];
    __m512bh va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (__bf16)(i * 0.25f);
        b[i] = (__bf16)(8.0f - i * 0.25f);
    }
    
    va = _mm512_loadu_bf16(a);
    vb = _mm512_loadu_bf16(b);
    
    // For bfloat16, we need to use integer blend since there's no direct FP16 blend
    // This will use the same instruction generator as V32HImode
    __m512i vai = _mm512_castps_si512(_mm512_castbh_ps(va));
    __m512i vbi = _mm512_castps_si512(_mm512_castbh_ps(vb));
    
    // Generate mask based on comparison
    __m512h vah = _mm512_cvtne2ps_pbh(_mm512_setzero_ps(), _mm512_castsi512_ps(vai));
    __m512h vbh = _mm512_cvtne2ps_pbh(_mm512_setzero_ps(), _mm512_castsi512_ps(vbi));
    mask = _mm512_cmp_ph_mask(vah, _mm512_set1_ph(4.0f), _CMP_GT_OQ);
    
    __m512i vresulti = _mm512_mask_blend_epi16(mask, vai, vbi);
    _mm512_storeu_si512((__m512i*)result, vresulti);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)a[i];
    }
    return sum;
}
#endif

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
static int test_v8di_blend(void) {
    long long a[8], b[8], result[8];
    __m512i va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = 80000LL - i * 10000LL;
    }
    
    va = _mm512_loadu_si512((__m512i*)a);
    vb = _mm512_loadu_si512((__m512i*)b);
    
    mask = _mm512_cmpgt_epi64_mask(va, _mm512_set1_epi64(40000));
    
    vresult = _mm512_mask_blend_epi64(mask, va, vb);
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return (int)sum;
}

/* ========== V8DFmode (8-double-precision floats) ========== */
static int test_v8df_blend(void) {
    double a[8], b[8], result[8];
    __m512d va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = 12.0 - i * 1.5;
    }
    
    va = _mm512_loadu_pd(a);
    vb = _mm512_loadu_pd(b);
    
    mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(6.0), _CMP_GT_OQ);
    
    vresult = _mm512_mask_blend_pd(mask, va, vb);
    _mm512_storeu_pd(result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (int)result[i];
    }
    return sum;
}

/* ========== V16SFmode (16-single-precision floats) ========== */
static int test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    __m512 va, vb, vresult;
    __mmask16 mask;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.25f;
        b[i] = 4.0f - i * 0.25f;
    }
    
    va = _mm512_loadu_ps(a);
    vb = _mm512_loadu_ps(b);
    
    mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(2.0f), _CMP_GT_OQ);
    
    vresult = _mm512_mask_blend_ps(mask, va, vb);
    _mm512_storeu_ps(result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (int)result[i];
    }
    return sum;
}

/* ========== Mixed data types in loop ========== */
static int test_mixed_blend_loop(void) {
    const int N = 1024;
    int sum = 0;
    
    // Process different data types in separate loops
    {
        // V16SF mode
        float fa[N], fb[N], fr[N];
        for (int i = 0; i < N; i++) {
            fa[i] = (float)i;
            fb[i] = (float)(N - i);
        }
        
        for (int i = 0; i < N; i += 16) {
            __m512 va = _mm512_loadu_ps(&fa[i]);
            __m512 vb = _mm512_loadu_ps(&fb[i]);
            __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(N/2.0f), _CMP_GT_OQ);
            __m512 vr = _mm512_mask_blend_ps(mask, va, vb);
            _mm512_storeu_ps(&fr[i], vr);
            
            for (int j = 0; j < 16 && (i+j) < N; j++) {
                sum += (int)fr[i+j];
            }
        }
    }
    
    {
        // V8DF mode
        double da[N/2], db[N/2], dr[N/2];
        for (int i = 0; i < N/2; i++) {
            da[i] = (double)i;
            db[i] = (double)(N/2 - i);
        }
        
        for (int i = 0; i < N/2; i += 8) {
            __m512d va = _mm512_loadu_pd(&da[i]);
            __m512d vb = _mm512_loadu_pd(&db[i]);
            __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(N/4.0), _CMP_GT_OQ);
            __m512d vr = _mm512_mask_blend_pd(mask, va, vb);
            _mm512_storeu_pd(&dr[i], vr);
            
            for (int j = 0; j < 8 && (i+j) < N/2; j++) {
                sum += (int)dr[i+j];
            }
        }
    }
    
    {
        // V16SI mode
        int ia[N], ib[N], ir[N];
        for (int i = 0; i < N; i++) {
            ia[i] = i * 10;
            ib[i] = (N - i) * 10;
        }
        
        for (int i = 0; i < N; i += 16) {
            __m512i va = _mm512_loadu_si512((__m512i*)&ia[i]);
            __m512i vb = _mm512_loadu_si512((__m512i*)&ib[i]);
            __mmask16 mask = _mm512_cmpgt_epi32_mask(va, _mm512_set1_epi32(N*5));
            __m512i vr = _mm512_mask_blend_epi32(mask, va, vb);
            _mm512_storeu_si512((__m512i*)&ir[i], vr);
            
            for (int j = 0; j < 16 && (i+j) < N; j++) {
                sum += ir[i+j];
            }
        }
    }
    
    return sum;
}

#endif // __AVX512BW__
#endif // __AVX512F__

/* ========== Scalar fallback implementations ========== */
static int scalar_test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = i;
        b[i] = 64 - i;
        result[i] = (a[i] > 32) ? b[i] : a[i];
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
        a[i] = i * 0.25f;
        b[i] = 4.0f - i * 0.25f;
        result[i] = (a[i] > 2.0f) ? b[i] : a[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (int)result[i];
    }
    return sum;
}

/* ========== Main driver ========== */
int main(void) {
    int total_sum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    total_sum += test_v64qi_blend();
    printf("V64QImode test completed\n");
    
    total_sum += test_v32hi_blend();
    printf("V32HImode test completed\n");
    
#ifdef __AVX512FP16__
    total_sum += test_v32hf_blend();
    printf("V32HFmode test completed\n");
#endif
    
#ifdef __AVX512BF16__
    total_sum += test_v32bf_blend();
    printf("V32BFmode test completed\n");
#endif
    
    total_sum += test_v16si_blend();
    printf("V16SImode test completed\n");
    
    total_sum += test_v8di_blend();
    printf("V8DImode test completed\n");
    
    total_sum += test_v8df_blend();
    printf("V8DFmode test completed\n");
    
    total_sum += test_v16sf_blend();
    printf("V16SFmode test completed\n");
    
    total_sum += test_mixed_blend_loop();
    printf("Mixed mode loop test completed\n");
    
#else
    printf("AVX-512BW not available. Running scalar tests...\n");
    total_sum += scalar_test_v64qi_blend();
    total_sum += scalar_test_v16sf_blend();
#endif
#else
    printf("AVX-512 not available. Running minimal scalar tests...\n");
    total_sum += scalar_test_v64qi_blend();
    total_sum += scalar_test_v16sf_blend();
#endif
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum;
}
