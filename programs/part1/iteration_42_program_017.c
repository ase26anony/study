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

#ifdef __AVX512FP16__
/* V32HFmode: 32-half-precision floats */
static float test_v32hf_blend(void) {
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    /* Generate mask */
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    /* Reduce sum */
    __m512h sum_vec = _mm512_reduce_add_ph(result);
    float sum = (float)sum_vec[0];
    
    return sum;
}
#endif

/* V32BFmode: 32-bfloat16 (emulated with epi16) */
static float test_v32bf_blend(void) {
    uint16_t a[32] __attribute__((aligned(64)));
    uint16_t b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        /* Create simple bfloat16 patterns */
        a[i] = (uint16_t)(i << 8);
        b[i] = (uint16_t)((31 - i) << 8);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate mask based on high byte comparison */
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_NE);
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    /* Compute checksum */
    uint64_t sum = 0;
    uint16_t res_arr[32];
    _mm512_store_si512((__m512i*)res_arr, result);
    for (int i = 0; i < 32; i++) {
        sum += res_arr[i];
    }
    
    return (float)sum;
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
    
    /* Generate mask */
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    /* Horizontal sum */
    __m512i sum_vec = _mm512_reduce_add_epi32(result);
    uint64_t sum = _mm512_reduce_add_epi64(sum_vec);
    
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
    
    /* Generate mask */
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_LE);
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    /* Horizontal sum */
    __m512i sum_vec = _mm512_reduce_add_epi64(result);
    uint64_t sum = _mm512_extract_epi64(sum_vec, 0);
    
    return sum;
}

/* V8DFmode: 8-double-precision floats */
static double test_v8df_blend(void) {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.1;
        b[i] = i * 2.2;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    /* Generate mask using comparison */
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, va, vb);
    
    /* Horizontal sum */
    __m512d sum_vec = _mm512_reduce_add_pd(result);
    double sum = _mm512_reduce_add_pd(sum_vec);
    
    return sum;
}

/* V16SFmode: 16-single-precision floats */
static float test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 0.75f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    /* Generate mask */
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    /* Horizontal sum */
    __m512 sum_vec = _mm512_reduce_add_ps(result);
    float sum = _mm512_reduce_add_ps(sum_vec);
    
    return sum;
}

/* Mixed data types in loop structure */
static uint64_t test_mixed_blends(void) {
    const int N = 1024;
    uint64_t total_sum = 0;
    
    /* Process different data types in separate loops */
    
    /* V64QImode in loop */
    {
        char arr1[N] __attribute__((aligned(64)));
        char arr2[N] __attribute__((aligned(64)));
        
        for (int i = 0; i < N; i++) {
            arr1[i] = (char)(i % 256);
            arr2[i] = (char)((i + 128) % 256);
        }
        
        for (int i = 0; i < N; i += 64) {
            __m512i v1 = _mm512_load_si512((__m512i*)(arr1 + i));
            __m512i v2 = _mm512_load_si512((__m512i*)(arr2 + i));
            __mmask64 mask = _mm512_cmp_epi8_mask(v1, v2, _MM_CMPINT_GT);
            __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
            
            __m512i sum_vec = _mm512_sad_epu8(result, _mm512_setzero_si512());
            total_sum += _mm512_reduce_add_epi64(sum_vec);
        }
    }
    
    /* V16SFmode in loop */
    {
        float arr1[N] __attribute__((aligned(64)));
        float arr2[N] __attribute__((aligned(64)));
        
        for (int i = 0; i < N; i++) {
            arr1[i] = i * 0.1f;
            arr2[i] = i * 0.2f;
        }
        
        for (int i = 0; i < N; i += 16) {
            __m512 v1 = _mm512_load_ps(arr1 + i);
            __m512 v2 = _mm512_load_ps(arr2 + i);
            __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_LT_OQ);
            __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
            
            __m512 sum_vec = _mm512_reduce_add_ps(result);
            total_sum += (uint64_t)_mm512_reduce_add_ps(sum_vec);
        }
    }
    
    /* V8DFmode in loop */
    {
        double arr1[N] __attribute__((aligned(64)));
        double arr2[N] __attribute__((aligned(64)));
        
        for (int i = 0; i < N; i++) {
            arr1[i] = i * 0.05;
            arr2[i] = i * 0.1;
        }
        
        for (int i = 0; i < N; i += 8) {
            __m512d v1 = _mm512_load_pd(arr1 + i);
            __m512d v2 = _mm512_load_pd(arr2 + i);
            __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_GT_OQ);
            __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
            
            __m512d sum_vec = _mm512_reduce_add_pd(result);
            total_sum += (uint64_t)_mm512_reduce_add_pd(sum_vec);
        }
    }
    
    return total_sum;
}

#else /* AVX-512 BW not available */
static uint64_t test_v64qi_blend(void) { return 0; }
static uint64_t test_v32hi_blend(void) { return 0; }
static float test_v32bf_blend(void) { return 0.0f; }
#endif /* __AVX512BW__ */

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

static float test_v32bf_blend(void) {
    uint16_t a[32], b[32];
    float sum = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (uint16_t)(i << 8);
        b[i] = (uint16_t)((31 - i) << 8);
        uint16_t result = (a[i] != b[i]) ? a[i] : b[i];
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
        sum += result;
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
        sum += result;
    }
    
    return sum;
}

static double test_v8df_blend(void) {
    double a[8], b[8];
    double sum = 0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.1;
        b[i] = i * 2.2;
        double result = (a[i] > b[i]) ? a[i] : b[i];
        sum += result;
    }
    
    return sum;
}

static float test_v16sf_blend(void) {
    float a[16], b[16];
    float sum = 0;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 0.75f;
        float result = (a[i] < b[i]) ? a[i] : b[i];
        sum += result;
    }
    
    return sum;
}

static uint64_t test_mixed_blends(void) {
    return 0;
}

#endif /* __AVX512F__ */

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
#endif
    
    /* Test each vector mode */
    total_checksum += test_v64qi_blend();
    printf("V64QImode test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode test completed\n");
    
#ifdef __AVX512FP16__
    total_checksum += (uint64_t)test_v32hf_blend();
    printf("V32HFmode test completed\n");
#endif
    
    total_checksum += (uint64_t)test_v32bf_blend();
    printf("V32BFmode test completed\n");
    
    total_checksum += test_v16si_blend();
    printf("V16SImode test completed\n");
    
    total_checksum += test_v8di_blend();
    printf("V8DImode test completed\n");
    
    total_checksum += (uint64_t)test_v8df_blend();
    printf("V8DFmode test completed\n");
    
    total_checksum += (uint64_t)test_v16sf_blend();
    printf("V16SFmode test completed\n");
    
    total_checksum += test_mixed_blends();
    printf("Mixed blends test completed\n");
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return (int)(total_checksum % 256);
}
