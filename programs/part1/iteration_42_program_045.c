#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
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
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv64qi
    __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
    
    // Use result in computation to prevent elimination
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
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate dynamic mask
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    // Compute sum of absolute differences
    __m512i abs_diff = _mm512_abs_epi16(result);
    __m512i sum_vec = _mm512_madd_epi16(abs_diff, _mm512_set1_epi16(1));
    uint64_t sum = _mm512_reduce_add_epi32(sum_vec);
    
    return sum;
}

/* V32HFmode: 32-half-precision floats */
#ifdef __AVX512FP16__
static float test_v32hf_blend(void) {
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h va = _mm512_load_ph((const void*)a);
    __m512h vb = _mm512_load_ph((const void*)b);
    
    // Generate mask
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    // Compute sum
    __m512h sum_vec = _mm512_add_ph(result, _mm512_setzero_ph());
    float sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (_Float16)result[i];
    }
    
    return sum;
}
#endif

/* V32BFmode: 32-bfloat16 floats */
#ifdef __AVX512BF16__
static float test_v32bf_blend(void) {
    __bfloat16 a[32] __attribute__((aligned(64)));
    __bfloat16 b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float(i * 1.5f);
        b[i] = bfloat16_from_float(i * 2.5f);
    }
    
    __m512bh va = _mm512_load_si512((const void*)a);
    __m512bh vb = _mm512_load_si512((const void*)b);
    
    // Generate mask (using integer comparison since bfloat16 uses epi16)
    __mmask32 mask = _mm512_cmp_epi16_mask((__m512i)va, (__m512i)vb, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv32bf
    __m512bh result = (__m512bh)_mm512_mask_blend_epi16(mask, (__m512i)va, (__m512i)vb);
    
    // Convert to float and compute sum
    float sum = 0;
    __bfloat16* res_ptr = (__bfloat16*)&result;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(res_ptr[i]);
    }
    
    return sum;
}
#endif

/* V16SImode: 16-dword integers */
static uint64_t test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 13;
        b[i] = i * 17;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate dynamic mask
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    // Compute horizontal sum
    uint64_t sum = _mm512_reduce_add_epi32(result);
    
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
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate dynamic mask
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_NEQ);
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    // Compute horizontal sum
    uint64_t sum = _mm512_reduce_add_epi64(result);
    
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
    
    // Generate dynamic mask
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d result = _mm512_mask_blend_pd(mask, va, vb);
    
    // Compute horizontal sum
    double sum = _mm512_reduce_add_pd(result);
    
    return sum;
}

/* V16SFmode: 16-single-precision floats */
static float test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 1.5f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    // Generate dynamic mask
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    // Compute horizontal sum
    float sum = _mm512_reduce_add_ps(result);
    
    return sum;
}

/* Mixed data types in loop structure */
static uint64_t test_mixed_blends(void) {
    const int N = 1024;
    
    // Allocate aligned arrays
    char*    data8  = aligned_alloc(64, N * sizeof(char));
    short*   data16 = aligned_alloc(64, N * sizeof(short));
    int*     data32 = aligned_alloc(64, N * sizeof(int));
    long long* data64 = aligned_alloc(64, N * sizeof(long long));
    float*   dataf  = aligned_alloc(64, N * sizeof(float));
    double*  datad  = aligned_alloc(64, N * sizeof(double));
    
    // Initialize with pattern
    for (int i = 0; i < N; i++) {
        data8[i]  = (char)(i % 256);
        data16[i] = (short)(i * 3);
        data32[i] = i * 5;
        data64[i] = i * 7LL;
        dataf[i]  = i * 0.25f;
        datad[i]  = i * 0.125;
    }
    
    uint64_t total_sum = 0;
    
    // Process in chunks of vector size
    for (int i = 0; i < N; i += 64) {
        // V64QImode blend
        __m512i va8 = _mm512_load_si512((const __m512i*)(data8 + i));
        __m512i vb8 = _mm512_set1_epi8(128);
        __mmask64 mask8 = _mm512_cmp_epi8_mask(va8, vb8, _MM_CMPINT_GT);
        __m512i res8 = _mm512_mask_blend_epi8(mask8, va8, vb8);
        total_sum += _mm512_reduce_add_epi64(_mm512_sad_epu8(res8, _mm512_setzero_si512()));
    }
    
    for (int i = 0; i < N; i += 32) {
        // V32HImode blend
        __m512i va16 = _mm512_load_si512((const __m512i*)(data16 + i));
        __m512i vb16 = _mm512_set1_epi16(1000);
        __mmask32 mask16 = _mm512_cmp_epi16_mask(va16, vb16, _MM_CMPINT_LT);
        __m512i res16 = _mm512_mask_blend_epi16(mask16, va16, vb16);
        total_sum += _mm512_reduce_add_epi32(_mm512_madd_epi16(res16, _mm512_set1_epi16(1)));
    }
    
    for (int i = 0; i < N; i += 16) {
        // V16SImode blend
        __m512i va32 = _mm512_load_si512((const __m512i*)(data32 + i));
        __m512i vb32 = _mm512_set1_epi32(500);
        __mmask16 mask32 = _mm512_cmp_epi32_mask(va32, vb32, _MM_CMPINT_EQ);
        __m512i res32 = _mm512_mask_blend_epi32(mask32, va32, vb32);
        total_sum += _mm512_reduce_add_epi32(res32);
    }
    
    for (int i = 0; i < N; i += 8) {
        // V8DImode blend
        __m512i va64 = _mm512_load_si512((const __m512i*)(data64 + i));
        __m512i vb64 = _mm512_set1_epi64(1000);
        __mmask8 mask64 = _mm512_cmp_epi64_mask(va64, vb64, _MM_CMPINT_NEQ);
        __m512i res64 = _mm512_mask_blend_epi64(mask64, va64, vb64);
        total_sum += _mm512_reduce_add_epi64(res64);
        
        // V8DFmode blend
        __m512d vaf = _mm512_load_pd(datad + i);
        __m512d vbf = _mm512_set1_pd(50.0);
        __mmask8 maskf = _mm512_cmp_pd_mask(vaf, vbf, _CMP_LT_OQ);
        __m512d resf = _mm512_mask_blend_pd(maskf, vaf, vbf);
        total_sum += (uint64_t)_mm512_reduce_add_pd(resf);
    }
    
    for (int i = 0; i < N; i += 16) {
        // V16SFmode blend
        __m512 vaf32 = _mm512_load_ps(dataf + i);
        __m512 vbf32 = _mm512_set1_ps(25.0f);
        __mmask16 maskf32 = _mm512_cmp_ps_mask(vaf32, vbf32, _CMP_GT_OQ);
        __m512 resf32 = _mm512_mask_blend_ps(maskf32, vaf32, vbf32);
        total_sum += (uint64_t)_mm512_reduce_add_ps(resf32);
    }
    
    free(data8);
    free(data16);
    free(data32);
    free(data64);
    free(dataf);
    free(datad);
    
    return total_sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* Scalar fallback implementations */
static uint64_t test_v64qi_blend_scalar(void) {
    char a[64], b[64];
    char result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (unsigned char)result[i];
    }
    
    return sum;
}

static uint64_t test_v32hi_blend_scalar(void) {
    short a[32], b[32];
    short result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
        result[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (unsigned short)result[i];
    }
    
    return sum;
}

static float test_v16sf_blend_scalar(void) {
    float a[16], b[16];
    float result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 1.5f;
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    
    float sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    return sum;
}

static double test_v8df_blend_scalar(void) {
    double a[8], b[8];
    double result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.1;
        b[i] = i * 2.2;
        result[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
    
    double sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    return sum;
}

static uint64_t test_v16si_blend_scalar(void) {
    int a[16], b[16];
    int result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 13;
        b[i] = i * 17;
        result[i] = (a[i] == b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (unsigned)result[i];
    }
    
    return sum;
}

static uint64_t test_v8di_blend_scalar(void) {
    long long a[8], b[8];
    long long result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 23LL;
        b[i] = i * 29LL;
        result[i] = (a[i] != b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (unsigned long long)result[i];
    }
    
    return sum;
}

int main(void) {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Using vectorized implementations.\n");
    
    total_checksum += test_v64qi_blend();
    printf("V64QImode blend checksum added\n");
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode blend checksum added\n");
    
#ifdef __AVX512FP16__
    total_checksum += (uint64_t)test_v32hf_blend();
    printf("V32HFmode blend checksum added\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += (uint64_t)test_v32bf_blend();
    printf("V32BFmode blend checksum added\n");
#endif
    
    total_checksum += test_v16si_blend();
    printf("V16SImode blend checksum added\n");
    
    total_checksum += test_v8di_blend();
    printf("V8DImode blend checksum added\n");
    
    total_checksum += (uint64_t)test_v8df_blend();
    printf("V8DFmode blend checksum added\n");
    
    total_checksum += (uint64_t)test_v16sf_blend();
    printf("V16SFmode blend checksum added\n");
    
    total_checksum += test_mixed_blends();
    printf("Mixed blends checksum added\n");
    
#else
    printf("AVX-512BW not available. Using scalar fallbacks.\n");
    goto scalar_fallback;
#endif
#else
    printf("AVX-512 not available. Using scalar fallbacks.\n");
    
scalar_fallback:
    total_checksum += test_v64qi_blend_scalar();
    printf("V64QImode scalar blend checksum added\n");
    
    total_checksum += test_v32hi_blend_scalar();
    printf("V32HImode scalar blend checksum added\n");
    
    total_checksum += (uint64_t)test_v16sf_blend_scalar();
    printf("V16SFmode scalar blend checksum added\n");
    
    total_checksum += (uint64_t)test_v8df_blend_scalar();
    printf("V8DFmode scalar blend checksum added\n");
    
    total_checksum += test_v16si_blend_scalar();
    printf("V16SImode scalar blend checksum added\n");
    
    total_checksum += test_v8di_blend_scalar();
    printf("V8DImode scalar blend checksum added\n");
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    return (int)(total_checksum % 256);
}
