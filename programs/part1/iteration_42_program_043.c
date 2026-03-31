#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ========== V64QImode: 64-byte integers ========== */
static int test_v64qi_blend(void) {
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    char result[64] __attribute__((aligned(64)));
    
    // Initialize with pattern data
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 2);
        b[i] = (char)(i * 3 + 1);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmpgt_epi8_mask(va, _mm512_set1_epi8(32));
    
    // This should trigger gen_avx512bw_blendmv64qi
    __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    // Compute checksum to prevent optimization
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V32HImode: 32-halfword integers ========== */
static int test_v32hi_blend(void) {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    short result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 10);
        b[i] = (short)(i * 15 + 5);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask: select from b where a > 160
    __mmask32 mask = _mm512_cmpgt_epi16_mask(va, _mm512_set1_epi16(160));
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V32HFmode: 32-half-precision floats ========== */
#ifdef __AVX512FP16__
static float test_v32hf_blend(void) {
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    _Float16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)(i * 0.75f + 0.25f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    // Generate mask: select from b where a > 8.0
    __mmask32 mask = _mm512_cmp_ph_mask(va, _mm512_set1_ph(8.0f), _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    return sum;
}
#endif

/* ========== V32BFmode: 32-bfloat16 ========== */
#ifdef __AVX512BF16__
static float test_v32bf_blend(void) {
    __bf16 a[32] __attribute__((aligned(64)));
    __bf16 b[32] __attribute__((aligned(64)));
    __bf16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        // Simple pattern for bfloat16
        a[i] = (__bf16)(i & 0xFF);
        b[i] = (__bf16)((i + 128) & 0xFF);
    }
    
    // Load as epi16 for bfloat16 emulation
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask based on comparison
    __mmask32 mask = _mm512_cmpgt_epi16_mask(va, _mm512_set1_epi16(16));
    
    // This should trigger gen_avx512bw_blendmv32bf
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    return sum;
}
#endif

/* ========== V16SImode: 16-dword integers ========== */
static long test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    int result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 100;
        b[i] = i * 150 + 50;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask: select from b where a > 800
    __mmask16 mask = _mm512_cmpgt_epi32_mask(va, _mm512_set1_epi32(800));
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    long sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V8DImode: 8-qword integers ========== */
static long long test_v8di_blend(void) {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    long long result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1000LL;
        b[i] = i * 1500LL + 500LL;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask: select from b where a > 4000
    __mmask8 mask = _mm512_cmpgt_epi64_mask(va, _mm512_set1_epi64(4000));
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V8DFmode: 8-double-precision floats ========== */
static double test_v8df_blend(void) {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.25 + 0.75;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    // Generate mask: select from b where a > 6.0
    __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(6.0), _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_store_pd(result, vresult);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V16SFmode: 16-single-precision floats ========== */
static float test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    float result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.75f;
        b[i] = i * 1.125f + 0.375f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    // Generate mask: select from b where a > 6.0
    __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(6.0f), _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_store_ps(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== Mixed data types in loop ========== */
static long test_mixed_blends(void) {
    const int N = 1024;
    char* char_data = aligned_alloc(64, N * sizeof(char));
    short* short_data = aligned_alloc(64, N * sizeof(short));
    int* int_data = aligned_alloc(64, N * sizeof(int));
    float* float_data = aligned_alloc(64, N * sizeof(float));
    double* double_data = aligned_alloc(64, N * sizeof(double));
    
    // Initialize with pattern
    for (int i = 0; i < N; i++) {
        char_data[i] = (char)(i % 256);
        short_data[i] = (short)(i % 32768);
        int_data[i] = i;
        float_data[i] = i * 0.1f;
        double_data[i] = i * 0.01;
    }
    
    long total_sum = 0;
    
    // Process in AVX-512 sized chunks
    for (int i = 0; i < N; i += 64) {
        // V64QImode blend
        __m512i vchar_a = _mm512_load_si512((__m512i*)(char_data + i));
        __m512i vchar_b = _mm512_load_si512((__m512i*)(char_data + N/2 + i));
        __mmask64 char_mask = _mm512_cmpgt_epi8_mask(vchar_a, _mm512_set1_epi8(128));
        __m512i vchar_result = _mm512_mask_blend_epi8(char_mask, vchar_a, vchar_b);
        _mm512_store_si512((__m512i*)(char_data + i), vchar_result);
    }
    
    for (int i = 0; i < N; i += 32) {
        // V32HImode blend
        __m512i vshort_a = _mm512_load_si512((__m512i*)(short_data + i));
        __m512i vshort_b = _mm512_load_si512((__m512i*)(short_data + N/2 + i));
        __mmask32 short_mask = _mm512_cmpgt_epi16_mask(vshort_a, _mm512_set1_epi16(16384));
        __m512i vshort_result = _mm512_mask_blend_epi16(short_mask, vshort_a, vshort_b);
        _mm512_store_si512((__m512i*)(short_data + i), vshort_result);
    }
    
    for (int i = 0; i < N; i += 16) {
        // V16SImode blend
        __m512i vint_a = _mm512_load_si512((__m512i*)(int_data + i));
        __m512i vint_b = _mm512_load_si512((__m512i*)(int_data + N/2 + i));
        __mmask16 int_mask = _mm512_cmpgt_epi32_mask(vint_a, _mm512_set1_epi32(N/4));
        __m512i vint_result = _mm512_mask_blend_epi32(int_mask, vint_a, vint_b);
        _mm512_store_si512((__m512i*)(int_data + i), vint_result);
        
        // V16SFmode blend
        __m512 vfloat_a = _mm512_load_ps(float_data + i);
        __m512 vfloat_b = _mm512_load_ps(float_data + N/2 + i);
        __mmask16 float_mask = _mm512_cmp_ps_mask(vfloat_a, _mm512_set1_ps(N/4 * 0.1f), _CMP_GT_OQ);
        __m512 vfloat_result = _mm512_mask_blend_ps(float_mask, vfloat_a, vfloat_b);
        _mm512_store_ps(float_data + i, vfloat_result);
    }
    
    for (int i = 0; i < N; i += 8) {
        // V8DImode blend (using int64_t)
        __m512i vlong_a = _mm512_load_si512((__m512i*)(int_data + i));
        __m512i vlong_b = _mm512_load_si512((__m512i*)(int_data + N/2 + i));
        __mmask8 long_mask = _mm512_cmpgt_epi64_mask(vlong_a, _mm512_set1_epi64(N/8));
        __m512i vlong_result = _mm512_mask_blend_epi64(long_mask, vlong_a, vlong_b);
        _mm512_store_si512((__m512i*)(int_data + i), vlong_result);
        
        // V8DFmode blend
        __m512d vdouble_a = _mm512_load_pd(double_data + i);
        __m512d vdouble_b = _mm512_load_pd(double_data + N/2 + i);
        __mmask8 double_mask = _mm512_cmp_pd_mask(vdouble_a, _mm512_set1_pd(N/8 * 0.01), _CMP_GT_OQ);
        __m512d vdouble_result = _mm512_mask_blend_pd(double_mask, vdouble_a, vdouble_b);
        _mm512_store_pd(double_data + i, vdouble_result);
    }
    
    // Compute final checksum
    for (int i = 0; i < N; i++) {
        total_sum += char_data[i] + short_data[i] + int_data[i] + 
                    (long)(float_data[i] * 100) + (long)(double_data[i] * 1000);
    }
    
    free(char_data);
    free(short_data);
    free(int_data);
    free(float_data);
    free(double_data);
    
    return total_sum;
}

#endif // __AVX512BW__
#endif // __AVX512F__

/* ========== Scalar fallback implementations ========== */
static int scalar_test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 2);
        b[i] = (char)(i * 3 + 1);
        result[i] = (a[i] > 32) ? b[i] : a[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 64; i++) sum += result[i];
    return sum;
}

static int scalar_test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 10);
        b[i] = (short)(i * 15 + 5);
        result[i] = (a[i] > 160) ? b[i] : a[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) sum += result[i];
    return sum;
}

static long scalar_test_v16si_blend(void) {
    int a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 100;
        b[i] = i * 150 + 50;
        result[i] = (a[i] > 800) ? b[i] : a[i];
    }
    
    long sum = 0;
    for (int i = 0; i < 16; i++) sum += result[i];
    return sum;
}

static long long scalar_test_v8di_blend(void) {
    long long a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1000LL;
        b[i] = i * 1500LL + 500LL;
        result[i] = (a[i] > 4000) ? b[i] : a[i];
    }
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) sum += result[i];
    return sum;
}

static float scalar_test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.75f;
        b[i] = i * 1.125f + 0.375f;
        result[i] = (a[i] > 6.0f) ? b[i] : a[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) sum += result[i];
    return sum;
}

static double scalar_test_v8df_blend(void) {
    double a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.25 + 0.75;
        result[i] = (a[i] > 6.0) ? b[i] : a[i];
    }
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) sum += result[i];
    return sum;
}

/* ========== Main driver ========== */
int main(void) {
    long total_result = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Using vectorized implementations.\n");
    
    total_result += test_v64qi_blend();
    total_result += test_v32hi_blend();
    
#ifdef __AVX512FP16__
    total_result += (long)test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total_result += (long)test_v32bf_blend();
#endif
    
    total_result += test_v16si_blend();
    total_result += test_v8di_blend();
    total_result += (long)test_v8df_blend();
    total_result += (long)test_v16sf_blend();
    total_result += test_mixed_blends();
    
#else
    printf("AVX-512BW not available. Using scalar fallbacks.\n");
    goto scalar_fallback;
#endif
#else
    printf("AVX-512 not available. Using scalar fallbacks.\n");
    goto scalar_fallback;
#endif
    
    printf("Total result: %ld\n", total_result);
    return (int)(total_result % 256);
    
scalar_fallback:
    total_result += scalar_test_v64qi_blend();
    total_result += scalar_test_v32hi_blend();
    total_result += scalar_test_v16si_blend();
    total_result += scalar_test_v8di_blend();
    total_result += (long)scalar_test_v16sf_blend();
    total_result += (long)scalar_test_v8df_blend();
    
    printf("Total result (scalar): %ld\n", total_result);
    return (int)(total_result % 256);
}
