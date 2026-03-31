#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode - 64-byte integers */
int test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    __m512i va, vb, vresult;
    __mmask64 mask;
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i % 128);
        b[i] = (char)(64 - i);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Generate dynamic mask using comparison */
    mask = _mm512_cmpgt_epi8_mask(va, vb);
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    /* Compute checksum to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

/* V32HImode - 32-halfword integers */
int test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    __m512i va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)(i * 50);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    mask = _mm512_cmpgt_epi16_mask(va, vb);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

#ifdef __AVX512FP16__
/* V32HFmode - 32-half-precision floats */
float test_v32hf_blend(void) {
    _Float16 a[32], b[32], result[32];
    __m512h va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.0f);
    }
    
    va = _mm512_loadu_ph(a);
    vb = _mm512_loadu_ph(b);
    
    mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_storeu_ph(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    return sum;
}
#endif

/* V32BFmode - 32-bfloat16 floats (emulated) */
int test_v32bf_blend(void) {
    uint16_t a[32], b[32], result[32];
    __m512i va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (uint16_t)(i << 8);
        b[i] = (uint16_t)((31 - i) << 8);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Compare as 16-bit integers */
    mask = _mm512_cmpgt_epi16_mask(va, vb);
    
    /* Use epi16 blend for bfloat16 - should still trigger blend logic */
    vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* V16SImode - 16-dword integers */
int test_v16si_blend(void) {
    int a[16], b[16], result[16];
    __m512i va, vb, vresult;
    __mmask16 mask;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 500;
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    mask = _mm512_cmpgt_epi32_mask(va, vb);
    
    /* This should trigger gen_avx512f_blendmv16si */
    vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* V8DImode - 8-qword integers */
long long test_v8di_blend(void) {
    long long a[8], b[8], result[8];
    __m512i va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = (long long)i * 10000LL;
        b[i] = (long long)i * 5000LL;
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    mask = _mm512_cmpgt_epi64_mask(va, vb);
    
    /* This should trigger gen_avx512f_blendmv8di */
    vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* V8DFmode - 8-double-precision floats */
double test_v8df_blend(void) {
    double a[8], b[8], result[8];
    __m512d va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = (double)i * 1.5;
        b[i] = (double)i * 2.5;
    }
    
    va = _mm512_loadu_pd(a);
    vb = _mm512_loadu_pd(b);
    
    mask = _mm512_cmp_pd_mask(va, vb, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_storeu_pd(result, vresult);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* V16SFmode - 16-single-precision floats */
float test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    __m512 va, vb, vresult;
    __mmask16 mask;
    
    for (int i = 0; i < 16; i++) {
        a[i] = (float)i * 1.1f;
        b[i] = (float)i * 1.2f;
    }
    
    va = _mm512_loadu_ps(a);
    vb = _mm512_loadu_ps(b);
    
    mask = _mm512_cmp_ps_mask(va, vb, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_storeu_ps(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* Mixed data types in loop structure */
long long test_mixed_blends(void) {
    const int N = 1024;
    long long total = 0;
    
    /* Process different data types in separate loops */
    {
        /* V64QImode */
        char arr1[N], arr2[N], result[N];
        for (int i = 0; i < N; i += 64) {
            for (int j = 0; j < 64; j++) {
                arr1[i + j] = (char)((i + j) % 128);
                arr2[i + j] = (char)(64 - ((i + j) % 128));
            }
            
            __m512i va = _mm512_loadu_si512((const __m512i*)&arr1[i]);
            __m512i vb = _mm512_loadu_si512((const __m512i*)&arr2[i]);
            __mmask64 mask = _mm512_cmpgt_epi8_mask(va, vb);
            __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
            _mm512_storeu_si512((__m512i*)&result[i], vresult);
            
            for (int j = 0; j < 64; j++) {
                total += result[i + j];
            }
        }
    }
    
    {
        /* V16SFmode */
        float arr1[N], arr2[N], result[N];
        for (int i = 0; i < N; i += 16) {
            for (int j = 0; j < 16; j++) {
                arr1[i + j] = (float)(i + j) * 0.1f;
                arr2[i + j] = (float)(i + j) * 0.2f;
            }
            
            __m512 va = _mm512_loadu_ps(&arr1[i]);
            __m512 vb = _mm512_loadu_ps(&arr2[i]);
            __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GT_OQ);
            __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
            _mm512_storeu_ps(&result[i], vresult);
            
            for (int j = 0; j < 16; j++) {
                total += (long long)result[i + j];
            }
        }
    }
    
    return total;
}

#else /* AVX512BW not available */
/* Scalar fallback implementations */
int test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i % 128);
        b[i] = (char)(64 - i);
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    int sum = 0;
    for (int i = 0; i < 64; i++) sum += result[i];
    return sum;
}

int test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)(i * 50);
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    int sum = 0;
    for (int i = 0; i < 32; i++) sum += result[i];
    return sum;
}

float test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    for (int i = 0; i < 16; i++) {
        a[i] = (float)i * 1.1f;
        b[i] = (float)i * 1.2f;
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) sum += result[i];
    return sum;
}

double test_v8df_blend(void) {
    double a[8], b[8], result[8];
    for (int i = 0; i < 8; i++) {
        a[i] = (double)i * 1.5;
        b[i] = (double)i * 2.5;
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    double sum = 0.0;
    for (int i = 0; i < 8; i++) sum += result[i];
    return sum;
}

int test_v16si_blend(void) {
    int a[16], b[16], result[16];
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 500;
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    int sum = 0;
    for (int i = 0; i < 16; i++) sum += result[i];
    return sum;
}

long long test_v8di_blend(void) {
    long long a[8], b[8], result[8];
    for (int i = 0; i < 8; i++) {
        a[i] = (long long)i * 10000LL;
        b[i] = (long long)i * 5000LL;
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    long long sum = 0;
    for (int i = 0; i < 8; i++) sum += result[i];
    return sum;
}

int test_v32bf_blend(void) {
    uint16_t a[32], b[32], result[32];
    for (int i = 0; i < 32; i++) {
        a[i] = (uint16_t)(i << 8);
        b[i] = (uint16_t)((31 - i) << 8);
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    int sum = 0;
    for (int i = 0; i < 32; i++) sum += result[i];
    return sum;
}

long long test_mixed_blends(void) {
    return 0;
}
#endif /* AVX512BW */
#else /* AVX512F not available */
/* Minimal scalar implementations when AVX-512 not available */
int test_v64qi_blend(void) { return 0; }
int test_v32hi_blend(void) { return 0; }
float test_v16sf_blend(void) { return 0.0f; }
double test_v8df_blend(void) { return 0.0; }
int test_v16si_blend(void) { return 0; }
long long test_v8di_blend(void) { return 0; }
int test_v32bf_blend(void) { return 0; }
long long test_mixed_blends(void) { return 0; }
#endif /* AVX512F */

int main(void) {
    long long total_checksum = 0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512F__
    printf("AVX-512F supported\n");
#ifdef __AVX512BW__
    printf("AVX-512BW supported\n");
#ifdef __AVX512FP16__
    printf("AVX-512FP16 supported\n");
#endif
#endif
#endif
    
    /* Call all test functions and accumulate results */
    total_checksum += test_v64qi_blend();
    total_checksum += test_v32hi_blend();
#ifdef __AVX512FP16__
    total_checksum += (long long)test_v32hf_blend();
#endif
    total_checksum += test_v32bf_blend();
    total_checksum += test_v16si_blend();
    total_checksum += test_v8di_blend();
    total_checksum += (long long)test_v8df_blend();
    total_checksum += (long long)test_v16sf_blend();
    total_checksum += test_mixed_blends();
    
    printf("Total checksum: %lld\n", total_checksum);
    
    return (int)(total_checksum % 256);
}
