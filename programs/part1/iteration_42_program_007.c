#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode - 64-byte integers */
static int test_v64qi_blend(void) {
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    char result[64] __attribute__((aligned(64)));
    
    /* Initialize with alternating pattern */
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 2);
        b[i] = (char)(i * 3);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate dynamic mask using comparison */
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    /* Force blend instruction generation */
    __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    /* Compute checksum to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

/* V32HImode - 32-halfword integers */
static int test_v32hi_blend(void) {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    short result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 10);
        b[i] = (short)(i * 15);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Complex mask generation */
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    /* Blend operation */
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

#ifdef __AVX512FP16__
/* V32HFmode - 32-half-precision floats */
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
    
    /* Generate mask using comparison */
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    /* Blend operation */
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph(result, vresult);
    
    /* Compute checksum */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    return sum;
}
#endif

/* V32BFmode - 32-bfloat16 (emulated using epi16) */
static float test_v32bf_blend(void) {
    uint16_t a[32] __attribute__((aligned(64)));
    uint16_t b[32] __attribute__((aligned(64)));
    uint16_t result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (uint16_t)(i * 100);
        b[i] = (uint16_t)(i * 200);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate mask - use comparison on reinterpreted data */
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_NE);
    
    /* Blend using epi16 intrinsic (same as V32HImode) */
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    return sum;
}

/* V16SImode - 16-dword integers */
static int test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    int result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 100;
        b[i] = i * 150;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Complex mask generation with bitwise operations */
    __m512i cmp = _mm512_xor_si512(va, vb);
    __mmask16 mask = _mm512_test_epi32_mask(cmp, cmp);
    
    /* Blend operation */
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* V8DImode - 8-qword integers */
static long long test_v8di_blend(void) {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    long long result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1000LL;
        b[i] = i * 2000LL;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate mask using comparison */
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_EQ);
    
    /* Blend operation */
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* V8DFmode - 8-double-precision floats */
static double test_v8df_blend(void) {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.25;
        b[i] = i * 2.75;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    /* Generate mask using floating comparison */
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
    
    /* Blend operation */
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_store_pd(result, vresult);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* V16SFmode - 16-single-precision floats */
static float test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    float result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 1.5f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    /* Generate mask using floating comparison */
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GT_OQ);
    
    /* Blend operation */
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_store_ps(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* Mixed data types in loop structure */
static double test_mixed_blend_loop(int iterations) {
    double total = 0.0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* V16SFmode blend in loop */
        {
            float a[16] __attribute__((aligned(64)));
            float b[16] __attribute__((aligned(64)));
            
            for (int i = 0; i < 16; i++) {
                a[i] = (float)(iter + i) * 0.25f;
                b[i] = (float)(iter + i) * 0.75f;
            }
            
            __m512 va = _mm512_load_ps(a);
            __m512 vb = _mm512_load_ps(b);
            __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_LT_OQ);
            __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
            
            float temp[16];
            _mm512_store_ps(temp, vresult);
            
            for (int i = 0; i < 16; i++) {
                total += temp[i];
            }
        }
        
        /* V8DFmode blend in same loop */
        {
            double a[8] __attribute__((aligned(64)));
            double b[8] __attribute__((aligned(64)));
            
            for (int i = 0; i < 8; i++) {
                a[i] = (double)(iter + i) * 0.33;
                b[i] = (double)(iter + i) * 0.66;
            }
            
            __m512d va = _mm512_load_pd(a);
            __m512d vb = _mm512_load_pd(b);
            __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_GT_OQ);
            __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
            
            double temp[8];
            _mm512_store_pd(temp, vresult);
            
            for (int i = 0; i < 8; i++) {
                total += temp[i];
            }
        }
        
        /* V16SImode blend in same loop */
        {
            int a[16] __attribute__((aligned(64)));
            int b[16] __attribute__((aligned(64)));
            
            for (int i = 0; i < 16; i++) {
                a[i] = iter * 10 + i;
                b[i] = iter * 20 + i;
            }
            
            __m512i va = _mm512_load_si512((__m512i*)a);
            __m512i vb = _mm512_load_si512((__m512i*)b);
            __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_NE);
            __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
            
            int temp[16];
            _mm512_store_si512((__m512i*)temp, vresult);
            
            for (int i = 0; i < 16; i++) {
                total += temp[i];
            }
        }
    }
    
    return total;
}

#else /* AVX512BW not available */
static int test_v64qi_blend(void) { return 0; }
static int test_v32hi_blend(void) { return 0; }
static float test_v32bf_blend(void) { return 0.0f; }
#ifdef __AVX512FP16__
static float test_v32hf_blend(void) { return 0.0f; }
#endif
static int test_v16si_blend(void) { return 0; }
static long long test_v8di_blend(void) { return 0; }
static double test_v8df_blend(void) { return 0.0; }
static float test_v16sf_blend(void) { return 0.0f; }
static double test_mixed_blend_loop(int iterations) { return 0.0; }
#endif /* AVX512BW */

#else /* AVX512F not available */

/* Scalar fallback implementations */
static int test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    int sum = 0;
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 2);
        b[i] = (char)(i * 3);
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
        sum += result[i];
    }
    return sum;
}

static int test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    int sum = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 10);
        b[i] = (short)(i * 15);
        result[i] = (a[i] < b[i]) ? a[i] : b[i];
        sum += result[i];
    }
    return sum;
}

static float test_v32bf_blend(void) {
    uint16_t a[32], b[32], result[32];
    float sum = 0.0f;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (uint16_t)(i * 100);
        b[i] = (uint16_t)(i * 200);
        result[i] = (a[i] != b[i]) ? a[i] : b[i];
        sum += (float)result[i];
    }
    return sum;
}

#ifdef __AVX512FP16__
static float test_v32hf_blend(void) {
    _Float16 a[32], b[32], result[32];
    float sum = 0.0f;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
        sum += (float)result[i];
    }
    return sum;
}
#endif

static int test_v16si_blend(void) {
    int a[16], b[16], result[16];
    int sum = 0;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 100;
        b[i] = i * 150;
        result[i] = ((a[i] ^ b[i]) != 0) ? a[i] : b[i];
        sum += result[i];
    }
    return sum;
}

static long long test_v8di_blend(void) {
    long long a[8], b[8], result[8];
    long long sum = 0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1000LL;
        b[i] = i * 2000LL;
        result[i] = (a[i] == b[i]) ? a[i] : b[i];
        sum += result[i];
    }
    return sum;
}

static double test_v8df_blend(void) {
    double a[8], b[8], result[8];
    double sum = 0.0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.25;
        b[i] = i * 2.75;
        result[i] = (a[i] < b[i]) ? a[i] : b[i];
        sum += result[i];
    }
    return sum;
}

static float test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    float sum = 0.0f;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 1.5f;
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
        sum += result[i];
    }
    return sum;
}

static double test_mixed_blend_loop(int iterations) {
    double total = 0.0;
    
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < 16; i++) {
            float a = (float)(iter + i) * 0.25f;
            float b = (float)(iter + i) * 0.75f;
            total += (a < b) ? a : b;
        }
        
        for (int i = 0; i < 8; i++) {
            double a = (double)(iter + i) * 0.33;
            double b = (double)(iter + i) * 0.66;
            total += (a > b) ? a : b;
        }
        
        for (int i = 0; i < 16; i++) {
            int a = iter * 10 + i;
            int b = iter * 20 + i;
            total += ((a ^ b) != 0) ? a : b;
        }
    }
    
    return total;
}

#endif /* AVX512F */

int main(void) {
    printf("Testing AVX-512 blend instruction generation...\n");
    
    long long total_checksum = 0;
    
    /* Test each vector mode individually */
    total_checksum += test_v64qi_blend();
    printf("V64QImode test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode test completed\n");
    
#ifdef __AVX512FP16__
    total_checksum += (long long)test_v32hf_blend();
    printf("V32HFmode test completed\n");
#endif
    
    total_checksum += (long long)test_v32bf_blend();
    printf("V32BFmode test completed\n");
    
    total_checksum += test_v16si_blend();
    printf("V16SImode test completed\n");
    
    total_checksum += test_v8di_blend();
    printf("V8DImode test completed\n");
    
    total_checksum += (long long)test_v8df_blend();
    printf("V8DFmode test completed\n");
    
    total_checksum += (long long)test_v16sf_blend();
    printf("V16SFmode test completed\n");
    
    /* Test mixed modes in loop */
    double loop_result = test_mixed_blend_loop(10);
    total_checksum += (long long)loop_result;
    printf("Mixed mode loop test completed\n");
    
    printf("Total checksum: %lld\n", total_checksum);
    
    return (int)(total_checksum & 0x7FFFFFFF);
}
