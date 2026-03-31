#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include <stdint.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ========== V64QImode (64-byte integers) ========== */
static int test_v64qi_blend(void) {
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    char result[64] __attribute__((aligned(64)));
    
    // Initialize with pattern data
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
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

/* ========== V32HImode (32-halfword integers) ========== */
static int test_v32hi_blend(void) {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    short result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V32HFmode (32-half-precision floats) ========== */
#ifdef __AVX512FP16__
static float test_v32hf_blend(void) {
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    _Float16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_LT_OQ);
    
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

/* ========== V32BFmode (32-bfloat16) ========== */
#ifdef __AVX512BF16__
static float test_v32bf_blend(void) {
    __bf16 a[32] __attribute__((aligned(64)));
    __bf16 b[32] __attribute__((aligned(64)));
    __bf16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float((float)(i * 1.2f));
        b[i] = bfloat16_from_float((float)(i * 2.2f));
    }
    
    // For bfloat16, we use integer blend since there's no direct bfloat16 blend intrinsic
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Create mask by comparing as 16-bit integers
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_NE);
    
    // This should trigger gen_avx512bw_blendmv32bf
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(result[i]);
    }
    return sum;
}
#endif

/* ========== V16SImode (16-dword integers) ========== */
static long test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    int result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 13;
        b[i] = i * 17;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    long sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V8DImode (8-qword integers) ========== */
static long long test_v8di_blend(void) {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    long long result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = (long long)i * 23LL;
        b[i] = (long long)i * 29LL;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V8DFmode (8-double-precision floats) ========== */
static double test_v8df_blend(void) {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = (double)i * 1.7;
        b[i] = (double)i * 2.3;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_store_pd(result, vresult);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V16SFmode (16-single-precision floats) ========== */
static float test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    float result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = (float)i * 1.1f;
        b[i] = (float)i * 1.9f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_LT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_store_ps(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== Mixed data types in loop structure ========== */
static long test_mixed_blends(void) {
    const int N = 1024;
    long total_sum = 0;
    
    // Process different data types in separate loops
    {
        // V64QImode
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
            
            char temp[64];
            _mm512_store_si512((__m512i*)temp, result);
            for (int j = 0; j < 64; j++) {
                total_sum += temp[j];
            }
        }
    }
    
    {
        // V16SFmode
        float arr1[N] __attribute__((aligned(64)));
        float arr2[N] __attribute__((aligned(64)));
        
        for (int i = 0; i < N; i++) {
            arr1[i] = (float)i * 0.1f;
            arr2[i] = (float)i * 0.2f;
        }
        
        for (int i = 0; i < N; i += 16) {
            __m512 v1 = _mm512_load_ps(arr1 + i);
            __m512 v2 = _mm512_load_ps(arr2 + i);
            __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_LT_OQ);
            __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
            
            float temp[16];
            _mm512_store_ps(temp, result);
            for (int j = 0; j < 16; j++) {
                total_sum += (long)temp[j];
            }
        }
    }
    
    return total_sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ========== Scalar fallback implementations ========== */
static int scalar_test_v64qi_blend(void) {
    char a[64];
    char b[64];
    char result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

static int scalar_test_v32hi_blend(void) {
    short a[32];
    short b[32];
    short result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
        result[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== Main driver ========== */
int main(void) {
    long total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    total_checksum += test_v64qi_blend();
    total_checksum += test_v32hi_blend();
    total_checksum += test_v16si_blend();
    total_checksum += test_v8di_blend();
    total_checksum += (long)test_v8df_blend();
    total_checksum += (long)test_v16sf_blend();
    total_checksum += test_mixed_blends();
    
#ifdef __AVX512FP16__
    total_checksum += (long)test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total_checksum += (long)test_v32bf_blend();
#endif
    
#else
    printf("AVX-512BW not available. Running scalar tests...\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
#endif
#else
    printf("AVX-512 not available. Running scalar tests...\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
#endif
    
    printf("Total checksum: %ld\n", total_checksum);
    return (int)(total_checksum & 0x7FFFFFFF);
}
