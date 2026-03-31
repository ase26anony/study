#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>
#include <x86intrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
static int test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    __m512i va, vb, vresult;
    __mmask64 mask = 0;
    
    /* Initialize with alternating pattern */
    for (int i = 0; i < 64; i++) {
        a[i] = i;
        b[i] = 64 - i;
    }
    
    va = _mm512_loadu_si512((__m512i*)a);
    vb = _mm512_loadu_si512((__m512i*)b);
    
    /* Create dynamic mask: select a[i] if i % 2 == 0, else b[i] */
    for (int i = 0; i < 64; i++) {
        if (i % 2 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    vresult = _mm512_mask_blend_epi8(mask, vb, va);
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    /* Compute checksum to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
static int test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    __m512i va, vb, vresult;
    __mmask32 mask = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = i * 2;
        b[i] = 1000 - i * 3;
    }
    
    va = _mm512_loadu_si512((__m512i*)a);
    vb = _mm512_loadu_si512((__m512i*)b);
    
    /* Generate mask using comparison */
    __m512i zero = _mm512_setzero_si512();
    __mmask32 cmp_mask = _mm512_cmpgt_epi16_mask(va, zero);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    vresult = _mm512_mask_blend_epi16(cmp_mask, vb, va);
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
static int test_v32hf_blend(void) {
    _Float16 a[32], b[32], result[32];
    __m512h va, vb, vresult;
    __mmask32 mask = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)(100.0f - i * 0.3f);
    }
    
    va = _mm512_loadu_ph(a);
    vb = _mm512_loadu_ph(b);
    
    /* Create mask based on comparison */
    __m512h zero = _mm512_setzero_ph();
    mask = _mm512_cmp_ph_mask(va, zero, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    vresult = _mm512_mask_blend_ph(mask, vb, va);
    _mm512_storeu_ph(result, vresult);
    
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
    __bf16 a[32], b[32], result[32];
    __m512bh va, vb, vresult;
    __mmask32 mask = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float(i * 1.5f);
        b[i] = bfloat16_from_float(200.0f - i * 2.0f);
    }
    
    va = _mm512_loadu_si512((__m512i*)a);
    vb = _mm512_loadu_si512((__m512i*)b);
    
    /* For bfloat16, we need to emulate blend using epi16 */
    __m512i vai = _mm512_castps_si512(_mm512_castsi512_ps(va));
    __m512i vbi = _mm512_castps_si512(_mm512_castsi512_ps(vb));
    
    /* Create mask - blend every other element */
    for (int i = 0; i < 32; i++) {
        if (i % 3 == 0) {
            mask |= (1U << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512i vresulti = _mm512_mask_blend_epi16(mask, vbi, vai);
    _mm512_storeu_si512((__m512i*)result, vresulti);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(result[i]);
    }
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
static int test_v16si_blend(void) {
    int a[16], b[16], result[16];
    __m512i va, vb, vresult;
    __mmask16 mask = 0;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 100;
        b[i] = 5000 - i * 200;
    }
    
    va = _mm512_loadu_si512((__m512i*)a);
    vb = _mm512_loadu_si512((__m512i*)b);
    
    /* Generate mask using comparison */
    __m512i threshold = _mm512_set1_epi32(800);
    mask = _mm512_cmpgt_epi32_mask(va, threshold);
    
    /* This should trigger gen_avx512f_blendmv16si */
    vresult = _mm512_mask_blend_epi32(mask, vb, va);
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
static long long test_v8di_blend(void) {
    long long a[8], b[8], result[8];
    __m512i va, vb, vresult;
    __mmask8 mask = 0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = 1000000LL * i;
        b[i] = 10000000LL - 500000LL * i;
    }
    
    va = _mm512_loadu_si512((__m512i*)a);
    vb = _mm512_loadu_si512((__m512i*)b);
    
    /* Create mask: select a[i] if a[i] > b[i] */
    __mmask8 cmp_mask = _mm512_cmpgt_epi64_mask(va, vb);
    mask = cmp_mask;
    
    /* This should trigger gen_avx512f_blendmv8di */
    vresult = _mm512_mask_blend_epi64(mask, vb, va);
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V8DFmode (8-double precision floats) ==================== */
static double test_v8df_blend(void) {
    double a[8], b[8], result[8];
    __m512d va, vb, vresult;
    __mmask8 mask = 0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = 20.0 - i * 0.7;
    }
    
    va = _mm512_loadu_pd(a);
    vb = _mm512_loadu_pd(b);
    
    /* Generate mask: select a[i] if a[i] > 5.0 */
    __m512d threshold = _mm512_set1_pd(5.0);
    mask = _mm512_cmp_pd_mask(va, threshold, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    vresult = _mm512_mask_blend_pd(mask, vb, va);
    _mm512_storeu_pd(result, vresult);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V16SFmode (16-single precision floats) ==================== */
static float test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    __m512 va, vb, vresult;
    __mmask16 mask = 0;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.25f;
        b[i] = 10.0f - i * 0.15f;
    }
    
    va = _mm512_loadu_ps(a);
    vb = _mm512_loadu_ps(b);
    
    /* Generate mask using comparison */
    __m512 zero = _mm512_setzero_ps();
    mask = _mm512_cmp_ps_mask(va, zero, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    vresult = _mm512_mask_blend_ps(mask, vb, va);
    _mm512_storeu_ps(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
static int test_mixed_blend_loop(void) {
    const int N = 1024;
    int total_sum = 0;
    
    /* Test different data types in a loop */
    for (int iter = 0; iter < 10; iter++) {
        /* Float blend */
        {
            float fa[16], fb[16], fr[16];
            for (int i = 0; i < 16; i++) {
                fa[i] = (iter + i) * 0.1f;
                fb[i] = 5.0f - (iter + i) * 0.05f;
            }
            __m512 fva = _mm512_loadu_ps(fa);
            __m512 fvb = _mm512_loadu_ps(fb);
            __mmask16 fmask = _mm512_cmp_ps_mask(fva, fvb, _CMP_GT_OQ);
            __m512 fvr = _mm512_mask_blend_ps(fmask, fvb, fva);
            _mm512_storeu_ps(fr, fvr);
            
            for (int i = 0; i < 16; i++) {
                total_sum += (int)fr[i];
            }
        }
        
        /* Double blend */
        {
            double da[8], db[8], dr[8];
            for (int i = 0; i < 8; i++) {
                da[i] = (iter + i) * 0.2;
                db[i] = 10.0 - (iter + i) * 0.1;
            }
            __m512d dva = _mm512_loadu_pd(da);
            __m512d dvb = _mm512_loadu_pd(db);
            __mmask8 dmask = _mm512_cmp_pd_mask(dva, dvb, _CMP_GT_OQ);
            __m512d dvr = _mm512_mask_blend_pd(dmask, dvb, dva);
            _mm512_storeu_pd(dr, dvr);
            
            for (int i = 0; i < 8; i++) {
                total_sum += (int)dr[i];
            }
        }
        
        /* 32-bit integer blend */
        {
            int ia[16], ib[16], ir[16];
            for (int i = 0; i < 16; i++) {
                ia[i] = iter * 100 + i * 10;
                ib[i] = 5000 - iter * 50 - i * 5;
            }
            __m512i iva = _mm512_loadu_si512((__m512i*)ia);
            __m512i ivb = _mm512_loadu_si512((__m512i*)ib);
            __mmask16 imask = _mm512_cmpgt_epi32_mask(iva, ivb);
            __m512i ivr = _mm512_mask_blend_epi32(imask, ivb, iva);
            _mm512_storeu_si512((__m512i*)ir, ivr);
            
            for (int i = 0; i < 16; i++) {
                total_sum += ir[i];
            }
        }
    }
    
    return total_sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ==================== Scalar Fallback Implementations ==================== */
static int scalar_test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = i;
        b[i] = 64 - i;
        result[i] = (i % 2 == 0) ? a[i] : b[i];
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
        a[i] = i * 2;
        b[i] = 1000 - i * 3;
        result[i] = (a[i] > 0) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

static int scalar_test_v16si_blend(void) {
    int a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 100;
        b[i] = 5000 - i * 200;
        result[i] = (a[i] > 800) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

static long long scalar_test_v8di_blend(void) {
    long long a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = 1000000LL * i;
        b[i] = 10000000LL - 500000LL * i;
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

static double scalar_test_v8df_blend(void) {
    double a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = 20.0 - i * 0.7;
        result[i] = (a[i] > 5.0) ? a[i] : b[i];
    }
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

static float scalar_test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.25f;
        b[i] = 10.0f - i * 0.15f;
        result[i] = (a[i] > 0.0f) ? a[i] : b[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

static int scalar_test_mixed_blend_loop(void) {
    int total_sum = 0;
    
    for (int iter = 0; iter < 10; iter++) {
        /* Float blend */
        float fa[16], fb[16], fr[16];
        for (int i = 0; i < 16; i++) {
            fa[i] = (iter + i) * 0.1f;
            fb[i] = 5.0f - (iter + i) * 0.05f;
            fr[i] = (fa[i] > fb[i]) ? fa[i] : fb[i];
            total_sum += (int)fr[i];
        }
        
        /* Double blend */
        double da[8], db[8], dr[8];
        for (int i = 0; i < 8; i++) {
            da[i] = (iter + i) * 0.2;
            db[i] = 10.0 - (iter + i) * 0.1;
            dr[i] = (da[i] > db[i]) ? da[i] : db[i];
            total_sum += (int)dr[i];
        }
        
        /* 32-bit integer blend */
        int ia[16], ib[16], ir[16];
        for (int i = 0; i < 16; i++) {
            ia[i] = iter * 100 + i * 10;
            ib[i] = 5000 - iter * 50 - i * 5;
            ir[i] = (ia[i] > ib[i]) ? ia[i] : ib[i];
            total_sum += ir[i];
        }
    }
    
    return total_sum;
}

/* ==================== Main Function ==================== */
int main(void) {
    int total_result = 0;
    
    printf("Testing AVX-512 Blend Operations...\n");
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512F and AVX-512BW detected. Using vectorized implementations.\n");
    
    /* Test all vector modes */
    total_result += test_v64qi_blend();
    printf("V64QImode test completed.\n");
    
    total_result += test_v32hi_blend();
    printf("V32HImode test completed.\n");
    
#ifdef __AVX512FP16__
    total_result += test_v32hf_blend();
    printf("V32HFmode test completed.\n");
#endif
    
#ifdef __AVX512BF16__
    total_result += test_v32bf_blend();
    printf("V32BFmode test completed.\n");
#endif
    
    total_result += test_v16si_blend();
    printf("V16SImode test completed.\n");
    
    total_result += (int)test_v8di_blend();
    printf("V8DImode test completed.\n");
    
    total_result += (int)test_v8df_blend();
    printf("V8DFmode test completed.\n");
    
    total_result += (int)test_v16sf_blend();
    printf("V16SFmode test completed.\n");
    
    total_result += test_mixed_blend_loop();
    printf("Mixed data type loop test completed.\n");
    
#else
    printf("AVX-512BW not detected. Using scalar fallbacks.\n");
    goto scalar_fallback;
#endif
#else
    printf("AVX-512F not detected. Using scalar fallbacks.\n");
    goto scalar_fallback;
#endif

    printf("All vector tests completed. Total checksum: %d\n", total_result);
    return total_result & 0xFF; /* Return lower byte to avoid large exit codes */

scalar_fallback:
    printf("Running scalar fallback implementations...\n");
    
    total_result += scalar_test_v64qi_blend();
    total_result += scalar_test_v32hi_blend();
    total_result += scalar_test_v16si_blend();
    total_result += (int)scalar_test_v8di_blend();
    total_result += (int)scalar_test_v8df_blend();
    total_result += (int)scalar_test_v16sf_blend();
    total_result += scalar_test_mixed_blend_loop();
    
    printf("All scalar tests completed. Total checksum: %d\n", total_result);
    return total_result & 0xFF;
}
