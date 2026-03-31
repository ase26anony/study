#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
int test_v64qi_blend(void) {
    __m512i a, b, result;
    __mmask64 mask;
    char data_a[64], data_b[64];
    int sum = 0;
    
    // Initialize with alternating pattern
    for (int i = 0; i < 64; i++) {
        data_a[i] = (char)(i * 2);
        data_b[i] = (char)(i * 3 + 1);
    }
    
    a = _mm512_loadu_si512((const __m512i*)data_a);
    b = _mm512_loadu_si512((const __m512i*)data_b);
    
    // Generate dynamic mask: select a[i] if a[i] > b[i], else b[i]
    mask = _mm512_cmp_epi8_mask(a, b, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv64qi
    result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Use result to prevent elimination
    _mm512_storeu_si512((__m512i*)data_a, result);
    for (int i = 0; i < 64; i++) {
        sum += data_a[i];
    }
    
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
int test_v32hi_blend(void) {
    __m512i a, b, result;
    __mmask32 mask;
    short data_a[32], data_b[32];
    int sum = 0;
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = (short)(i * 10);
        data_b[i] = (short)(i * 15 + 5);
    }
    
    a = _mm512_loadu_si512((const __m512i*)data_a);
    b = _mm512_loadu_si512((const __m512i*)data_b);
    
    mask = _mm512_cmp_epi16_mask(a, b, _MM_CMPINT_LT);
    
    // This should trigger gen_avx512bw_blendmv32hi
    result = _mm512_mask_blend_epi16(mask, a, b);
    
    _mm512_storeu_si512((__m512i*)data_a, result);
    for (int i = 0; i < 32; i++) {
        sum += data_a[i];
    }
    
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
float test_v32hf_blend(void) {
    __m512h a, b, result;
    __mmask32 mask;
    _Float16 data_a[32], data_b[32];
    float sum = 0.0f;
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = (_Float16)(i * 1.5f);
        data_b[i] = (_Float16)(i * 2.0f + 0.5f);
    }
    
    a = _mm512_loadu_ph(data_a);
    b = _mm512_loadu_ph(data_b);
    
    mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_storeu_ph(data_a, result);
    for (int i = 0; i < 32; i++) {
        sum += (float)data_a[i];
    }
    
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
float test_v32bf_blend(void) {
    __m512bh a, b, result;
    __mmask32 mask;
    __bfloat16 data_a[32], data_b[32];
    float sum = 0.0f;
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = bfloat16_from_float((float)i * 1.5f);
        data_b[i] = bfloat16_from_float((float)i * 2.0f + 0.5f);
    }
    
    a = _mm512_loadu_si512((const __m512i*)data_a);
    b = _mm512_loadu_si512((const __m512i*)data_b);
    
    // For bfloat16, we need to use integer comparison since there's no direct bfloat16 compare
    mask = _mm512_cmp_epi16_mask((__m512i)a, (__m512i)b, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv32bf
    result = _mm512_mask_blend_epi16(mask, (__m512i)a, (__m512i)b);
    
    _mm512_storeu_si512((__m512i*)data_a, (__m512i)result);
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(data_a[i]);
    }
    
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
int test_v16si_blend(void) {
    __m512i a, b, result;
    __mmask16 mask;
    int data_a[16], data_b[16];
    int sum = 0;
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = i * 100;
        data_b[i] = i * 150 + 50;
    }
    
    a = _mm512_loadu_si512((const __m512i*)data_a);
    b = _mm512_loadu_si512((const __m512i*)data_b);
    
    mask = _mm512_cmp_epi32_mask(a, b, _MM_CMPINT_EQ);
    
    // This should trigger gen_avx512f_blendmv16si
    result = _mm512_mask_blend_epi32(mask, a, b);
    
    _mm512_storeu_si512((__m512i*)data_a, result);
    for (int i = 0; i < 16; i++) {
        sum += data_a[i];
    }
    
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
long long test_v8di_blend(void) {
    __m512i a, b, result;
    __mmask8 mask;
    long long data_a[8], data_b[8];
    long long sum = 0;
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = (long long)i * 1000LL;
        data_b[i] = (long long)i * 1500LL + 500LL;
    }
    
    a = _mm512_loadu_si512((const __m512i*)data_a);
    b = _mm512_loadu_si512((const __m512i*)data_b);
    
    mask = _mm512_cmp_epi64_mask(a, b, _MM_CMPINT_NEQ);
    
    // This should trigger gen_avx512f_blendmv8di
    result = _mm512_mask_blend_epi64(mask, a, b);
    
    _mm512_storeu_si512((__m512i*)data_a, result);
    for (int i = 0; i < 8; i++) {
        sum += data_a[i];
    }
    
    return sum;
}

/* ==================== V8DFmode (8-double precision floats) ==================== */
double test_v8df_blend(void) {
    __m512d a, b, result;
    __mmask8 mask;
    double data_a[8], data_b[8];
    double sum = 0.0;
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = (double)i * 1.1;
        data_b[i] = (double)i * 2.2 + 0.5;
    }
    
    a = _mm512_loadu_pd(data_a);
    b = _mm512_loadu_pd(data_b);
    
    mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    result = _mm512_mask_blend_pd(mask, a, b);
    
    _mm512_storeu_pd(data_a, result);
    for (int i = 0; i < 8; i++) {
        sum += data_a[i];
    }
    
    return sum;
}

/* ==================== V16SFmode (16-single precision floats) ==================== */
float test_v16sf_blend(void) {
    __m512 a, b, result;
    __mmask16 mask;
    float data_a[16], data_b[16];
    float sum = 0.0f;
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = (float)i * 0.5f;
        data_b[i] = (float)i * 0.75f + 0.25f;
    }
    
    a = _mm512_loadu_ps(data_a);
    b = _mm512_loadu_ps(data_b);
    
    mask = _mm512_cmp_ps_mask(a, b, _CMP_GE_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    result = _mm512_mask_blend_ps(mask, a, b);
    
    _mm512_storeu_ps(data_a, result);
    for (int i = 0; i < 16; i++) {
        sum += data_a[i];
    }
    
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
double test_mixed_blend_loop(int iterations) {
    double total_sum = 0.0;
    
    for (int iter = 0; iter < iterations; iter++) {
        // V16SF mode
        {
            __m512 a = _mm512_set1_ps((float)iter * 0.1f);
            __m512 b = _mm512_set1_ps((float)iter * 0.2f);
            __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
            __m512 result = _mm512_mask_blend_ps(mask, a, b);
            float temp[16];
            _mm512_storeu_ps(temp, result);
            for (int i = 0; i < 16; i++) total_sum += temp[i];
        }
        
        // V8DF mode
        {
            __m512d a = _mm512_set1_pd((double)iter * 0.3);
            __m512d b = _mm512_set1_pd((double)iter * 0.4);
            __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
            __m512d result = _mm512_mask_blend_pd(mask, a, b);
            double temp[8];
            _mm512_storeu_pd(temp, result);
            for (int i = 0; i < 8; i++) total_sum += temp[i];
        }
        
        // V16SI mode
        {
            __m512i a = _mm512_set1_epi32(iter * 10);
            __m512i b = _mm512_set1_epi32(iter * 20);
            __mmask16 mask = _mm512_cmp_epi32_mask(a, b, _MM_CMPINT_EQ);
            __m512i result = _mm512_mask_blend_epi32(mask, a, b);
            int temp[16];
            _mm512_storeu_si512((__m512i*)temp, result);
            for (int i = 0; i < 16; i++) total_sum += temp[i];
        }
    }
    
    return total_sum;
}

#else  /* !__AVX512BW__ */

/* Scalar fallback implementations for systems without AVX-512BW */
int test_v64qi_blend(void) {
    char data_a[64], data_b[64];
    int sum = 0;
    
    for (int i = 0; i < 64; i++) {
        data_a[i] = (char)(i * 2);
        data_b[i] = (char)(i * 3 + 1);
        char result = (data_a[i] > data_b[i]) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

int test_v32hi_blend(void) {
    short data_a[32], data_b[32];
    int sum = 0;
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = (short)(i * 10);
        data_b[i] = (short)(i * 15 + 5);
        short result = (data_a[i] < data_b[i]) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

float test_v16sf_blend(void) {
    float data_a[16], data_b[16];
    float sum = 0.0f;
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = (float)i * 0.5f;
        data_b[i] = (float)i * 0.75f + 0.25f;
        float result = (data_a[i] >= data_b[i]) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

double test_v8df_blend(void) {
    double data_a[8], data_b[8];
    double sum = 0.0;
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = (double)i * 1.1;
        data_b[i] = (double)i * 2.2 + 0.5;
        double result = (data_a[i] < data_b[i]) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

int test_v16si_blend(void) {
    int data_a[16], data_b[16];
    int sum = 0;
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = i * 100;
        data_b[i] = i * 150 + 50;
        int result = (data_a[i] == data_b[i]) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

long long test_v8di_blend(void) {
    long long data_a[8], data_b[8];
    long long sum = 0;
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = (long long)i * 1000LL;
        data_b[i] = (long long)i * 1500LL + 500LL;
        long long result = (data_a[i] != data_b[i]) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

float test_v32hf_blend(void) { return 0.0f; }
float test_v32bf_blend(void) { return 0.0f; }
double test_mixed_blend_loop(int iterations) { return 0.0; }

#endif  /* __AVX512BW__ */
#else  /* !__AVX512F__ */

/* Scalar fallback for systems without any AVX-512 support */
int test_v64qi_blend(void) { return 0; }
int test_v32hi_blend(void) { return 0; }
float test_v32hf_blend(void) { return 0.0f; }
float test_v32bf_blend(void) { return 0.0f; }
int test_v16si_blend(void) { return 0; }
long long test_v8di_blend(void) { return 0; }
double test_v8df_blend(void) { return 0.0; }
float test_v16sf_blend(void) { return 0.0f; }
double test_mixed_blend_loop(int iterations) { return 0.0; }

#endif  /* __AVX512F__ */

/* ==================== Main Driver ==================== */
int main(void) {
    long long total_checksum = 0;
    
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
#endif
    
    // Test all vector modes
    total_checksum += test_v64qi_blend();
    printf("V64QImode test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode test completed\n");
    
#ifdef __AVX512FP16__
    total_checksum += (long long)test_v32hf_blend();
    printf("V32HFmode test completed\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += (long long)test_v32bf_blend();
    printf("V32BFmode test completed\n");
#endif
    
    total_checksum += test_v16si_blend();
    printf("V16SImode test completed\n");
    
    total_checksum += test_v8di_blend();
    printf("V8DImode test completed\n");
    
    total_checksum += (long long)test_v8df_blend();
    printf("V8DFmode test completed\n");
    
    total_checksum += (long long)test_v16sf_blend();
    printf("V16SFmode test completed\n");
    
    // Test mixed mode loop
    double mixed_result = test_mixed_blend_loop(10);
    total_checksum += (long long)mixed_result;
    printf("Mixed mode loop test completed\n");
    
    printf("Total checksum: %lld\n", total_checksum);
    
    // Return non-zero if any test produced meaningful result
    return (total_checksum != 0) ? 0 : 1;
}
