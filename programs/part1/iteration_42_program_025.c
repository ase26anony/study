#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ========== V64QImode: 64-byte integers ========== */
static uint64_t test_v64qi_blend(void) {
    char a[64], b[64];
    __m512i va, vb, vresult;
    __mmask64 mask;
    
    // Initialize with alternating patterns
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    // Generate dynamic mask: select a[i] where a[i] > b[i]
    mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv64qi
    vresult = _mm512_mask_blend_epi8(mask, vb, va);
    
    // Store and compute checksum
    char result[64];
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)result[i];
    }
    return sum;
}

/* ========== V32HImode: 32-halfword integers ========== */
static uint64_t test_v32hi_blend(void) {
    short a[32], b[32];
    __m512i va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    // This should trigger gen_avx512bw_blendmv32hi
    vresult = _mm512_mask_blend_epi16(mask, vb, va);
    
    short result[32];
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)result[i];
    }
    return sum;
}

/* ========== V32HFmode: 32-half-precision floats ========== */
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) {
    _Float16 a[32], b[32];
    __m512h va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
    }
    
    va = _mm512_loadu_ph(a);
    vb = _mm512_loadu_ph(b);
    
    mask = _mm512_cmp_ph_mask(va, vb, _CMP_LT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    vresult = _mm512_mask_blend_ph(mask, vb, va);
    
    _Float16 result[32];
    _mm512_storeu_ph(result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        // Convert to integer for checksum
        sum += (uint16_t)(result[i] * 100);
    }
    return sum;
}
#endif

/* ========== V32BFmode: 32-bfloat16 ========== */
#ifdef __AVX512BF16__
static uint64_t test_v32bf_blend(void) {
    __bfloat16 a[32], b[32];
    __m512bh va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        // Use _mm_set1_epi16 to create bfloat16 values
        a[i] = (__bfloat16)(i * 1.2f);
        b[i] = (__bfloat16)(i * 2.2f);
    }
    
    va = _mm512_loadu_bf16(a);
    vb = _mm512_loadu_bf16(b);
    
    // For bfloat16, we need to use integer blend since there's no direct bfloat16 blend
    // This will use the same generator as V32HImode
    __m512i vai = _mm512_castps_si512(_mm512_castbf16_ps(va));
    __m512i vbi = _mm512_castps_si512(_mm512_castbf16_ps(vb));
    
    // Create mask based on comparison
    __m512 vaf = _mm512_castbf16_ps(va);
    __m512 vbf = _mm512_castbf16_ps(vb);
    mask = _mm512_cmp_ps_mask(vaf, vbf, _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hi through the integer path
    __m512i vresulti = _mm512_mask_blend_epi16(mask, vbi, vai);
    vresult = _mm512_castsi512_bf16(vresulti);
    
    __bfloat16 result[32];
    _mm512_storeu_bf16(result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)(result[i] * 100);
    }
    return sum;
}
#endif

/* ========== V16SImode: 16-dword integers ========== */
static uint64_t test_v16si_blend(void) {
    int a[16], b[16];
    __m512i va, vb, vresult;
    __mmask16 mask;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 13;
        b[i] = i * 17;
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
    
    // This should trigger gen_avx512f_blendmv16si
    vresult = _mm512_mask_blend_epi32(mask, vb, va);
    
    int result[16];
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)result[i];
    }
    return sum;
}

/* ========== V8DImode: 8-qword integers ========== */
static uint64_t test_v8di_blend(void) {
    long long a[8], b[8];
    __m512i va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 23LL;
        b[i] = i * 29LL;
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_NE);
    
    // This should trigger gen_avx512f_blendmv8di
    vresult = _mm512_mask_blend_epi64(mask, vb, va);
    
    long long result[8];
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)result[i];
    }
    return sum;
}

/* ========== V8DFmode: 8-double-precision floats ========== */
static uint64_t test_v8df_blend(void) {
    double a[8], b[8];
    __m512d va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.1;
        b[i] = i * 2.2;
    }
    
    va = _mm512_loadu_pd(a);
    vb = _mm512_loadu_pd(b);
    
    mask = _mm512_cmp_pd_mask(va, vb, _CMP_LE_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    vresult = _mm512_mask_blend_pd(mask, vb, va);
    
    double result[8];
    _mm512_storeu_pd(result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)(result[i] * 1000);
    }
    return sum;
}

/* ========== V16SFmode: 16-single-precision floats ========== */
static uint64_t test_v16sf_blend(void) {
    float a[16], b[16];
    __m512 va, vb, vresult;
    __mmask16 mask;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.7f;
        b[i] = i * 1.7f;
    }
    
    va = _mm512_loadu_ps(a);
    vb = _mm512_loadu_ps(b);
    
    mask = _mm512_cmp_ps_mask(va, vb, _CMP_GE_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    vresult = _mm512_mask_blend_ps(mask, vb, va);
    
    float result[16];
    _mm512_storeu_ps(result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)(result[i] * 1000);
    }
    return sum;
}

/* ========== Mixed data types in loop structure ========== */
static uint64_t test_mixed_blends(void) {
    const int N = 1024;
    uint64_t total_sum = 0;
    
    // Process arrays with different data types
    for (int iter = 0; iter < 10; iter++) {
        // Float arrays
        float fa[N], fb[N];
        for (int i = 0; i < N; i += 16) {
            for (int j = 0; j < 16 && i + j < N; j++) {
                fa[i + j] = (i + j) * 0.3f;
                fb[i + j] = (i + j) * 0.7f;
            }
            
            __m512 va = _mm512_loadu_ps(&fa[i]);
            __m512 vb = _mm512_loadu_ps(&fb[i]);
            __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GT_OQ);
            __m512 vresult = _mm512_mask_blend_ps(mask, vb, va);
            _mm512_storeu_ps(&fa[i], vresult);
        }
        
        // Double arrays
        double da[N], db[N];
        for (int i = 0; i < N; i += 8) {
            for (int j = 0; j < 8 && i + j < N; j++) {
                da[i + j] = (i + j) * 0.5;
                db[i + j] = (i + j) * 1.5;
            }
            
            __m512d va = _mm512_loadu_pd(&da[i]);
            __m512d vb = _mm512_loadu_pd(&db[i]);
            __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
            __m512d vresult = _mm512_mask_blend_pd(mask, vb, va);
            _mm512_storeu_pd(&da[i], vresult);
        }
        
        // Integer arrays
        int ia[N], ib[N];
        for (int i = 0; i < N; i += 16) {
            for (int j = 0; j < 16 && i + j < N; j++) {
                ia[i + j] = (i + j) * 3;
                ib[i + j] = (i + j) * 5;
            }
            
            __m512i va = _mm512_loadu_si512((const __m512i*)&ia[i]);
            __m512i vb = _mm512_loadu_si512((const __m512i*)&ib[i]);
            __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
            __m512i vresult = _mm512_mask_blend_epi32(mask, vb, va);
            _mm512_storeu_si512((__m512i*)&ia[i], vresult);
        }
        
        // Compute checksums
        for (int i = 0; i < N; i++) {
            total_sum += (uint32_t)(fa[i] * 100);
            total_sum += (uint64_t)(da[i] * 100);
            total_sum += (uint32_t)ia[i];
        }
    }
    
    return total_sum;
}

#endif // __AVX512BW__
#endif // __AVX512F__

/* ========== Scalar fallback implementations ========== */
static uint64_t scalar_test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)result[i];
    }
    return sum;
}

static uint64_t scalar_test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
        result[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)result[i];
    }
    return sum;
}

// ... similar scalar implementations for other modes ...

/* ========== Main driver function ========== */
int main(void) {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    // Run all vector mode tests
    total_checksum += test_v64qi_blend();
    total_checksum += test_v32hi_blend();
    
#ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total_checksum += test_v32bf_blend();
#endif
    
    total_checksum += test_v16si_blend();
    total_checksum += test_v8di_blend();
    total_checksum += test_v8df_blend();
    total_checksum += test_v16sf_blend();
    total_checksum += test_mixed_blends();
    
#else
    printf("AVX-512BW not available. Using scalar fallbacks...\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
    // ... call other scalar fallbacks ...
#endif
#else
    printf("AVX-512 not available. Using scalar fallbacks...\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
    // ... call other scalar fallbacks ...
#endif
    
    printf("Final checksum: %lu\n", total_checksum);
    
    // Return non-zero to ensure execution
    return (total_checksum > 0) ? 0 : 1;
}
