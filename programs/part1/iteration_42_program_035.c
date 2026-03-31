#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
int test_v64qi_blend(void) {
    char a[64], b[64];
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv64qi
    __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
    
    // Use result in computation to prevent elimination
    char res[64];
    _mm512_storeu_si512((__m512i*)res, result);
    
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += res[i];
    }
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
int test_v32hi_blend(void) {
    short a[32], b[32];
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
    }
    
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    short res[32];
    _mm512_storeu_si512((__m512i*)res, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += res[i];
    }
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
float test_v32hf_blend(void) {
    _Float16 a[32], b[32];
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h va = _mm512_loadu_ph(a);
    __m512h vb = _mm512_loadu_ph(b);
    
    // Generate mask by comparing
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_LT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    _Float16 res[32];
    _mm512_storeu_ph(res, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)res[i];
    }
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
float test_v32bf_blend(void) {
    __bf16 a[32], b[32];
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float((float)(i * 1.2f));
        b[i] = bfloat16_from_float((float)(i * 2.2f));
    }
    
    __m512bh va = _mm512_loadu_si512((__m512i*)a);
    __m512bh vb = _mm512_loadu_si512((__m512i*)b);
    
    // For bfloat16, we need to use integer comparison on the underlying data
    __m512i va_int = _mm512_loadu_si512((__m512i*)a);
    __m512i vb_int = _mm512_loadu_si512((__m512i*)b);
    __mmask32 mask = _mm512_cmp_epi16_mask(va_int, vb_int, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv32bf
    __m512bh result = _mm512_mask_blend_epi16(mask, va, vb);
    
    __bf16 res[32];
    _mm512_storeu_si512((__m512i*)res, (__m512i)result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(res[i]);
    }
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
int test_v16si_blend(void) {
    int a[16], b[16];
    for (int i = 0; i < 16; i++) {
        a[i] = i * 13;
        b[i] = i * 17;
    }
    
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    int res[16];
    _mm512_storeu_si512((__m512i*)res, result);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += res[i];
    }
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
long long test_v8di_blend(void) {
    long long a[8], b[8];
    for (int i = 0; i < 8; i++) {
        a[i] = i * 23LL;
        b[i] = i * 29LL;
    }
    
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_NE);
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    long long res[8];
    _mm512_storeu_si512((__m512i*)res, result);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += res[i];
    }
    return sum;
}

/* ==================== V8DFmode (8-double-precision floats) ==================== */
double test_v8df_blend(void) {
    double a[8], b[8];
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.7;
        b[i] = i * 2.9;
    }
    
    __m512d va = _mm512_loadu_pd(a);
    __m512d vb = _mm512_loadu_pd(b);
    
    // Generate mask using comparison
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d result = _mm512_mask_blend_pd(mask, va, vb);
    
    double res[8];
    _mm512_storeu_pd(res, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += res[i];
    }
    return sum;
}

/* ==================== V16SFmode (16-single-precision floats) ==================== */
float test_v16sf_blend(void) {
    float a[16], b[16];
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1.3f;
        b[i] = i * 2.1f;
    }
    
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    
    // Generate mask using comparison
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_LT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    float res[16];
    _mm512_storeu_ps(res, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += res[i];
    }
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
void test_mixed_blends_in_loop(float* farr, double* darr, int* iarr, short* sarr, 
                               size_t size, float* fout, double* dout, int* iout, short* sout) {
    for (size_t i = 0; i < size; i += 16) {
        // Process 16 floats at a time
        if (i + 16 <= size) {
            __m512 va = _mm512_loadu_ps(&farr[i]);
            __m512 vb = _mm512_loadu_ps(&farr[(i + 8) % size]);
            __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GT_OQ);
            __m512 result = _mm512_mask_blend_ps(mask, va, vb);
            _mm512_storeu_ps(&fout[i], result);
        }
        
        // Process 8 doubles at a time
        if (i + 8 <= size) {
            __m512d vda = _mm512_loadu_pd(&darr[i]);
            __m512d vdb = _mm512_loadu_pd(&darr[(i + 4) % size]);
            __mmask8 mask = _mm512_cmp_pd_mask(vda, vdb, _CMP_LT_OQ);
            __m512d result = _mm512_mask_blend_pd(mask, vda, vdb);
            _mm512_storeu_pd(&dout[i], result);
        }
        
        // Process 16 ints at a time
        if (i + 16 <= size) {
            __m512i via = _mm512_loadu_si512((__m512i*)&iarr[i]);
            __m512i vib = _mm512_loadu_si512((__m512i*)&iarr[(i + 8) % size]);
            __mmask16 mask = _mm512_cmp_epi32_mask(via, vib, _MM_CMPINT_EQ);
            __m512i result = _mm512_mask_blend_epi32(mask, via, vib);
            _mm512_storeu_si512((__m512i*)&iout[i], result);
        }
        
        // Process 32 shorts at a time
        if (i + 32 <= size) {
            __m512i vsa = _mm512_loadu_si512((__m512i*)&sarr[i]);
            __m512i vsb = _mm512_loadu_si512((__m512i*)&sarr[(i + 16) % size]);
            __mmask32 mask = _mm512_cmp_epi16_mask(vsa, vsb, _MM_CMPINT_GT);
            __m512i result = _mm512_mask_blend_epi16(mask, vsa, vsb);
            _mm512_storeu_si512((__m512i*)&sout[i], result);
        }
    }
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ==================== Scalar Fallbacks ==================== */
int scalar_test_v64qi_blend(void) {
    char a[64], b[64];
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    char res[64];
    for (int i = 0; i < 64; i++) {
        res[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += res[i];
    }
    return sum;
}

int scalar_test_v32hi_blend(void) {
    short a[32], b[32];
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
    }
    
    short res[32];
    for (int i = 0; i < 32; i++) {
        res[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += res[i];
    }
    return sum;
}

/* ==================== Main Function ==================== */
int main(void) {
    long long total = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    // Test each vector mode individually
    total += test_v64qi_blend();
    total += test_v32hi_blend();
    total += test_v16si_blend();
    total += test_v8di_blend();
    total += test_v8df_blend();
    total += test_v16sf_blend();
    
#ifdef __AVX512FP16__
    total += (long long)test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total += (long long)test_v32bf_blend();
#endif
    
    // Test mixed data types in loop
    const size_t ARRAY_SIZE = 1024;
    float* farr = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double* darr = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int* iarr = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    short* sarr = (short*)aligned_alloc(64, ARRAY_SIZE * sizeof(short));
    
    float* fout = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double* dout = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int* iout = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    short* sout = (short*)aligned_alloc(64, ARRAY_SIZE * sizeof(short));
    
    // Initialize arrays
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = (float)(i * 1.1f);
        darr[i] = (double)(i * 1.5);
        iarr[i] = (int)(i * 3);
        sarr[i] = (short)(i * 5);
    }
    
    test_mixed_blends_in_loop(farr, darr, iarr, sarr, ARRAY_SIZE, fout, dout, iout, sout);
    
    // Compute checksums
    float fsum = 0.0f;
    double dsum = 0.0;
    int isum = 0;
    long long ssum = 0;
    
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        fsum += fout[i];
        dsum += dout[i];
        isum += iout[i];
        ssum += sout[i];
    }
    
    total += (long long)fsum + (long long)dsum + isum + ssum;
    
    free(farr); free(darr); free(iarr); free(sarr);
    free(fout); free(dout); free(iout); free(sout);
    
#else
    printf("AVX-512BW not supported. Running scalar tests...\n");
    total += scalar_test_v64qi_blend();
    total += scalar_test_v32hi_blend();
#endif /* __AVX512BW__ */
#else
    printf("AVX-512 not supported. Running scalar tests...\n");
    total += scalar_test_v64qi_blend();
    total += scalar_test_v32hi_blend();
#endif /* __AVX512F__ */
    
    printf("Final checksum: %lld\n", total);
    return (int)(total % 1000);
}
