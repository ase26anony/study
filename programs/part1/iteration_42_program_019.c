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
        data_a[i] = (char)(i * 2);
        data_b[i] = (char)(i * 3);
    }
    
    a = _mm512_loadu_si512((__m512i*)data_a);
    b = _mm512_loadu_si512((__m512i*)data_b);
    
    // Generate dynamic mask: elements where a > b
    mask = _mm512_cmpgt_epi8_mask(a, b);
    
    // Blend based on mask: if mask bit = 1, take from a, else from b
    result = _mm512_mask_blend_epi8(mask, b, a);
    
    // Store and compute checksum
    char result_data[64];
    _mm512_storeu_si512((__m512i*)result_data, result);
    
    for (int i = 0; i < 64; i++) {
        sum += result_data[i];
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
        data_a[i] = (short)(i * 10);
        data_b[i] = (short)(i * 15);
    }
    
    a = _mm512_loadu_si512((__m512i*)data_a);
    b = _mm512_loadu_si512((__m512i*)data_b);
    
    // Generate mask using comparison
    mask = _mm512_cmpgt_epi16_mask(a, b);
    
    // Blend operation for 16-bit integers
    result = _mm512_mask_blend_epi16(mask, b, a);
    
    short result_data[32];
    _mm512_storeu_si512((__m512i*)result_data, result);
    
    for (int i = 0; i < 32; i++) {
        sum += result_data[i];
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
        data_a[i] = (_Float16)(i * 1.5f);
        data_b[i] = (_Float16)(i * 2.0f);
    }
    
    a = _mm512_loadu_ph(data_a);
    b = _mm512_loadu_ph(data_b);
    
    // Generate mask: elements where a > b
    mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
    
    // Blend operation for half-precision floats
    result = _mm512_mask_blend_ph(mask, b, a);
    
    _Float16 result_data[32];
    _mm512_storeu_ph(result_data, result);
    
    for (int i = 0; i < 32; i++) {
        sum += result_data[i];
    }
    
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
float test_v32bf_blend() {
    __m512bh a, b, result;
    __mmask32 mask;
    __bfloat16 data_a[32], data_b[32];
    float sum = 0.0f;
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = bfloat16_from_float((float)i * 1.5f);
        data_b[i] = bfloat16_from_float((float)i * 2.0f);
    }
    
    a = _mm512_loadu_si512((__m512i*)data_a);
    b = _mm512_loadu_si512((__m512i*)data_b);
    
    // For bfloat16, we need to use integer blend since there's no direct bfloat16 blend
    // This should still trigger the V32BFmode case
    __m512i a_int = _mm512_loadu_si512((__m512i*)data_a);
    __m512i b_int = _mm512_loadu_si512((__m512i*)data_b);
    
    // Generate mask (using integer comparison on the same data)
    mask = _mm512_cmpgt_epi16_mask(a_int, b_int);
    
    // Use 16-bit integer blend (same width as bfloat16)
    __m512i result_int = _mm512_mask_blend_epi16(mask, b_int, a_int);
    
    __bfloat16 result_data[32];
    _mm512_storeu_si512((__m512i*)result_data, result_int);
    
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(result_data[i]);
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
        data_a[i] = i * 100;
        data_b[i] = i * 150;
    }
    
    a = _mm512_loadu_si512((__m512i*)data_a);
    b = _mm512_loadu_si512((__m512i*)data_b);
    
    mask = _mm512_cmpgt_epi32_mask(a, b);
    
    // Blend operation for 32-bit integers
    result = _mm512_mask_blend_epi32(mask, b, a);
    
    int result_data[16];
    _mm512_storeu_si512((__m512i*)result_data, result);
    
    for (int i = 0; i < 16; i++) {
        sum += result_data[i];
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
        data_a[i] = (long long)i * 1000LL;
        data_b[i] = (long long)i * 1500LL;
    }
    
    a = _mm512_loadu_si512((__m512i*)data_a);
    b = _mm512_loadu_si512((__m512i*)data_b);
    
    mask = _mm512_cmpgt_epi64_mask(a, b);
    
    // Blend operation for 64-bit integers
    result = _mm512_mask_blend_epi64(mask, b, a);
    
    long long result_data[8];
    _mm512_storeu_si512((__m512i*)result_data, result);
    
    for (int i = 0; i < 8; i++) {
        sum += result_data[i];
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
        data_a[i] = (double)i * 1.1;
        data_b[i] = (double)i * 1.5;
    }
    
    a = _mm512_loadu_pd(data_a);
    b = _mm512_loadu_pd(data_b);
    
    mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
    
    // Blend operation for double-precision floats
    result = _mm512_mask_blend_pd(mask, b, a);
    
    double result_data[8];
    _mm512_storeu_pd(result_data, result);
    
    for (int i = 0; i < 8; i++) {
        sum += result_data[i];
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
        data_a[i] = (float)i * 0.5f;
        data_b[i] = (float)i * 0.75f;
    }
    
    a = _mm512_loadu_ps(data_a);
    b = _mm512_loadu_ps(data_b);
    
    mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    
    // Blend operation for single-precision floats
    result = _mm512_mask_blend_ps(mask, b, a);
    
    float result_data[16];
    _mm512_storeu_ps(result_data, result);
    
    for (int i = 0; i < 16; i++) {
        sum += result_data[i];
    }
    
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
float test_mixed_blend_loop(int iterations) {
    float total_sum = 0.0f;
    
    for (int iter = 0; iter < iterations; iter++) {
        // Test different blend operations in sequence
        if (iter % 7 == 0) {
            // V16SFmode
            __m512 a = _mm512_set1_ps(iter * 0.1f);
            __m512 b = _mm512_set1_ps(iter * 0.2f);
            __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
            __m512 result = _mm512_mask_blend_ps(mask, b, a);
            float temp[16];
            _mm512_storeu_ps(temp, result);
            for (int i = 0; i < 16; i++) total_sum += temp[i];
        }
        else if (iter % 7 == 1) {
            // V8DFmode
            __m512d a = _mm512_set1_pd(iter * 0.01);
            __m512d b = _mm512_set1_pd(iter * 0.02);
            __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
            __m512d result = _mm512_mask_blend_pd(mask, b, a);
            double temp[8];
            _mm512_storeu_pd(temp, result);
            for (int i = 0; i < 8; i++) total_sum += (float)temp[i];
        }
        else if (iter % 7 == 2) {
            // V16SImode
            __m512i a = _mm512_set1_epi32(iter * 10);
            __m512i b = _mm512_set1_epi32(iter * 15);
            __mmask16 mask = _mm512_cmpgt_epi32_mask(a, b);
            __m512i result = _mm512_mask_blend_epi32(mask, b, a);
            int temp[16];
            _mm512_storeu_si512((__m512i*)temp, result);
            for (int i = 0; i < 16; i++) total_sum += temp[i];
        }
        else if (iter % 7 == 3) {
            // V8DImode
            __m512i a = _mm512_set1_epi64(iter * 100LL);
            __m512i b = _mm512_set1_epi64(iter * 150LL);
            __mmask8 mask = _mm512_cmpgt_epi64_mask(a, b);
            __m512i result = _mm512_mask_blend_epi64(mask, b, a);
            long long temp[8];
            _mm512_storeu_si512((__m512i*)temp, result);
            for (int i = 0; i < 8; i++) total_sum += (float)temp[i];
        }
        else if (iter % 7 == 4) {
            // V32HImode
            __m512i a = _mm512_set1_epi16(iter * 5);
            __m512i b = _mm512_set1_epi16(iter * 7);
            __mmask32 mask = _mm512_cmpgt_epi16_mask(a, b);
            __m512i result = _mm512_mask_blend_epi16(mask, b, a);
            short temp[32];
            _mm512_storeu_si512((__m512i*)temp, result);
            for (int i = 0; i < 32; i++) total_sum += temp[i];
        }
        else if (iter % 7 == 5) {
            // V64QImode
            __m512i a = _mm512_set1_epi8(iter);
            __m512i b = _mm512_set1_epi8(iter * 2);
            __mmask64 mask = _mm512_cmpgt_epi8_mask(a, b);
            __m512i result = _mm512_mask_blend_epi8(mask, b, a);
            char temp[64];
            _mm512_storeu_si512((__m512i*)temp, result);
            for (int i = 0; i < 64; i++) total_sum += temp[i];
        }
    }
    
    return total_sum;
}

#else
/* ==================== Scalar Fallback Implementations ==================== */
int test_v64qi_blend() {
    char data_a[64], data_b[64];
    int sum = 0;
    
    for (int i = 0; i < 64; i++) {
        data_a[i] = (char)(i * 2);
        data_b[i] = (char)(i * 3);
        char result = (data_a[i] > data_b[i]) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

int test_v32hi_blend() {
    short data_a[32], data_b[32];
    int sum = 0;
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = (short)(i * 10);
        data_b[i] = (short)(i * 15);
        short result = (data_a[i] > data_b[i]) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

float test_v16sf_blend() {
    float data_a[16], data_b[16];
    float sum = 0.0f;
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = (float)i * 0.5f;
        data_b[i] = (float)i * 0.75f;
        float result = (data_a[i] > data_b[i]) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

double test_v8df_blend() {
    double data_a[8], data_b[8];
    double sum = 0.0;
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = (double)i * 1.1;
        data_b[i] = (double)i * 1.5;
        double result = (data_a[i] > data_b[i]) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

int test_v16si_blend() {
    int data_a[16], data_b[16];
    int sum = 0;
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = i * 100;
        data_b[i] = i * 150;
        int result = (data_a[i] > data_b[i]) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

long long test_v8di_blend() {
    long long data_a[8], data_b[8];
    long long sum = 0;
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = (long long)i * 1000LL;
        data_b[i] = (long long)i * 1500LL;
        long long result = (data_a[i] > data_b[i]) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

float test_mixed_blend_loop(int iterations) {
    float total_sum = 0.0f;
    
    for (int iter = 0; iter < iterations; iter++) {
        // Scalar implementations of the blend operations
        if (iter % 7 == 0) {
            for (int i = 0; i < 16; i++) {
                float a = iter * 0.1f;
                float b = iter * 0.2f;
                total_sum += (a > b) ? a : b;
            }
        }
        else if (iter % 7 == 1) {
            for (int i = 0; i < 8; i++) {
                double a = iter * 0.01;
                double b = iter * 0.02;
                total_sum += (float)((a > b) ? a : b);
            }
        }
        else if (iter % 7 == 2) {
            for (int i = 0; i < 16; i++) {
                int a = iter * 10;
                int b = iter * 15;
                total_sum += (a > b) ? a : b;
            }
        }
        else if (iter % 7 == 3) {
            for (int i = 0; i < 8; i++) {
                long long a = iter * 100LL;
                long long b = iter * 150LL;
                total_sum += (float)((a > b) ? a : b);
            }
        }
        else if (iter % 7 == 4) {
            for (int i = 0; i < 32; i++) {
                short a = iter * 5;
                short b = iter * 7;
                total_sum += (a > b) ? a : b;
            }
        }
        else if (iter % 7 == 5) {
            for (int i = 0; i < 64; i++) {
                char a = iter;
                char b = iter * 2;
                total_sum += (a > b) ? a : b;
            }
        }
    }
    
    return total_sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ==================== Main Driver Function ==================== */
int main() {
    long long total_checksum = 0;
    
    printf("AVX-512 Blend Operations Test\n");
    printf("=============================\n");
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("Using AVX-512 intrinsics\n");
    
    // Run all blend tests
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
    
    // Mixed mode loop test
    float mixed_result = test_mixed_blend_loop(100);
    total_checksum += (long long)mixed_result;
    printf("Mixed mode loop test completed\n");
    
#else
    printf("AVX-512BW not available, using scalar fallback\n");
#endif
#else
    printf("AVX-512F not available, using scalar fallback\n");
#endif
    
    // Always run scalar fallbacks if intrinsics not available
#ifndef __AVX512F__
    total_checksum += test_v64qi_blend();
    total_checksum += test_v32hi_blend();
    total_checksum += (long long)test_v16sf_blend();
    total_checksum += (long long)test_v8df_blend();
    total_checksum += test_v16si_blend();
    total_checksum += test_v8di_blend();
    float mixed_result = test_mixed_blend_loop(100);
    total_checksum += (long long)mixed_result;
#endif
    
    printf("\nTotal checksum: %lld\n", total_checksum);
    printf("Test completed successfully!\n");
    
    return (int)(total_checksum & 0x7FFFFFFF);
}
