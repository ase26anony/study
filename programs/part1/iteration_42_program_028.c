#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>
#include <x86intrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64-byte integers */
static uint64_t test_v64qi_blend(void) {
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    __mmask64 mask = _mm512_cmpgt_epi8_mask(va, _mm512_set1_epi8(50));
    __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    char result[64] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)result[i];
    }
    return sum;
}

/* V32HImode: 32-halfword integers */
static uint64_t test_v32hi_blend(void) {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)(i * 200);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    __mmask32 mask = _mm512_cmpgt_epi16_mask(va, _mm512_set1_epi16(1000));
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    short result[32] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)result[i];
    }
    return sum;
}

/* V32HFmode: 32-half-precision floats */
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) {
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h va = _mm512_load_ph((const __m512h*)a);
    __m512h vb = _mm512_load_ph((const __m512h*)b);
    
    __mmask32 mask = _mm512_cmp_ph_mask(va, _mm512_set1_ph(20.0f), _CMP_GT_OQ);
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _Float16 result[32] __attribute__((aligned(64)));
    _mm512_store_ph((__m512h*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)(result[i] * 100);
    }
    return sum;
}
#endif

/* V32BFmode: 32-bfloat16 floats */
#ifdef __AVX512BF16__
static uint64_t test_v32bf_blend(void) {
    __bfloat16 a[32] __attribute__((aligned(64)));
    __bfloat16 b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float((float)(i * 1.5f));
        b[i] = bfloat16_from_float((float)(i * 2.5f));
    }
    
    __m512bh va = _mm512_load_si512((const __m512i*)a);
    __m512bh vb = _mm512_load_si512((const __m512i*)b);
    
    __mmask32 mask = _mm512_cmp_epi16_mask((__m512i)va, _mm512_set1_epi16(0x4000), _MM_CMPINT_GT);
    __m512bh vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    __bfloat16 result[32] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)result, (__m512i)vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)result[i];
    }
    return sum;
}
#endif

/* V16SImode: 16-dword integers */
static uint64_t test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 2000;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    __mmask16 mask = _mm512_cmpgt_epi32_mask(va, _mm512_set1_epi32(5000));
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    int result[16] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)result[i];
    }
    return sum;
}

/* V8DImode: 8-qword integers */
static uint64_t test_v8di_blend(void) {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = i * 20000LL;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    __mmask8 mask = _mm512_cmpgt_epi64_mask(va, _mm512_set1_epi64(30000));
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    long long result[8] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)result[i];
    }
    return sum;
}

/* V8DFmode: 8-double-precision floats */
static uint64_t test_v8df_blend(void) {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.25;
        b[i] = i * 2.75;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(3.0), _CMP_GT_OQ);
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    double result[8] __attribute__((aligned(64)));
    _mm512_store_pd(result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)(result[i] * 1000);
    }
    return sum;
}

/* V16SFmode: 16-single-precision floats */
static uint64_t test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 1.5f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(4.0f), _CMP_GT_OQ);
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    float result[16] __attribute__((aligned(64)));
    _mm512_store_ps(result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)(result[i] * 1000);
    }
    return sum;
}

/* Mixed data types in loop structure */
static uint64_t test_mixed_blend_loop(void) {
    const int N = 1024;
    uint64_t total_sum = 0;
    
    /* Process float arrays */
    {
        float fa[N] __attribute__((aligned(64)));
        float fb[N] __attribute__((aligned(64)));
        
        for (int i = 0; i < N; i++) {
            fa[i] = (float)(i % 32);
            fb[i] = (float)(i % 64);
        }
        
        for (int i = 0; i < N; i += 16) {
            __m512 va = _mm512_load_ps(&fa[i]);
            __m512 vb = _mm512_load_ps(&fb[i]);
            __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(15.0f), _CMP_GT_OQ);
            __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
            
            float result[16] __attribute__((aligned(64)));
            _mm512_store_ps(result, vresult);
            
            for (int j = 0; j < 16; j++) {
                total_sum += (uint32_t)(result[j] * 100);
            }
        }
    }
    
    /* Process double arrays */
    {
        double da[N/2] __attribute__((aligned(64)));
        double db[N/2] __attribute__((aligned(64)));
        
        for (int i = 0; i < N/2; i++) {
            da[i] = (double)(i % 16);
            db[i] = (double)(i % 32);
        }
        
        for (int i = 0; i < N/2; i += 8) {
            __m512d va = _mm512_load_pd(&da[i]);
            __m512d vb = _mm512_load_pd(&db[i]);
            __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(7.5), _CMP_GT_OQ);
            __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
            
            double result[8] __attribute__((aligned(64)));
            _mm512_store_pd(result, vresult);
            
            for (int j = 0; j < 8; j++) {
                total_sum += (uint64_t)(result[j] * 1000);
            }
        }
    }
    
    /* Process integer arrays */
    {
        int ia[N] __attribute__((aligned(64)));
        int ib[N] __attribute__((aligned(64)));
        
        for (int i = 0; i < N; i++) {
            ia[i] = i * 2;
            ib[i] = i * 3;
        }
        
        for (int i = 0; i < N; i += 16) {
            __m512i va = _mm512_load_si512((const __m512i*)&ia[i]);
            __m512i vb = _mm512_load_si512((const __m512i*)&ib[i]);
            __mmask16 mask = _mm512_cmpgt_epi32_mask(va, _mm512_set1_epi32(500));
            __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
            
            int result[16] __attribute__((aligned(64)));
            _mm512_store_si512((__m512i*)result, vresult);
            
            for (int j = 0; j < 16; j++) {
                total_sum += (uint32_t)result[j];
            }
        }
    }
    
    return total_sum;
}

#else /* AVX-512 BW not available */
static uint64_t test_v64qi_blend(void) { return 0; }
static uint64_t test_v32hi_blend(void) { return 0; }
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) { return 0; }
#endif
#ifdef __AVX512BF16__
static uint64_t test_v32bf_blend(void) { return 0; }
#endif
static uint64_t test_v16si_blend(void) { return 0; }
static uint64_t test_v8di_blend(void) { return 0; }
static uint64_t test_v8df_blend(void) { return 0; }
static uint64_t test_v16sf_blend(void) { return 0; }
static uint64_t test_mixed_blend_loop(void) { return 0; }
#endif /* AVX-512 BW */

#else /* AVX-512 F not available */

/* Scalar fallback implementations */
static uint64_t test_v64qi_blend(void) {
    char a[64], b[64];
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    char result[64];
    for (int i = 0; i < 64; i++) {
        result[i] = (a[i] > 50) ? b[i] : a[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)result[i];
    }
    return sum;
}

static uint64_t test_v32hi_blend(void) {
    short a[32], b[32];
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)(i * 200);
    }
    
    short result[32];
    for (int i = 0; i < 32; i++) {
        result[i] = (a[i] > 1000) ? b[i] : a[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)result[i];
    }
    return sum;
}

static uint64_t test_v16si_blend(void) {
    int a[16], b[16];
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 2000;
    }
    
    int result[16];
    for (int i = 0; i < 16; i++) {
        result[i] = (a[i] > 5000) ? b[i] : a[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)result[i];
    }
    return sum;
}

static uint64_t test_v8di_blend(void) {
    long long a[8], b[8];
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = i * 20000LL;
    }
    
    long long result[8];
    for (int i = 0; i < 8; i++) {
        result[i] = (a[i] > 30000) ? b[i] : a[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)result[i];
    }
    return sum;
}

static uint64_t test_v8df_blend(void) {
    double a[8], b[8];
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.25;
        b[i] = i * 2.75;
    }
    
    double result[8];
    for (int i = 0; i < 8; i++) {
        result[i] = (a[i] > 3.0) ? b[i] : a[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)(result[i] * 1000);
    }
    return sum;
}

static uint64_t test_v16sf_blend(void) {
    float a[16], b[16];
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 1.5f;
    }
    
    float result[16];
    for (int i = 0; i < 16; i++) {
        result[i] = (a[i] > 4.0f) ? b[i] : a[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)(result[i] * 1000);
    }
    return sum;
}

static uint64_t test_mixed_blend_loop(void) {
    const int N = 1024;
    uint64_t total_sum = 0;
    
    /* Process float arrays */
    float fa[N], fb[N];
    for (int i = 0; i < N; i++) {
        fa[i] = (float)(i % 32);
        fb[i] = (float)(i % 64);
    }
    
    for (int i = 0; i < N; i++) {
        float result = (fa[i] > 15.0f) ? fb[i] : fa[i];
        total_sum += (uint32_t)(result * 100);
    }
    
    /* Process double arrays */
    double da[N/2], db[N/2];
    for (int i = 0; i < N/2; i++) {
        da[i] = (double)(i % 16);
        db[i] = (double)(i % 32);
    }
    
    for (int i = 0; i < N/2; i++) {
        double result = (da[i] > 7.5) ? db[i] : da[i];
        total_sum += (uint64_t)(result * 1000);
    }
    
    /* Process integer arrays */
    int ia[N], ib[N];
    for (int i = 0; i < N; i++) {
        ia[i] = i * 2;
        ib[i] = i * 3;
    }
    
    for (int i = 0; i < N; i++) {
        int result = (ia[i] > 500) ? ib[i] : ia[i];
        total_sum += (uint32_t)result;
    }
    
    return total_sum;
}

#endif /* AVX-512 F */

int main(void) {
    uint64_t total_checksum = 0;
    
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
    
    /* Test each vector mode */
    total_checksum += test_v64qi_blend();
    printf("V64QImode test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode test completed\n");
    
#ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend();
    printf("V32HFmode test completed\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += test_v32bf_blend();
    printf("V32BFmode test completed\n");
#endif
    
    total_checksum += test_v16si_blend();
    printf("V16SImode test completed\n");
    
    total_checksum += test_v8di_blend();
    printf("V8DImode test completed\n");
    
    total_checksum += test_v8df_blend();
    printf("V8DFmode test completed\n");
    
    total_checksum += test_v16sf_blend();
    printf("V16SFmode test completed\n");
    
    total_checksum += test_mixed_blend_loop();
    printf("Mixed blend loop test completed\n");
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return (int)(total_checksum % 256);
}
