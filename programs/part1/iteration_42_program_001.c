#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>
#include <x86intrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
static int test_v64qi_blend(void) {
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    char result[64] __attribute__((aligned(64)));
    
    // Initialize with alternating patterns
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i % 16);
        b[i] = (char)((i + 8) % 16);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmpgt_epi8_mask(va, _mm512_set1_epi8(7));
    
    // This should trigger gen_avx512bw_blendmv64qi
    __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    // Compute checksum to prevent optimization
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
static int test_v32hi_blend(void) {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    short result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)(i * 50);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask: select from a where a > 1500
    __mmask32 mask = _mm512_cmpgt_epi16_mask(va, _mm512_set1_epi16(1500));
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
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
    
    // Generate mask: select from a where a > 20.0
    __mmask32 mask = _mm512_cmp_ph_mask(va, _mm512_set1_ph(20.0f), _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
static float test_v32bf_blend(void) {
    __bf16 a[32] __attribute__((aligned(64)));
    __bf16 b[32] __attribute__((aligned(64)));
    __bf16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (__bf16)(i * 1.5f);
        b[i] = (__bf16)(i * 2.5f);
    }
    
    // Load as epi16 for bfloat16 emulation
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask: select every other element
    __mmask32 mask = 0xAAAAAAAA; // 10101010... pattern
    
    // This should trigger gen_avx512bw_blendmv32bf
    // Using epi16 blend for bfloat16 (same size)
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
static long test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    int result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 500;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask: select from a where a > 8000
    __mmask16 mask = _mm512_cmpgt_epi32_mask(va, _mm512_set1_epi32(8000));
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_si512((const __m512i*)result, vresult);
    
    long sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
static long long test_v8di_blend(void) {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    long long result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = i * 5000LL;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask: select from a where a > 30000
    __mmask8 mask = _mm512_cmpgt_epi64_mask(va, _mm512_set1_epi64(30000));
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V8DFmode (8-double-precision floats) ==================== */
static double test_v8df_blend(void) {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.1;
        b[i] = i * 2.2;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    // Generate mask: select from a where a > 4.0
    __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(4.0), _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_store_pd(result, vresult);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V16SFmode (16-single-precision floats) ==================== */
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
    
    // Generate mask: select from a where a > 4.0f
    __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(4.0f), _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_store_ps(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
static long test_mixed_blend_loop(void) {
    const int N = 1024;
    
    // Allocate aligned arrays
    float* fa = (float*)_mm_malloc(N * sizeof(float), 64);
    float* fb = (float*)_mm_malloc(N * sizeof(float), 64);
    double* da = (double*)_mm_malloc(N * sizeof(double), 64);
    double* db = (double*)_mm_malloc(N * sizeof(double), 64);
    int* ia = (int*)_mm_malloc(N * sizeof(int), 64);
    int* ib = (int*)_mm_malloc(N * sizeof(int), 64);
    short* sa = (short*)_mm_malloc(N * sizeof(short), 64);
    short* sb = (short*)_mm_malloc(N * sizeof(short), 64);
    
    // Initialize
    for (int i = 0; i < N; i++) {
        fa[i] = i * 0.25f;
        fb[i] = i * 0.75f;
        da[i] = i * 0.125;
        db[i] = i * 0.375;
        ia[i] = i * 100;
        ib[i] = i * 200;
        sa[i] = (short)(i * 10);
        sb[i] = (short)(i * 20);
    }
    
    long total_sum = 0;
    
    // Process in chunks of vector size
    for (int i = 0; i < N; i += 16) {
        // Float blend (V16SFmode)
        __m512 vfa = _mm512_load_ps(&fa[i]);
        __m512 vfb = _mm512_load_ps(&fb[i]);
        __mmask16 fmask = _mm512_cmp_ps_mask(vfa, _mm512_set1_ps(32.0f), _CMP_GT_OQ);
        __m512 vfresult = _mm512_mask_blend_ps(fmask, vfa, vfb);
        _mm512_store_ps(&fa[i], vfresult);
        
        // Int blend (V16SImode) - process same indices
        if (i + 16 <= N) {
            __m512i via = _mm512_load_si512((const __m512i*)&ia[i]);
            __m512i vib = _mm512_load_si512((const __m512i*)&ib[i]);
            __mmask16 imask = _mm512_cmpgt_epi32_mask(via, _mm512_set1_epi32(800));
            __m512i viresult = _mm512_mask_blend_epi32(imask, via, vib);
            _mm512_store_si512((__m512i*)&ia[i], viresult);
        }
    }
    
    for (int i = 0; i < N; i += 8) {
        // Double blend (V8DFmode)
        __m512d vda = _mm512_load_pd(&da[i]);
        __m512d vdb = _mm512_load_pd(&db[i]);
        __mmask8 dmask = _mm512_cmp_pd_mask(vda, _mm512_set1_pd(32.0), _CMP_GT_OQ);
        __m512d vdresult = _mm512_mask_blend_pd(dmask, vda, vdb);
        _mm512_store_pd(&da[i], vdresult);
    }
    
    for (int i = 0; i < N; i += 32) {
        // Short blend (V32HImode)
        if (i + 32 <= N) {
            __m512i vsa = _mm512_load_si512((const __m512i*)&sa[i]);
            __m512i vsb = _mm512_load_si512((const __m512i*)&sb[i]);
            __mmask32 smask = _mm512_cmpgt_epi16_mask(vsa, _mm512_set1_epi16(160));
            __m512i vsresult = _mm512_mask_blend_epi16(smask, vsa, vsb);
            _mm512_store_si512((__m512i*)&sa[i], vsresult);
        }
    }
    
    // Compute checksums
    for (int i = 0; i < N; i++) {
        total_sum += (long)fa[i] + (long)da[i] + ia[i] + sa[i];
    }
    
    // Cleanup
    _mm_free(fa);
    _mm_free(fb);
    _mm_free(da);
    _mm_free(db);
    _mm_free(ia);
    _mm_free(ib);
    _mm_free(sa);
    _mm_free(sb);
    
    return total_sum;
}

#endif // __AVX512BW__
#endif // __AVX512F__

/* ==================== Scalar Fallbacks ==================== */
static int scalar_test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i % 16);
        b[i] = (char)((i + 8) % 16);
        result[i] = (a[i] > 7) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

static int scalar_test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)(i * 50);
        result[i] = (a[i] > 1500) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

static long scalar_test_v16si_blend(void) {
    int a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 500;
        result[i] = (a[i] > 8000) ? a[i] : b[i];
    }
    
    long sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

static long long scalar_test_v8di_blend(void) {
    long long a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = i * 5000LL;
        result[i] = (a[i] > 30000) ? a[i] : b[i];
    }
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

static double scalar_test_v8df_blend(void) {
    double a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.1;
        b[i] = i * 2.2;
        result[i] = (a[i] > 4.0) ? a[i] : b[i];
    }
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

static float scalar_test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 1.5f;
        result[i] = (a[i] > 4.0f) ? a[i] : b[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

static long scalar_mixed_blend_loop(void) {
    const int N = 1024;
    
    float fa[N], fb[N];
    double da[N], db[N];
    int ia[N], ib[N];
    short sa[N], sb[N];
    
    for (int i = 0; i < N; i++) {
        fa[i] = i * 0.25f;
        fb[i] = i * 0.75f;
        da[i] = i * 0.125;
        db[i] = i * 0.375;
        ia[i] = i * 100;
        ib[i] = i * 200;
        sa[i] = (short)(i * 10);
        sb[i] = (short)(i * 20);
    }
    
    for (int i = 0; i < N; i++) {
        fa[i] = (fa[i] > 32.0f) ? fa[i] : fb[i];
        da[i] = (da[i] > 32.0) ? da[i] : db[i];
        ia[i] = (ia[i] > 800) ? ia[i] : ib[i];
        sa[i] = (sa[i] > 160) ? sa[i] : sb[i];
    }
    
    long total_sum = 0;
    for (int i = 0; i < N; i++) {
        total_sum += (long)fa[i] + (long)da[i] + ia[i] + sa[i];
    }
    
    return total_sum;
}

/* ==================== Main Function ==================== */
int main(void) {
    long total_result = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    // Run all vector mode tests
    total_result += test_v64qi_blend();
    total_result += test_v32hi_blend();
    
#ifdef __AVX512FP16__
    total_result += (long)test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total_result += (long)test_v32bf_blend();
#endif
    
    total_result += test_v16si_blend();
    total_result += (long)test_v8di_blend();
    total_result += (long)test_v8df_blend();
    total_result += (long)test_v16sf_blend();
    total_result += test_mixed_blend_loop();
    
    printf("Vector tests completed. Total checksum: %ld\n", total_result);
#else
    printf("AVX-512BW not available. Running scalar tests...\n");
#endif
#else
    printf("AVX-512 not available. Running scalar tests...\n");
#endif

#ifndef __AVX512F__
    // Run scalar fallbacks
    total_result += scalar_test_v64qi_blend();
    total_result += scalar_test_v32hi_blend();
    total_result += scalar_test_v16si_blend();
    total_result += (long)scalar_test_v8di_blend();
    total_result += (long)scalar_test_v8df_blend();
    total_result += (long)scalar_test_v16sf_blend();
    total_result += scalar_mixed_blend_loop();
    
    printf("Scalar tests completed. Total checksum: %ld\n", total_result);
#endif
    
    return (int)(total_result & 0x7FFFFFFF);
}
