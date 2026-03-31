#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64-byte integers */
static uint64_t test_v64qi_blend(void) {
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate dynamic mask using comparison */
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
    
    /* Use result in computation to prevent elimination */
    __m512i sum_vec = _mm512_sad_epu8(result, _mm512_setzero_si512());
    uint64_t sum = _mm512_reduce_add_epi64(sum_vec);
    
    return sum;
}

/* V32HImode: 32-halfword integers */
static uint64_t test_v32hi_blend(void) {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate dynamic mask */
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    /* Compute sum of absolute differences */
    __m512i abs_diff = _mm512_abs_epi16(result);
    __m512i sum_vec = _mm512_sad_epu16(abs_diff, _mm512_setzero_si512());
    uint64_t sum = _mm512_reduce_add_epi64(sum_vec);
    
    return sum;
}

/* V32HFmode: 32-half-precision floats */
static float test_v32hf_blend(void) {
#ifdef __AVX512FP16__
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    /* Generate mask using comparison */
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    /* Reduce sum */
    __m512h sum_vec = _mm512_reduce_add_ph(result);
    float sum = (float)sum_vec;
    
    return sum;
#else
    return 0.0f;
#endif
}

/* V32BFmode: 32-bfloat16 floats */
static float test_v32bf_blend(void) {
#ifdef __AVX512BF16__
    __bf16 a[32] __attribute__((aligned(64)));
    __bf16 b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (__bf16)(i * 1.2f);
        b[i] = (__bf16)(i * 2.2f);
    }
    
    /* Load as integers since bfloat16 uses same storage as epi16 */
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate mask using integer comparison */
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_NE);
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    /* Convert to float and sum */
    __m512 bf_vec = _mm512_cvtneobf16_ps(result);
    float sum = _mm512_reduce_add_ps(bf_vec);
    
    return sum;
#else
    return 0.0f;
#endif
}

/* V16SImode: 16-dword integers */
static uint64_t test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 13;
        b[i] = i * 17;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate dynamic mask */
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    /* Horizontal sum */
    __m512i sum_vec = _mm512_add_epi32(result, _mm512_srli_epi32(result, 16));
    sum_vec = _mm512_add_epi32(sum_vec, _mm512_srli_epi32(sum_vec, 8));
    uint64_t sum = _mm512_reduce_add_epi32(sum_vec);
    
    return sum;
}

/* V8DImode: 8-qword integers */
static uint64_t test_v8di_blend(void) {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 23LL;
        b[i] = i * 29LL;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate dynamic mask */
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_LE);
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    /* Horizontal sum */
    __m512i sum_vec = _mm512_add_epi64(result, _mm512_srli_epi64(result, 32));
    uint64_t sum = _mm512_reduce_add_epi64(sum_vec);
    
    return sum;
}

/* V8DFmode: 8-double-precision floats */
static double test_v8df_blend(void) {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.7;
        b[i] = i * 2.9;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    /* Generate dynamic mask */
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, va, vb);
    
    /* Horizontal sum */
    __m512d sum_vec = _mm512_add_pd(result, _mm512_permute_pd(result, 0b0101));
    sum_vec = _mm512_add_pd(sum_vec, _mm512_permute_pd(sum_vec, 0b0011));
    double sum = _mm512_reduce_add_pd(sum_vec);
    
    return sum;
}

/* V16SFmode: 16-single-precision floats */
static float test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1.3f;
        b[i] = i * 2.1f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    /* Generate dynamic mask */
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GE_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    /* Horizontal sum */
    __m512 sum_vec = _mm512_add_ps(result, _mm512_permute_ps(result, 0b0101));
    sum_vec = _mm512_add_ps(sum_vec, _mm512_permute_ps(sum_vec, 0b1010));
    sum_vec = _mm512_add_ps(sum_vec, _mm512_permute_ps(sum_vec, 0b1111));
    float sum = _mm512_reduce_add_ps(sum_vec);
    
    return sum;
}

/* Mixed data types in loop structure */
static uint64_t test_mixed_blends(void) {
    const int N = 1024;
    uint64_t total_sum = 0;
    
    /* Test different data types in loops */
    for (int iter = 0; iter < 4; iter++) {
        /* V16SFmode in loop */
        float fa[16] __attribute__((aligned(64)));
        float fb[16] __attribute__((aligned(64)));
        for (int i = 0; i < 16; i++) {
            fa[i] = (iter * 16 + i) * 0.7f;
            fb[i] = (iter * 16 + i) * 1.3f;
        }
        __m512 fva = _mm512_load_ps(fa);
        __m512 fvb = _mm512_load_ps(fb);
        __mmask16 fmask = _mm512_cmp_ps_mask(fva, fvb, _CMP_NLE_UQ);
        __m512 fresult = _mm512_mask_blend_ps(fmask, fva, fvb);
        total_sum += (uint64_t)_mm512_reduce_add_ps(fresult);
        
        /* V8DFmode in loop */
        double da[8] __attribute__((aligned(64)));
        double db[8] __attribute__((aligned(64)));
        for (int i = 0; i < 8; i++) {
            da[i] = (iter * 8 + i) * 0.9;
            db[i] = (iter * 8 + i) * 1.7;
        }
        __m512d dva = _mm512_load_pd(da);
        __m512d dvb = _mm512_load_pd(db);
        __mmask8 dmask = _mm512_cmp_pd_mask(dva, dvb, _CMP_NGE_UQ);
        __m512d dresult = _mm512_mask_blend_pd(dmask, dva, dvb);
        total_sum += (uint64_t)_mm512_reduce_add_pd(dresult);
        
        /* V16SImode in loop */
        int ia[16] __attribute__((aligned(64)));
        int ib[16] __attribute__((aligned(64)));
        for (int i = 0; i < 16; i++) {
            ia[i] = iter * 16 + i;
            ib[i] = iter * 16 + i * 2;
        }
        __m512i iva = _mm512_load_si512((__m512i*)ia);
        __m512i ivb = _mm512_load_si512((__m512i*)ib);
        __mmask16 imask = _mm512_cmp_epi32_mask(iva, ivb, _MM_CMPINT_GT);
        __m512i iresult = _mm512_mask_blend_epi32(imask, iva, ivb);
        total_sum += _mm512_reduce_add_epi32(iresult);
    }
    
    return total_sum;
}

#else /* AVX-512 BW not available */
static uint64_t test_v64qi_blend(void) { return 0; }
static uint64_t test_v32hi_blend(void) { return 0; }
static float test_v32hf_blend(void) { return 0.0f; }
static float test_v32bf_blend(void) { return 0.0f; }
static uint64_t test_v16si_blend(void) { return 0; }
static uint64_t test_v8di_blend(void) { return 0; }
static double test_v8df_blend(void) { return 0.0; }
static float test_v16sf_blend(void) { return 0.0f; }
static uint64_t test_mixed_blends(void) { return 0; }
#endif /* AVX512BW */

#else /* AVX-512F not available */

/* Scalar fallback implementations */
static uint64_t test_v64qi_blend(void) {
    char a[64], b[64];
    uint64_t sum = 0;
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
        char result = (a[i] > b[i]) ? a[i] : b[i];
        sum += (uint8_t)result;
    }
    return sum;
}

static uint64_t test_v32hi_blend(void) {
    short a[32], b[32];
    uint64_t sum = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
        short result = (a[i] < b[i]) ? a[i] : b[i];
        sum += (uint16_t)result;
    }
    return sum;
}

static float test_v32hf_blend(void) {
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        float a = i * 1.5f;
        float b = i * 2.5f;
        float result = (a > b) ? a : b;
        sum += result;
    }
    return sum;
}

static float test_v32bf_blend(void) {
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        float a = i * 1.2f;
        float b = i * 2.2f;
        float result = (a != b) ? a : b;
        sum += result;
    }
    return sum;
}

static uint64_t test_v16si_blend(void) {
    int a[16], b[16];
    uint64_t sum = 0;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 13;
        b[i] = i * 17;
        int result = (a[i] == b[i]) ? a[i] : b[i];
        sum += (uint32_t)result;
    }
    return sum;
}

static uint64_t test_v8di_blend(void) {
    long long a[8], b[8];
    uint64_t sum = 0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 23LL;
        b[i] = i * 29LL;
        long long result = (a[i] <= b[i]) ? a[i] : b[i];
        sum += (uint64_t)result;
    }
    return sum;
}

static double test_v8df_blend(void) {
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        double a = i * 1.7;
        double b = i * 2.9;
        double result = (a < b) ? a : b;
        sum += result;
    }
    return sum;
}

static float test_v16sf_blend(void) {
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        float a = i * 1.3f;
        float b = i * 2.1f;
        float result = (a >= b) ? a : b;
        sum += result;
    }
    return sum;
}

static uint64_t test_mixed_blends(void) {
    uint64_t total_sum = 0;
    
    for (int iter = 0; iter < 4; iter++) {
        /* V16SFmode scalar */
        for (int i = 0; i < 16; i++) {
            float a = (iter * 16 + i) * 0.7f;
            float b = (iter * 16 + i) * 1.3f;
            float result = (!(a <= b)) ? a : b;
            total_sum += (uint64_t)result;
        }
        
        /* V8DFmode scalar */
        for (int i = 0; i < 8; i++) {
            double a = (iter * 8 + i) * 0.9;
            double b = (iter * 8 + i) * 1.7;
            double result = (!(a >= b)) ? a : b;
            total_sum += (uint64_t)result;
        }
        
        /* V16SImode scalar */
        for (int i = 0; i < 16; i++) {
            int a = iter * 16 + i;
            int b = iter * 16 + i * 2;
            int result = (a > b) ? a : b;
            total_sum += (uint32_t)result;
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
    
    /* Test all vector modes */
    total_checksum += test_v64qi_blend();
    total_checksum += test_v32hi_blend();
    total_checksum += (uint64_t)test_v32hf_blend();
    total_checksum += (uint64_t)test_v32bf_blend();
    total_checksum += test_v16si_blend();
    total_checksum += test_v8di_blend();
    total_checksum += (uint64_t)test_v8df_blend();
    total_checksum += (uint64_t)test_v16sf_blend();
    total_checksum += test_mixed_blends();
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return (int)(total_checksum % 256);
}
