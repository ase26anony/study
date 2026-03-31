#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
int test_v64qi_blend() {
    __m512i a, b, result;
    __mmask64 mask;
    char data_a[64], data_b[64];
    int sum = 0;
    
    // Initialize with alternating pattern
    for (int i = 0; i < 64; i++) {
        data_a[i] = i;
        data_b[i] = 64 - i;
    }
    
    a = _mm512_loadu_si512((__m512i*)data_a);
    b = _mm512_loadu_si512((__m512i*)data_b);
    
    // Generate dynamic mask: elements where a > 32
    mask = _mm512_cmp_epi8_mask(a, _mm512_set1_epi8(32), _MM_CMPINT_GT);
    
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
int test_v32hi_blend() {
    __m512i a, b, result;
    __mmask32 mask;
    short data_a[32], data_b[32];
    int sum = 0;
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = i * 100;
        data_b[i] = 3200 - i * 100;
    }
    
    a = _mm512_loadu_si512((__m512i*)data_a);
    b = _mm512_loadu_si512((__m512i*)data_b);
    
    // Generate mask: elements where a > 1600
    mask = _mm512_cmp_epi16_mask(a, _mm512_set1_epi16(1600), _MM_CMPINT_GT);
    
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
float test_v32hf_blend() {
    __m512h a, b, result;
    __mmask32 mask;
    _Float16 data_a[32], data_b[32];
    float sum = 0.0f;
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = (_Float16)(i * 0.5f);
        data_b[i] = (_Float16)(16.0f - i * 0.5f);
    }
    
    a = _mm512_loadu_ph(data_a);
    b = _mm512_loadu_ph(data_b);
    
    // Generate mask: elements where a > 8.0
    mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(8.0f), _CMP_GT_OQ);
    
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
float test_v32bf_blend() {
    __m512bh a, b, result;
    __mmask32 mask;
    __bf16 data_a[32], data_b[32];
    float sum = 0.0f;
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = bf16_from_float(i * 0.5f);
        data_b[i] = bf16_from_float(16.0f - i * 0.5f);
    }
    
    a = _mm512_loadu_si512((__m512i*)data_a);
    b = _mm512_loadu_si512((__m512i*)data_b);
    
    // For bfloat16, we need to use integer blend since there's no direct bfloat16 blend
    // This should still trigger the blend logic
    __m512i a_int = _mm512_castps_si512(_mm512_cvtne2ps_pbh(a, a));
    __m512i b_int = _mm512_castps_si512(_mm512_cvtne2ps_pbh(b, b));
    
    mask = _mm512_cmp_epi16_mask(_mm512_loadu_si512((__m512i*)data_a), 
                                 _mm512_set1_epi16(bf16_from_float(8.0f)), 
                                 _MM_CMPINT_GT);
    
    __m512i result_int = _mm512_mask_blend_epi16(mask, a_int, b_int);
    
    _mm512_storeu_si512((__m512i*)data_a, result_int);
    for (int i = 0; i < 32; i++) {
        sum += bf16_to_float(data_a[i]);
    }
    
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
int test_v16si_blend() {
    __m512i a, b, result;
    __mmask16 mask;
    int data_a[16], data_b[16];
    int sum = 0;
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = i * 1000;
        data_b[i] = 16000 - i * 1000;
    }
    
    a = _mm512_loadu_si512((__m512i*)data_a);
    b = _mm512_loadu_si512((__m512i*)data_b);
    
    // Generate mask: elements where a > 8000
    mask = _mm512_cmp_epi32_mask(a, _mm512_set1_epi32(8000), _MM_CMPINT_GT);
    
    // This should trigger gen_avx512f_blendmv16si
    result = _mm512_mask_blend_epi32(mask, a, b);
    
    _mm512_storeu_si512((__m512i*)data_a, result);
    for (int i = 0; i < 16; i++) {
        sum += data_a[i];
    }
    
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
long long test_v8di_blend() {
    __m512i a, b, result;
    __mmask8 mask;
    long long data_a[8], data_b[8];
    long long sum = 0;
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = i * 10000LL;
        data_b[i] = 80000LL - i * 10000LL;
    }
    
    a = _mm512_loadu_si512((__m512i*)data_a);
    b = _mm512_loadu_si512((__m512i*)data_b);
    
    // Generate mask: elements where a > 40000
    mask = _mm512_cmp_epi64_mask(a, _mm512_set1_epi64(40000), _MM_CMPINT_GT);
    
    // This should trigger gen_avx512f_blendmv8di
    result = _mm512_mask_blend_epi64(mask, a, b);
    
    _mm512_storeu_si512((__m512i*)data_a, result);
    for (int i = 0; i < 8; i++) {
        sum += data_a[i];
    }
    
    return sum;
}

/* ==================== V8DFmode (8-double-precision floats) ==================== */
double test_v8df_blend() {
    __m512d a, b, result;
    __mmask8 mask;
    double data_a[8], data_b[8];
    double sum = 0.0;
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = i * 1.5;
        data_b[i] = 12.0 - i * 1.5;
    }
    
    a = _mm512_loadu_pd(data_a);
    b = _mm512_loadu_pd(data_b);
    
    // Generate mask: elements where a > 6.0
    mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(6.0), _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    result = _mm512_mask_blend_pd(mask, a, b);
    
    _mm512_storeu_pd(data_a, result);
    for (int i = 0; i < 8; i++) {
        sum += data_a[i];
    }
    
    return sum;
}

/* ==================== V16SFmode (16-single-precision floats) ==================== */
float test_v16sf_blend() {
    __m512 a, b, result;
    __mmask16 mask;
    float data_a[16], data_b[16];
    float sum = 0.0f;
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = i * 0.25f;
        data_b[i] = 4.0f - i * 0.25f;
    }
    
    a = _mm512_loadu_ps(data_a);
    b = _mm512_loadu_ps(data_b);
    
    // Generate mask: elements where a > 2.0
    mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(2.0f), _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    result = _mm512_mask_blend_ps(mask, a, b);
    
    _mm512_storeu_ps(data_a, result);
    for (int i = 0; i < 16; i++) {
        sum += data_a[i];
    }
    
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
void test_mixed_blend_loop(float* farr, double* darr, int* iarr, short* sarr, 
                          size_t size) {
    for (size_t i = 0; i < size; i += 16) {
        // Process floats (V16SFmode)
        if (i + 16 <= size) {
            __m512 a = _mm512_loadu_ps(&farr[i]);
            __m512 b = _mm512_set1_ps(0.5f);
            __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
            __m512 result = _mm512_mask_blend_ps(mask, a, b);
            _mm512_storeu_ps(&farr[i], result);
        }
        
        // Process doubles (V8DFmode)
        if (i + 8 <= size) {
            __m512d a = _mm512_loadu_pd(&darr[i]);
            __m512d b = _mm512_set1_pd(0.5);
            __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
            __m512d result = _mm512_mask_blend_pd(mask, a, b);
            _mm512_storeu_pd(&darr[i], result);
        }
        
        // Process ints (V16SImode)
        if (i + 16 <= size) {
            __m512i a = _mm512_loadu_si512((__m512i*)&iarr[i]);
            __m512i b = _mm512_set1_epi32(100);
            __mmask16 mask = _mm512_cmp_epi32_mask(a, b, _MM_CMPINT_GT);
            __m512i result = _mm512_mask_blend_epi32(mask, a, b);
            _mm512_storeu_si512((__m512i*)&iarr[i], result);
        }
        
        // Process shorts (V32HImode)
        if (i + 32 <= size) {
            __m512i a = _mm512_loadu_si512((__m512i*)&sarr[i*2]);
            __m512i b = _mm512_set1_epi16(50);
            __mmask32 mask = _mm512_cmp_epi16_mask(a, b, _MM_CMPINT_GT);
            __m512i result = _mm512_mask_blend_epi16(mask, a, b);
            _mm512_storeu_si512((__m512i*)&sarr[i*2], result);
        }
    }
}

#endif  // __AVX512BW__
#endif  // __AVX512F__

/* ==================== Scalar Fallbacks ==================== */
#ifndef __AVX512F__

int test_v64qi_blend() {
    char data_a[64], data_b[64];
    int sum = 0;
    
    for (int i = 0; i < 64; i++) {
        data_a[i] = i;
        data_b[i] = 64 - i;
        data_a[i] = (i > 32) ? data_b[i] : data_a[i];
        sum += data_a[i];
    }
    
    return sum;
}

int test_v32hi_blend() {
    short data_a[32], data_b[32];
    int sum = 0;
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = i * 100;
        data_b[i] = 3200 - i * 100;
        data_a[i] = (i * 100 > 1600) ? data_b[i] : data_a[i];
        sum += data_a[i];
    }
    
    return sum;
}

float test_v16sf_blend() {
    float data_a[16], data_b[16];
    float sum = 0.0f;
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = i * 0.25f;
        data_b[i] = 4.0f - i * 0.25f;
        data_a[i] = (i * 0.25f > 2.0f) ? data_b[i] : data_a[i];
        sum += data_a[i];
    }
    
    return sum;
}

double test_v8df_blend() {
    double data_a[8], data_b[8];
    double sum = 0.0;
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = i * 1.5;
        data_b[i] = 12.0 - i * 1.5;
        data_a[i] = (i * 1.5 > 6.0) ? data_b[i] : data_a[i];
        sum += data_a[i];
    }
    
    return sum;
}

int test_v16si_blend() {
    int data_a[16], data_b[16];
    int sum = 0;
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = i * 1000;
        data_b[i] = 16000 - i * 1000;
        data_a[i] = (i * 1000 > 8000) ? data_b[i] : data_a[i];
        sum += data_a[i];
    }
    
    return sum;
}

long long test_v8di_blend() {
    long long data_a[8], data_b[8];
    long long sum = 0;
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = i * 10000LL;
        data_b[i] = 80000LL - i * 10000LL;
        data_a[i] = (i * 10000LL > 40000LL) ? data_b[i] : data_a[i];
        sum += data_a[i];
    }
    
    return sum;
}

void test_mixed_blend_loop(float* farr, double* darr, int* iarr, short* sarr, 
                          size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (i < size) farr[i] = (farr[i] > 0.5f) ? 0.5f : farr[i];
        if (i < size) darr[i] = (darr[i] > 0.5) ? 0.5 : darr[i];
        if (i < size) iarr[i] = (iarr[i] > 100) ? 100 : iarr[i];
        if (i < size) sarr[i] = (sarr[i] > 50) ? 50 : sarr[i];
    }
}

#endif

/* ==================== Main Driver ==================== */
int main() {
    int total_sum = 0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512F and AVX-512BW detected, using intrinsics\n");
    
    // Test each vector mode individually
    total_sum += test_v64qi_blend();
    printf("V64QImode blend test completed\n");
    
    total_sum += test_v32hi_blend();
    printf("V32HImode blend test completed\n");
    
#ifdef __AVX512FP16__
    total_sum += (int)test_v32hf_blend();
    printf("V32HFmode blend test completed\n");
#endif
    
#ifdef __AVX512BF16__
    total_sum += (int)test_v32bf_blend();
    printf("V32BFmode blend test completed\n");
#endif
    
    total_sum += test_v16si_blend();
    printf("V16SImode blend test completed\n");
    
    total_sum += (int)test_v8di_blend();
    printf("V8DImode blend test completed\n");
    
    total_sum += (int)test_v8df_blend();
    printf("V8DFmode blend test completed\n");
    
    total_sum += (int)test_v16sf_blend();
    printf("V16SFmode blend test completed\n");
    
    // Test mixed types in loop
    const size_t ARRAY_SIZE = 1024;
    float* farr = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double* darr = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int* iarr = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    short* sarr = (short*)aligned_alloc(64, ARRAY_SIZE * sizeof(short));
    
    if (farr && darr && iarr && sarr) {
        // Initialize with patterned data
        for (size_t i = 0; i < ARRAY_SIZE; i++) {
            farr[i] = (i % 100) * 0.01f;
            darr[i] = (i % 100) * 0.01;
            iarr[i] = i % 200;
            sarr[i] = i % 100;
        }
        
        test_mixed_blend_loop(farr, darr, iarr, sarr, ARRAY_SIZE);
        
        // Compute checksum
        for (size_t i = 0; i < ARRAY_SIZE; i++) {
            total_sum += (int)farr[i] + (int)darr[i] + iarr[i] + sarr[i];
        }
        
        free(farr);
        free(darr);
        free(iarr);
        free(sarr);
    }
    
#else
    printf("AVX-512BW not detected, using scalar fallback\n");
#endif
#else
    printf("AVX-512F not detected, using scalar fallback\n");
#endif
    
    // Run scalar versions if intrinsics not available
#ifndef __AVX512F__
    total_sum += test_v64qi_blend();
    total_sum += test_v32hi_blend();
    total_sum += (int)test_v16sf_blend();
    total_sum += (int)test_v8df_blend();
    total_sum += test_v16si_blend();
    total_sum += (int)test_v8di_blend();
    
    const size_t ARRAY_SIZE = 1024;
    float farr[ARRAY_SIZE];
    double darr[ARRAY_SIZE];
    int iarr[ARRAY_SIZE];
    short sarr[ARRAY_SIZE];
    
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = (i % 100) * 0.01f;
        darr[i] = (i % 100) * 0.01;
        iarr[i] = i % 200;
        sarr[i] = i % 100;
    }
    
    test_mixed_blend_loop(farr, darr, iarr, sarr, ARRAY_SIZE);
    
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        total_sum += (int)farr[i] + (int)darr[i] + iarr[i] + sarr[i];
    }
#endif
    
    printf("Total checksum: %d\n", total_sum);
    printf("All blend tests completed\n");
    
    return total_sum % 256;  // Return non-zero to indicate execution
}
