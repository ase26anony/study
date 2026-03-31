#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>
#include <math.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
static int test_v64qi_blend(void) {
    alignas(64) int8_t a[64], b[64], result[64];
    
    // Initialize with alternating patterns
    for (int i = 0; i < 64; i++) {
        a[i] = (int8_t)(i * 2);
        b[i] = (int8_t)(i * 3 + 1);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmpgt_epi8_mask(va, _mm512_set1_epi8(32));
    
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
    alignas(64) int16_t a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (int16_t)(i * 100);
        b[i] = (int16_t)(i * 150 + 50);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask: select a where a > 1000
    __mmask32 mask = _mm512_cmpgt_epi16_mask(va, _mm512_set1_epi16(1000));
    
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
static int test_v32hf_blend(void) {
    alignas(64) _Float16 a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)(i * 0.75f + 0.25f);
    }
    
    __m512h va = _mm512_load_ph((__m512h*)a);
    __m512h vb = _mm512_load_ph((__m512h*)b);
    
    // Generate mask: select a where a > 8.0
    __mmask32 mask = _mm512_cmp_ph_mask(va, _mm512_set1_ph(8.0f), _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph((__m512h*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)result[i];
    }
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
static int test_v32bf_blend(void) {
    alignas(64) __bf16 a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        // Use integer representation for bfloat16
        a[i] = (__bf16)(i * 0x40);  // Simple pattern
        b[i] = (__bf16)(i * 0x80 + 0x40);
    }
    
    __m512bh va = _mm512_load_si512((__m512i*)a);
    __m512bh vb = _mm512_load_si512((__m512i*)b);
    
    // For bfloat16, we need to use epi16 blend since there's no direct bf16 blend
    // This emulates the blend operation
    __mmask32 mask = 0xAAAAAAAA;  // Alternating pattern
    
    // This should trigger gen_avx512bw_blendmv32bf
    // Note: We use _mm512_mask_blend_epi16 as bfloat16 is stored in 16-bit containers
    __m512i vresult = _mm512_mask_blend_epi16(mask, 
        _mm512_castsi512_si512((__m512i)va),
        _mm512_castsi512_si512((__m512i)vb));
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)result[i];
    }
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
static int test_v16si_blend(void) {
    alignas(64) int32_t a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 2000 + 500;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask using comparison
    __mmask16 mask = _mm512_cmpgt_epi32_mask(va, _mm512_set1_epi32(8000));
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
static long long test_v8di_blend(void) {
    alignas(64) int64_t a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = i * 20000LL + 5000LL;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask: select a where a > 30000
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
    alignas(64) double a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.5 + 0.5;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    // Generate mask: select a where a > 6.0
    __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(6.0), _CMP_GT_OQ);
    
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
    alignas(64) float a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.25f;
        b[i] = i * 0.5f + 0.125f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    // Generate mask: select a where a > 2.0
    __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(2.0f), _CMP_GT_OQ);
    
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
static int test_mixed_blend_loop(void) {
    const int N = 1024;
    alignas(64) float fa[N], fb[N], fr[N];
    alignas(64) double da[N/2], db[N/2], dr[N/2];
    alignas(64) int32_t ia[N/4], ib[N/4], ir[N/4];
    alignas(64) int16_t sa[N/2], sb[N/2], sr[N/2];
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        fa[i] = (float)(i % 100);
        fb[i] = (float)((i + 50) % 100);
        if (i < N/2) {
            da[i] = (double)(i % 50);
            db[i] = (double)((i + 25) % 50);
            sa[i] = (int16_t)(i % 200);
            sb[i] = (int16_t)((i + 100) % 200);
        }
        if (i < N/4) {
            ia[i] = i % 300;
            ib[i] = (i + 150) % 300;
        }
    }
    
    int total_sum = 0;
    
    // Process in AVX-512 chunks
    for (int i = 0; i < N; i += 16) {
        // Float blend
        __m512 va = _mm512_load_ps(&fa[i]);
        __m512 vb = _mm512_load_ps(&fb[i]);
        __mmask16 mask_f = _mm512_cmp_ps_mask(va, _mm512_set1_ps(50.0f), _CMP_GT_OQ);
        __m512 vr = _mm512_mask_blend_ps(mask_f, va, vb);
        _mm512_store_ps(&fr[i], vr);
        
        // Integer blend (every 4th iteration)
        if (i % 64 == 0 && i < N/4) {
            int idx = i / 4;
            __m512i via = _mm512_load_si512((__m512i*)&ia[idx]);
            __m512i vib = _mm512_load_si512((__m512i*)&ib[idx]);
            __mmask16 mask_i = _mm512_cmpgt_epi32_mask(via, _mm512_set1_epi32(150));
            __m512i vir = _mm512_mask_blend_epi32(mask_i, via, vib);
            _mm512_store_si512((__m512i*)&ir[idx], vir);
        }
        
        // Short blend (every 2nd iteration)
        if (i % 32 == 0 && i < N/2) {
            int idx = i / 2;
            __m512i vsa = _mm512_load_si512((__m512i*)&sa[idx]);
            __m512i vsb = _mm512_load_si512((__m512i*)&sb[idx]);
            __mmask32 mask_s = _mm512_cmpgt_epi16_mask(vsa, _mm512_set1_epi16(100));
            __m512i vsr = _mm512_mask_blend_epi16(mask_s, vsa, vsb);
            _mm512_store_si512((__m512i*)&sr[idx], vsr);
        }
        
        // Double blend (every 8th iteration)
        if (i % 128 == 0 && i < N/2) {
            int idx = i / 8;
            __m512d vda = _mm512_load_pd(&da[idx]);
            __m512d vdb = _mm512_load_pd(&db[idx]);
            __mmask8 mask_d = _mm512_cmp_pd_mask(vda, _mm512_set1_pd(25.0), _CMP_GT_OQ);
            __m512d vdr = _mm512_mask_blend_pd(mask_d, vda, vdb);
            _mm512_store_pd(&dr[idx], vdr);
        }
    }
    
    // Compute checksums
    for (int i = 0; i < N; i++) {
        total_sum += (int)fr[i];
        if (i < N/2) {
            total_sum += (int)dr[i];
            total_sum += sr[i];
        }
        if (i < N/4) {
            total_sum += ir[i];
        }
    }
    
    return total_sum;
}

#endif  // __AVX512BW__
#endif  // __AVX512F__

/* ==================== Scalar Fallback Implementations ==================== */
static int scalar_test_v64qi_blend(void) {
    int8_t a[64], b[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (int8_t)(i * 2);
        b[i] = (int8_t)(i * 3 + 1);
        result[i] = (a[i] > 32) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

static int scalar_test_v32hi_blend(void) {
    int16_t a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (int16_t)(i * 100);
        b[i] = (int16_t)(i * 150 + 50);
        result[i] = (a[i] > 1000) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

static int scalar_test_v16si_blend(void) {
    int32_t a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 2000 + 500;
        result[i] = (a[i] > 8000) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

static long long scalar_test_v8di_blend(void) {
    int64_t a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = i * 20000LL + 5000LL;
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
        a[i] = i * 1.5;
        b[i] = i * 2.5 + 0.5;
        result[i] = (a[i] > 6.0) ? a[i] : b[i];
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
        a[i] = i * 0.25f;
        b[i] = i * 0.5f + 0.125f;
        result[i] = (a[i] > 2.0f) ? a[i] : b[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== Main Driver ==================== */
int main(void) {
    int total_checksum = 0;
    
    printf("AVX-512 Blend Coverage Test\n");
    printf("===========================\n");
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512F and AVX-512BW detected. Using vectorized implementations.\n");
    
    // Test each vector mode
    total_checksum += test_v64qi_blend();
    printf("V64QImode blend test completed.\n");
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode blend test completed.\n");
    
#ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend();
    printf("V32HFmode blend test completed.\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += test_v32bf_blend();
    printf("V32BFmode blend test completed.\n");
#endif
    
    total_checksum += test_v16si_blend();
    printf("V16SImode blend test completed.\n");
    
    total_checksum += (int)test_v8di_blend();
    printf("V8DImode blend test completed.\n");
    
    total_checksum += (int)test_v8df_blend();
    printf("V8DFmode blend test completed.\n");
    
    total_checksum += (int)test_v16sf_blend();
    printf("V16SFmode blend test completed.\n");
    
    total_checksum += test_mixed_blend_loop();
    printf("Mixed data type loop test completed.\n");
    
#else
    printf("AVX-512BW not detected. Using scalar fallbacks.\n");
#endif
#else
    printf("AVX-512F not detected. Using scalar fallbacks.\n");
#endif

#ifndef __AVX512F__
    // Use scalar fallbacks
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
    total_checksum += scalar_test_v16si_blend();
    total_checksum += (int)scalar_test_v8di_blend();
    total_checksum += (int)scalar_test_v8df_blend();
    total_checksum += (int)scalar_test_v16sf_blend();
#endif
    
    printf("\nTotal checksum: %d\n", total_checksum);
    printf("Test completed successfully.\n");
    
    return total_checksum != 0 ? 0 : 1;
}
