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
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        data_a[i] = i;
        data_b[i] = 64 - i;
    }
    
    a = _mm512_loadu_si512((const __m512i*)data_a);
    b = _mm512_loadu_si512((const __m512i*)data_b);
    
    /* Create dynamic mask: select where a[i] > 32 */
    mask = _mm512_cmpgt_epi8_mask(a, _mm512_set1_epi8(32));
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Use result to prevent elimination */
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
        data_a[i] = i * 100;
        data_b[i] = 3200 - i * 100;
    }
    
    a = _mm512_loadu_si512((const __m512i*)data_a);
    b = _mm512_loadu_si512((const __m512i*)data_b);
    
    /* Dynamic mask based on comparison */
    mask = _mm512_cmpgt_epi16_mask(a, _mm512_set1_epi16(1600));
    
    /* This should trigger gen_avx512bw_blendmv32hi */
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
        data_a[i] = (_Float16)(i * 0.5f);
        data_b[i] = (_Float16)(16.0f - i * 0.5f);
    }
    
    a = _mm512_loadu_ph(data_a);
    b = _mm512_loadu_ph(data_b);
    
    /* Create mask using comparison */
    mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(8.0f), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
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
        data_a[i] = bfloat16_from_float(i * 0.5f);
        data_b[i] = bfloat16_from_float(16.0f - i * 0.5f);
    }
    
    a = _mm512_loadu_si512((const __m512i*)data_a);
    b = _mm512_loadu_si512((const __m512i*)data_b);
    
    /* For bfloat16, we need to emulate blend using epi16 */
    mask = _mm512_cmp_epi16_mask((__m512i)a, _mm512_set1_epi16(0x4000), _MM_CMPINT_GT);
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    result = (__m512bh)_mm512_mask_blend_epi16(mask, (__m512i)a, (__m512i)b);
    
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
        data_a[i] = i * 1000;
        data_b[i] = 16000 - i * 1000;
    }
    
    a = _mm512_loadu_si512((const __m512i*)data_a);
    b = _mm512_loadu_si512((const __m512i*)data_b);
    
    mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(8000));
    
    /* This should trigger gen_avx512f_blendmv16si */
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
        data_a[i] = i * 10000LL;
        data_b[i] = 80000LL - i * 10000LL;
    }
    
    a = _mm512_loadu_si512((const __m512i*)data_a);
    b = _mm512_loadu_si512((const __m512i*)data_b);
    
    mask = _mm512_cmpgt_epi64_mask(a, _mm512_set1_epi64(40000));
    
    /* This should trigger gen_avx512f_blendmv8di */
    result = _mm512_mask_blend_epi64(mask, a, b);
    
    _mm512_storeu_si512((__m512i*)data_a, result);
    for (int i = 0; i < 8; i++) {
        sum += data_a[i];
    }
    
    return sum;
}

/* ==================== V8DFmode (8-double floats) ==================== */
double test_v8df_blend(void) {
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
    
    mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(6.0), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    result = _mm512_mask_blend_pd(mask, a, b);
    
    _mm512_storeu_pd(data_a, result);
    for (int i = 0; i < 8; i++) {
        sum += data_a[i];
    }
    
    return sum;
}

/* ==================== V16SFmode (16-single floats) ==================== */
float test_v16sf_blend(void) {
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
    
    mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(2.0f), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    result = _mm512_mask_blend_ps(mask, a, b);
    
    _mm512_storeu_ps(data_a, result);
    for (int i = 0; i < 16; i++) {
        sum += data_a[i];
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
    __mmask16 fmask, imask;
    __mmask8 dmask;
    __mmask32 smask;
    
    *result_int = 0;
    *result_double = 0.0;
    
    /* Process in chunks of vector size */
    for (size_t i = 0; i < size; i += 16) {
        /* Float blend */
        fvec1 = _mm512_loadu_ps(&farr[i]);
        fvec2 = _mm512_loadu_ps(&farr[i + 16]);
        fmask = _mm512_cmp_ps_mask(fvec1, fvec2, _CMP_GT_OQ);
        fresult = _mm512_mask_blend_ps(fmask, fvec1, fvec2);
        
        /* Double blend */
        dvec1 = _mm512_loadu_pd(&darr[i/2]);
        dvec2 = _mm512_loadu_pd(&darr[i/2 + 8]);
        dmask = _mm512_cmp_pd_mask(dvec1, dvec2, _CMP_GT_OQ);
        dresult = _mm512_mask_blend_pd(dmask, dvec1, dvec2);
        
        /* Integer blend */
        ivec1 = _mm512_loadu_si512((const __m512i*)&iarr[i]);
        ivec2 = _mm512_loadu_si512((const __m512i*)&iarr[i + 16]);
        imask = _mm512_cmpgt_epi32_mask(ivec1, ivec2);
        iresult = _mm512_mask_blend_epi32(imask, ivec1, ivec2);
        
        /* Short blend */
        svec1 = _mm512_loadu_si512((const __m512i*)&sarr[i*2]);
        svec2 = _mm512_loadu_si512((const __m512i*)&sarr[i*2 + 32]);
        smask = _mm512_cmpgt_epi16_mask(svec1, svec2);
        sresult = _mm512_mask_blend_epi16(smask, svec1, svec2);
        
        /* Accumulate results */
        float ftemp[16];
        double dtemp[8];
        int itemp[16];
        short stemp[32];
        
        _mm512_storeu_ps(ftemp, fresult);
        _mm512_storeu_pd(dtemp, dresult);
        _mm512_storeu_si512((__m512i*)itemp, iresult);
        _mm512_storeu_si512((__m512i*)stemp, sresult);
        
        for (int j = 0; j < 16; j++) {
            *result_int += (int)ftemp[j] + itemp[j];
        }
        for (int j = 0; j < 8; j++) {
            *result_double += dtemp[j];
        }
        for (int j = 0; j < 32; j++) {
            *result_int += stemp[j];
        }
    }
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ==================== Scalar Fallbacks ==================== */
int scalar_test_v64qi_blend(void) {
    char data_a[64], data_b[64];
    int sum = 0;
    
    for (int i = 0; i < 64; i++) {
        data_a[i] = i;
        data_b[i] = 64 - i;
        char result = (data_a[i] > 32) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

int scalar_test_v32hi_blend(void) {
    short data_a[32], data_b[32];
    int sum = 0;
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = i * 100;
        data_b[i] = 3200 - i * 100;
        short result = (data_a[i] > 1600) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

float scalar_test_v16sf_blend(void) {
    float data_a[16], data_b[16];
    float sum = 0.0f;
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = i * 0.25f;
        data_b[i] = 4.0f - i * 0.25f;
        float result = (data_a[i] > 2.0f) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

double scalar_test_v8df_blend(void) {
    double data_a[8], data_b[8];
    double sum = 0.0;
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = i * 1.5;
        data_b[i] = 12.0 - i * 1.5;
        double result = (data_a[i] > 6.0) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

int scalar_test_v16si_blend(void) {
    int data_a[16], data_b[16];
    int sum = 0;
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = i * 1000;
        data_b[i] = 16000 - i * 1000;
        int result = (data_a[i] > 8000) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

long long scalar_test_v8di_blend(void) {
    long long data_a[8], data_b[8];
    long long sum = 0;
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = i * 10000LL;
        data_b[i] = 80000LL - i * 10000LL;
        long long result = (data_a[i] > 40000) ? data_a[i] : data_b[i];
        sum += result;
    }
    
    return sum;
}

/* ==================== Main Driver ==================== */
int main(void) {
    int total_int_result = 0;
    double total_double_result = 0.0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Using vectorized blend operations.\n");
    
    /* Test each vector mode individually */
    total_int_result += test_v64qi_blend();
    total_int_result += test_v32hi_blend();
    total_int_result += test_v16si_blend();
    total_int_result += (int)test_v16sf_blend();
    
    total_double_result += test_v8df_blend();
    total_double_result += (double)test_v8di_blend();
    
#ifdef __AVX512FP16__
    total_double_result += test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total_double_result += test_v32bf_blend();
#endif
    
    /* Test mixed data types in loop */
    const size_t ARRAY_SIZE = 1024;
    float* farr = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double* darr = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int* iarr = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    short* sarr = (short*)aligned_alloc(64, ARRAY_SIZE * 2 * sizeof(short));
    
    /* Initialize with pattern */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = (float)(i % 100) * 0.1f;
        darr[i] = (double)(i % 100) * 0.2;
        iarr[i] = (int)(i % 100) * 10;
        sarr[i*2] = (short)(i % 100);
        sarr[i*2 + 1] = (short)(100 - (i % 100));
    }
    
    int loop_int_result;
    double loop_double_result;
    test_mixed_blends_in_loop(farr, darr, iarr, sarr, ARRAY_SIZE, 
                              &loop_int_result, &loop_double_result);
    
    total_int_result += loop_int_result;
    total_double_result += loop_double_result;
    
    free(farr);
    free(darr);
    free(iarr);
    free(sarr);
    
#else
    printf("AVX-512BW not detected. Using scalar fallbacks.\n");
#endif
#else
    printf("AVX-512 not detected. Using scalar fallbacks.\n");
#endif
    
    /* Fallback to scalar implementations if AVX-512 not available */
#ifndef __AVX512F__
    total_int_result += scalar_test_v64qi_blend();
    total_int_result += scalar_test_v32hi_blend();
    total_int_result += scalar_test_v16si_blend();
    total_int_result += (int)scalar_test_v16sf_blend();
    
    total_double_result += scalar_test_v8df_blend();
    total_double_result += (double)scalar_test_v8di_blend();
#endif
    
    printf("Total integer checksum: %d\n", total_int_result);
    printf("Total double checksum: %f\n", total_double_result);
    
    return (total_int_result + (int)total_double_result) % 256;
}
