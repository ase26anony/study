#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>
#include <math.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64-byte integers */
static uint64_t test_v64qi_blend(void) {
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    char result[64] __attribute__((aligned(64)));
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        a[i] = i;
        b[i] = 64 - i;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate dynamic mask using comparison */
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    /* Compute checksum to prevent optimization */
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)result[i];
    }
    return sum;
}

/* V32HImode: 32-halfword integers */
static uint64_t test_v32hi_blend(void) {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    short result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = i * 100;
        b[i] = i * 50;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate mask using bitwise pattern */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 3 == 0) mask |= (1ULL << i);
    }
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)result[i];
    }
    return sum;
}

/* V32HFmode: 32-half-precision floats */
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) {
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    _Float16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    /* Generate mask using comparison */
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph(result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)*(uint16_t*)&result[i];
    }
    return sum;
}
#endif

/* V32BFmode: 32-bfloat16 floats */
#ifdef __AVX512BF16__
static uint64_t test_v32bf_blend(void) {
    __bf16 a[32] __attribute__((aligned(64)));
    __bf16 b[32] __attribute__((aligned(64)));
    __bf16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float(i * 1.1f);
        b[i] = bfloat16_from_float(i * 2.2f);
    }
    
    /* For bfloat16, we need to use integer blend since there's no direct bfloat16 blend */
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)*(uint16_t*)&result[i];
    }
    return sum;
}
#endif

/* V16SImode: 16-singleword integers */
static uint64_t test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    int result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 2000;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_LT);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)result[i];
    }
    return sum;
}

/* V8DImode: 8-doubleword integers */
static uint64_t test_v8di_blend(void) {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    long long result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = i * 5000LL;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_EQ);
    mask = ~mask; /* Invert for variety */
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)result[i];
    }
    return sum;
}

/* V8DFmode: 8-double-precision floats */
static uint64_t test_v8df_blend(void) {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.111;
        b[i] = i * 2.222;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LE_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_store_pd(result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)(result[i] * 1000.0);
    }
    return sum;
}

/* V16SFmode: 16-single-precision floats */
static uint64_t test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    float result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1.1f;
        b[i] = i * 2.2f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_NEQ_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_store_ps(result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)(result[i] * 100.0f);
    }
    return sum;
}

/* Mixed data types in loop structure */
static uint64_t test_mixed_blend_loop(int iterations) {
    uint64_t total_sum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* V16SFmode blend in loop */
        {
            float fa[16], fb[16], fr[16];
            for (int i = 0; i < 16; i++) {
                fa[i] = (iter + i) * 0.5f;
                fb[i] = (iter - i) * 0.5f;
            }
            __m512 fva = _mm512_loadu_ps(fa);
            __m512 fvb = _mm512_loadu_ps(fb);
            __mmask16 fmask = _mm512_cmp_ps_mask(fva, fvb, _CMP_GT_OQ);
            __m512 fresult = _mm512_mask_blend_ps(fmask, fva, fvb);
            _mm512_storeu_ps(fr, fresult);
            
            for (int i = 0; i < 16; i++) {
                total_sum += (uint32_t)fr[i];
            }
        }
        
        /* V8DFmode blend in loop */
        {
            double da[8], db[8], dr[8];
            for (int i = 0; i < 8; i++) {
                da[i] = (iter + i) * 0.25;
                db[i] = (iter - i) * 0.25;
            }
            __m512d dva = _mm512_loadu_pd(da);
            __m512d dvb = _mm512_loadu_pd(db);
            __mmask8 dmask = _mm512_cmp_pd_mask(dva, dvb, _CMP_LT_OQ);
            __m512d dresult = _mm512_mask_blend_pd(dmask, dva, dvb);
            _mm512_storeu_pd(dr, dresult);
            
            for (int i = 0; i < 8; i++) {
                total_sum += (uint64_t)(dr[i] * 100.0);
            }
        }
        
        /* V16SImode blend in loop */
        {
            int ia[16], ib[16], ir[16];
            for (int i = 0; i < 16; i++) {
                ia[i] = iter * 100 + i;
                ib[i] = iter * 200 - i;
            }
            __m512i iva = _mm512_loadu_si512((__m512i*)ia);
            __m512i ivb = _mm512_loadu_si512((__m512i*)ib);
            __mmask16 imask = _mm512_cmp_epi32_mask(iva, ivb, _MM_CMPINT_NE);
            __m512i iresult = _mm512_mask_blend_epi32(imask, iva, ivb);
            _mm512_storeu_si512((__m512i*)ir, iresult);
            
            for (int i = 0; i < 16; i++) {
                total_sum += ir[i];
            }
        }
    }
    
    return total_sum;
}

#else /* AVX512BW not available */
static uint64_t test_v64qi_blend(void) { return 0; }
static uint64_t test_v32hi_blend(void) { return 0; }
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) { return 0; }
#endif
#ifdef __AVX512BF16__
static uint64_t test_v32bf_blend(void) { return 0; }
#endif
static uint64_t test_v16si_blend(void) { return 0; }
static uint64_t test_v8di_blend(void) { return 0; }
static uint64_t test_v8df_blend(void) { return 0; }
static uint64_t test_v16sf_blend(void) { return 0; }
static uint64_t test_mixed_blend_loop(int iterations) { return 0; }
#endif /* AVX512BW */

#else /* AVX512F not available */

/* Scalar fallback implementations */
static uint64_t test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    uint64_t sum = 0;
    
    for (int i = 0; i < 64; i++) {
        a[i] = i;
        b[i] = 64 - i;
        result[i] = (i % 2) ? a[i] : b[i];
        sum += (uint8_t)result[i];
    }
    return sum;
}

static uint64_t test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    uint64_t sum = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = i * 100;
        b[i] = i * 50;
        result[i] = (i % 3 == 0) ? a[i] : b[i];
        sum += (uint16_t)result[i];
    }
    return sum;
}

static uint64_t test_v16si_blend(void) {
    int a[16], b[16], result[16];
    uint64_t sum = 0;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 2000;
        result[i] = (a[i] < b[i]) ? a[i] : b[i];
        sum += (uint32_t)result[i];
    }
    return sum;
}

static uint64_t test_v8di_blend(void) {
    long long a[8], b[8], result[8];
    uint64_t sum = 0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = i * 5000LL;
        result[i] = (a[i] == b[i]) ? b[i] : a[i];
        sum += (uint64_t)result[i];
    }
    return sum;
}

static uint64_t test_v8df_blend(void) {
    double a[8], b[8], result[8];
    uint64_t sum = 0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.111;
        b[i] = i * 2.222;
        result[i] = (a[i] <= b[i]) ? a[i] : b[i];
        sum += (uint64_t)(result[i] * 1000.0);
    }
    return sum;
}

static uint64_t test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    uint64_t sum = 0;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1.1f;
        b[i] = i * 2.2f;
        result[i] = (a[i] != b[i]) ? a[i] : b[i];
        sum += (uint32_t)(result[i] * 100.0f);
    }
    return sum;
}

static uint64_t test_mixed_blend_loop(int iterations) {
    uint64_t total_sum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Float blend simulation */
        for (int i = 0; i < 16; i++) {
            float fa = (iter + i) * 0.5f;
            float fb = (iter - i) * 0.5f;
            float fr = (fa > fb) ? fa : fb;
            total_sum += (uint32_t)fr;
        }
        
        /* Double blend simulation */
        for (int i = 0; i < 8; i++) {
            double da = (iter + i) * 0.25;
            double db = (iter - i) * 0.25;
            double dr = (da < db) ? da : db;
            total_sum += (uint64_t)(dr * 100.0);
        }
        
        /* Integer blend simulation */
        for (int i = 0; i < 16; i++) {
            int ia = iter * 100 + i;
            int ib = iter * 200 - i;
            int ir = (ia != ib) ? ia : ib;
            total_sum += ir;
        }
    }
    
    return total_sum;
}

#endif /* AVX512F */

int main(void) {
    uint64_t total_checksum = 0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512F__
    printf("AVX-512F supported\n");
#ifdef __AVX512BW__
    printf("AVX-512BW supported\n");
#endif
#ifdef __AVX512FP16__
    printf("AVX-512-FP16 supported\n");
#endif
#ifdef __AVX512BF16__
    printf("AVX-512-BF16 supported\n");
#endif
#else
    printf("Using scalar fallback implementations\n");
#endif
    
    /* Test each vector mode */
    total_checksum += test_v64qi_blend();
    printf("V64QImode test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode test completed\n");
    
#ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend();
    printf("V32HFmode test completed\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += test_v32bf_blend();
    printf("V32BFmode test completed\n");
#endif
    
    total_checksum += test_v16si_blend();
    printf("V16SImode test completed\n");
    
    total_checksum += test_v8di_blend();
    printf("V8DImode test completed\n");
    
    total_checksum += test_v8df_blend();
    printf("V8DFmode test completed\n");
    
    total_checksum += test_v16sf_blend();
    printf("V16SFmode test completed\n");
    
    /* Test mixed modes in loop */
    total_checksum += test_mixed_blend_loop(10);
    printf("Mixed mode loop test completed\n");
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return (int)(total_checksum % 256);
}
