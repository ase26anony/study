#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
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
    mask = _mm512_cmpgt_epi8_mask(a, b);
    
    // This should trigger gen_avx512bw_blendmv64qi
    result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Store and compute checksum to prevent optimization
    char result_data[64];
    _mm512_storeu_si512((__m512i*)result_data, result);
    
    for (int i = 0; i < 64; i++) {
        sum += result_data[i];
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
    
    // Generate mask based on comparison
    mask = _mm512_cmpgt_epi16_mask(a, b);
    
    // This should trigger gen_avx512bw_blendmv32hi
    result = _mm512_mask_blend_epi16(mask, a, b);
    
    short result_data[32];
    _mm512_storeu_si512((__m512i*)result_data, result);
    
    for (int i = 0; i < 32; i++) {
        sum += result_data[i];
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
    
    // Generate mask: select a if a > b
    mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    result = _mm512_mask_blend_ph(mask, a, b);
    
    _Float16 result_data[32];
    _mm512_storeu_ph(result_data, result);
    
    for (int i = 0; i < 32; i++) {
        sum += (float)result_data[i];
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
    
    // For bfloat16, we need to use integer blend since there's no direct FP16 blend
    // This should still trigger the blend logic
    mask = _mm512_cmp_epi16_mask((__m512i)a, (__m512i)b, _MM_CMPINT_GT);
    result = (__m512bh)_mm512_mask_blend_epi16(mask, (__m512i)a, (__m512i)b);
    
    __bfloat16 result_data[32];
    _mm512_storeu_si512((__m512i*)result_data, (__m512i)result);
    
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(result_data[i]);
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
    
    mask = _mm512_cmpgt_epi32_mask(a, b);
    
    // This should trigger gen_avx512f_blendmv16si
    result = _mm512_mask_blend_epi32(mask, a, b);
    
    int result_data[16];
    _mm512_storeu_si512((__m512i*)result_data, result);
    
    for (int i = 0; i < 16; i++) {
        sum += result_data[i];
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
    
    mask = _mm512_cmpgt_epi64_mask(a, b);
    
    // This should trigger gen_avx512f_blendmv8di
    result = _mm512_mask_blend_epi64(mask, a, b);
    
    long long result_data[8];
    _mm512_storeu_si512((__m512i*)result_data, result);
    
    for (int i = 0; i < 8; i++) {
        sum += result_data[i];
    }
    
    return sum;
}

/* ==================== V8DFmode (8-double-precision floats) ==================== */
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
    
    mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    result = _mm512_mask_blend_pd(mask, a, b);
    
    double result_data[8];
    _mm512_storeu_pd(result_data, result);
    
    for (int i = 0; i < 8; i++) {
        sum += result_data[i];
    }
    
    return sum;
}

/* ==================== V16SFmode (16-single-precision floats) ==================== */
float test_v16sf_blend(void) {
    __m512 a, b, result;
    __mmask16 mask;
    float data_a[16], data_b[16];
    float sum = 0.0f;
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = (float)i * 1.5f;
        data_b[i] = (float)i * 2.0f + 0.5f;
    }
    
    a = _mm512_loadu_ps(data_a);
    b = _mm512_loadu_ps(data_b);
    
    mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    result = _mm512_mask_blend_ps(mask, a, b);
    
    float result_data[16];
    _mm512_storeu_ps(result_data, result);
    
    for (int i = 0; i < 16; i++) {
        sum += result_data[i];
    }
    
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
void test_mixed_blends_in_loop(float* farr, double* darr, int* iarr, short* sarr, 
                               size_t size, int* result_int, double* result_double) {
    __m512 fvec1, fvec2, fresult;
    __m512d dvec1, dvec2, dresult;
    __m512i ivec1, ivec2, iresult;
    __m512i svec1, svec2, sresult;
    
    __mmask16 fmask;
    __mmask8 dmask;
    __mmask16 imask;
    __mmask32 smask;
    
    float fsum = 0.0f;
    double dsum = 0.0;
    int isum = 0;
    int ssum = 0;
    
    // Process in chunks of vector size
    for (size_t i = 0; i < size; i += 16) {
        // Load float vectors
        fvec1 = _mm512_loadu_ps(&farr[i]);
        fvec2 = _mm512_loadu_ps(&farr[i] + 8);
        
        // Generate mask and blend for floats
        fmask = _mm512_cmp_ps_mask(fvec1, fvec2, _CMP_GT_OQ);
        fresult = _mm512_mask_blend_ps(fmask, fvec1, fvec2);
        
        // Store and accumulate
        float ftemp[16];
        _mm512_storeu_ps(ftemp, fresult);
        for (int j = 0; j < 16 && (i + j) < size; j++) {
            fsum += ftemp[j];
        }
    }
    
    for (size_t i = 0; i < size; i += 8) {
        // Load double vectors
        dvec1 = _mm512_loadu_pd(&darr[i]);
        dvec2 = _mm512_loadu_pd(&darr[i] + 4);
        
        // Generate mask and blend for doubles
        dmask = _mm512_cmp_pd_mask(dvec1, dvec2, _CMP_GT_OQ);
        dresult = _mm512_mask_blend_pd(dmask, dvec1, dvec2);
        
        // Store and accumulate
        double dtemp[8];
        _mm512_storeu_pd(dtemp, dresult);
        for (int j = 0; j < 8 && (i + j) < size; j++) {
            dsum += dtemp[j];
        }
    }
    
    for (size_t i = 0; i < size; i += 16) {
        // Load integer vectors
        ivec1 = _mm512_loadu_si512((const __m512i*)&iarr[i]);
        ivec2 = _mm512_loadu_si512((const __m512i*)&iarr[i] + 8);
        
        // Generate mask and blend for integers
        imask = _mm512_cmpgt_epi32_mask(ivec1, ivec2);
        iresult = _mm512_mask_blend_epi32(imask, ivec1, ivec2);
        
        // Store and accumulate
        int itemp[16];
        _mm512_storeu_si512((__m512i*)itemp, iresult);
        for (int j = 0; j < 16 && (i + j) < size; j++) {
            isum += itemp[j];
        }
    }
    
    for (size_t i = 0; i < size; i += 32) {
        // Load short vectors
        svec1 = _mm512_loadu_si512((const __m512i*)&sarr[i]);
        svec2 = _mm512_loadu_si512((const __m512i*)&sarr[i] + 16);
        
        // Generate mask and blend for shorts
        smask = _mm512_cmpgt_epi16_mask(svec1, svec2);
        sresult = _mm512_mask_blend_epi16(smask, svec1, svec2);
        
        // Store and accumulate
        short stemp[32];
        _mm512_storeu_si512((__m512i*)stemp, sresult);
        for (int j = 0; j < 32 && (i + j) < size; j++) {
            ssum += stemp[j];
        }
    }
    
    *result_int = isum + ssum;
    *result_double = dsum + (double)fsum;
}

#endif  // __AVX512BW__
#endif  // __AVX512F__

/* ==================== Scalar Fallbacks ==================== */
#ifndef __AVX512F__

int test_v64qi_blend(void) {
    char data_a[64], data_b[64];
    int sum = 0;
    
    for (int i = 0; i < 64; i++) {
        data_a[i] = (char)(i * 2);
        data_b[i] = (char)(i * 3 + 1);
    }
    
    for (int i = 0; i < 64; i++) {
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
    }
    
    for (int i = 0; i < 32; i++) {
        short result = (data_a[i] > data_b[i]) ? data_a[i] : data_b[i];
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
    }
    
    for (int i = 0; i < 16; i++) {
        int result = (data_a[i] > data_b[i]) ? data_a[i] : data_b[i];
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
    }
    
    for (int i = 0; i < 8; i++) {
        long long result = (data_a[i] > data_b[i]) ? data_a[i] : data_b[i];
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
    }
    
    for (int i = 0; i < 8; i++) {
        double result = (data_a[i] > data_b[i]) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

float test_v16sf_blend(void) {
    float data_a[16], data_b[16];
    float sum = 0.0f;
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = (float)i * 1.5f;
        data_b[i] = (float)i * 2.0f + 0.5f;
    }
    
    for (int i = 0; i < 16; i++) {
        float result = (data_a[i] > data_b[i]) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

#endif

/* ==================== Main Driver ==================== */
int main(void) {
    int total_int_sum = 0;
    double total_double_sum = 0.0;
    
    printf("Testing AVX-512 blend operations...\n");
    
    // Test each vector mode individually
    total_int_sum += test_v64qi_blend();
    printf("V64QImode test completed\n");
    
    total_int_sum += test_v32hi_blend();
    printf("V32HImode test completed\n");
    
#ifdef __AVX512FP16__
    total_double_sum += test_v32hf_blend();
    printf("V32HFmode test completed\n");
#endif
    
#ifdef __AVX512BF16__
    total_double_sum += test_v32bf_blend();
    printf("V32BFmode test completed\n");
#endif
    
    total_int_sum += test_v16si_blend();
    printf("V16SImode test completed\n");
    
    total_int_sum += (int)test_v8di_blend();
    printf("V8DImode test completed\n");
    
    total_double_sum += test_v8df_blend();
    printf("V8DFmode test completed\n");
    
    total_double_sum += test_v16sf_blend();
    printf("V16SFmode test completed\n");
    
    // Test mixed data types in loop
#ifdef __AVX512F__
#ifdef __AVX512BW__
    const size_t ARRAY_SIZE = 1024;
    float* farr = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double* darr = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int* iarr = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    short* sarr = (short*)aligned_alloc(64, ARRAY_SIZE * sizeof(short));
    
    if (farr && darr && iarr && sarr) {
        // Initialize with patterned data
        for (size_t i = 0; i < ARRAY_SIZE; i++) {
            farr[i] = (float)(i % 100) * 1.1f;
            darr[i] = (double)(i % 100) * 2.2;
            iarr[i] = (int)(i % 100) * 3;
            sarr[i] = (short)(i % 100) * 4;
        }
        
        int mixed_int_result = 0;
        double mixed_double_result = 0.0;
        test_mixed_blends_in_loop(farr, darr, iarr, sarr, ARRAY_SIZE, 
                                  &mixed_int_result, &mixed_double_result);
        
        total_int_sum += mixed_int_result;
        total_double_sum += mixed_double_result;
        
        printf("Mixed data types test completed\n");
        
        free(farr);
        free(darr);
        free(iarr);
        free(sarr);
    }
#endif
#endif
    
    printf("\nFinal checksums:\n");
    printf("Integer sum: %d\n", total_int_sum);
    printf("Floating-point sum: %f\n", total_double_sum);
    
    // Return a non-zero value based on checksums to ensure execution
    return (total_int_sum + (int)total_double_sum) != 0 ? 0 : 1;
}
