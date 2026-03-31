#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>
#include <x86intrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
static uint64_t test_v64qi_blend(void) {
    char src1[64] __attribute__((aligned(64)));
    char src2[64] __attribute__((aligned(64)));
    char dst[64] __attribute__((aligned(64)));
    
    // Initialize with pattern data
    for (int i = 0; i < 64; i++) {
        src1[i] = (char)(i * 2);
        src2[i] = (char)(i * 3 + 1);
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmp_epi8_mask(v1, v2, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv64qi
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    // Compute checksum to prevent optimization
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)dst[i];
    }
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
static uint64_t test_v32hi_blend(void) {
    short src1[32] __attribute__((aligned(64)));
    short src2[32] __attribute__((aligned(64)));
    short dst[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (short)(i * 10);
        src2[i] = (short)(i * 15 + 5);
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Generate mask using comparison
    __mmask32 mask = _mm512_cmp_epi16_mask(v1, v2, _MM_CMPINT_LT);
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)dst[i];
    }
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) {
    _Float16 src1[32] __attribute__((aligned(64)));
    _Float16 src2[32] __attribute__((aligned(64)));
    _Float16 dst[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.0f + 0.5f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    // Generate mask using comparison
    __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_LT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_ph(dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)(dst[i] * 100);
    }
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
static uint64_t test_v32bf_blend(void) {
    __bf16 src1[32] __attribute__((aligned(64)));
    __bf16 src2[32] __attribute__((aligned(64)));
    __bf16 dst[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (__bf16)(i * 1.2f);
        src2[i] = (__bf16)(i * 1.8f + 0.3f);
    }
    
    // Load as integers for bfloat16
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Generate mask using integer comparison
    __mmask32 mask = _mm512_cmp_epi16_mask(v1, v2, _MM_CMPINT_NE);
    
    // Use epi16 blend for bfloat16 emulation
    // This should trigger gen_avx512bw_blendmv32bf
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)dst[i];
    }
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
static uint64_t test_v16si_blend(void) {
    int src1[16] __attribute__((aligned(64)));
    int src2[16] __attribute__((aligned(64)));
    int dst[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 100;
        src2[i] = i * 150 + 50;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Generate mask using comparison
    __mmask16 mask = _mm512_cmp_epi32_mask(v1, v2, _MM_CMPINT_EQ);
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)dst[i];
    }
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
static uint64_t test_v8di_blend(void) {
    long long src1[8] __attribute__((aligned(64)));
    long long src2[8] __attribute__((aligned(64)));
    long long dst[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1000LL;
        src2[i] = i * 1500LL + 500LL;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Generate mask using comparison
    __mmask8 mask = _mm512_cmp_epi64_mask(v1, v2, _MM_CMPINT_GE);
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)dst[i];
    }
    return sum;
}

/* ==================== V8DFmode (8-double-precision floats) ==================== */
static uint64_t test_v8df_blend(void) {
    double src1[8] __attribute__((aligned(64)));
    double src2[8] __attribute__((aligned(64)));
    double dst[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.25;
        src2[i] = i * 1.75 + 0.25;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    // Generate mask using comparison
    __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_store_pd(dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)(dst[i] * 1000);
    }
    return sum;
}

/* ==================== V16SFmode (16-single-precision floats) ==================== */
static uint64_t test_v16sf_blend(void) {
    float src1[16] __attribute__((aligned(64)));
    float src2[16] __attribute__((aligned(64)));
    float dst[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 0.75f + 0.125f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    // Generate mask using comparison
    __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_LE_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_store_ps(dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)(dst[i] * 1000);
    }
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
static uint64_t test_mixed_blend_loop(void) {
    const int N = 1024;
    uint64_t total_sum = 0;
    
    // Process different data types in loops
    {
        // Float blend in loop
        float fa[N] __attribute__((aligned(64)));
        float fb[N] __attribute__((aligned(64)));
        float fc[N] __attribute__((aligned(64)));
        
        for (int i = 0; i < N; i++) {
            fa[i] = i * 0.1f;
            fb[i] = i * 0.2f + 0.05f;
        }
        
        for (int i = 0; i < N; i += 16) {
            __m512 v1 = _mm512_load_ps(&fa[i]);
            __m512 v2 = _mm512_load_ps(&fb[i]);
            __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_GT_OQ);
            __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
            _mm512_store_ps(&fc[i], result);
        }
        
        for (int i = 0; i < N; i++) {
            total_sum += (uint32_t)(fc[i] * 100);
        }
    }
    
    {
        // Double blend in loop
        double da[N] __attribute__((aligned(64)));
        double db[N] __attribute__((aligned(64)));
        double dc[N] __attribute__((aligned(64)));
        
        for (int i = 0; i < N; i++) {
            da[i] = i * 0.05;
            db[i] = i * 0.08 + 0.01;
        }
        
        for (int i = 0; i < N; i += 8) {
            __m512d v1 = _mm512_load_pd(&da[i]);
            __m512d v2 = _mm512_load_pd(&db[i]);
            __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
            __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
            _mm512_store_pd(&dc[i], result);
        }
        
        for (int i = 0; i < N; i++) {
            total_sum += (uint64_t)(dc[i] * 1000);
        }
    }
    
    {
        // Integer blend in loop
        int ia[N] __attribute__((aligned(64)));
        int ib[N] __attribute__((aligned(64)));
        int ic[N] __attribute__((aligned(64)));
        
        for (int i = 0; i < N; i++) {
            ia[i] = i * 10;
            ib[i] = i * 15 + 5;
        }
        
        for (int i = 0; i < N; i += 16) {
            __m512i v1 = _mm512_load_si512((__m512i*)&ia[i]);
            __m512i v2 = _mm512_load_si512((__m512i*)&ib[i]);
            __mmask16 mask = _mm512_cmp_epi32_mask(v1, v2, _MM_CMPINT_NE);
            __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
            _mm512_store_si512((__m512i*)&ic[i], result);
        }
        
        for (int i = 0; i < N; i++) {
            total_sum += ic[i];
        }
    }
    
    return total_sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ==================== Scalar Fallback Implementations ==================== */
static uint64_t scalar_test_v64qi_blend(void) {
    char src1[64];
    char src2[64];
    char dst[64];
    
    for (int i = 0; i < 64; i++) {
        src1[i] = (char)(i * 2);
        src2[i] = (char)(i * 3 + 1);
        dst[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)dst[i];
    }
    return sum;
}

static uint64_t scalar_test_v16sf_blend(void) {
    float src1[16];
    float src2[16];
    float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 0.75f + 0.125f;
        dst[i] = (src1[i] <= src2[i]) ? src1[i] : src2[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)(dst[i] * 1000);
    }
    return sum;
}

/* ==================== Main Driver ==================== */
int main(void) {
    uint64_t total_checksum = 0;
    
    printf("AVX-512 Blend Coverage Test\n");
    printf("===========================\n");
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512F and AVX-512BW detected. Using vectorized implementations.\n");
    
    total_checksum += test_v64qi_blend();
    printf("  V64QImode blend test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("  V32HImode blend test completed\n");
    
#ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend();
    printf("  V32HFmode blend test completed\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += test_v32bf_blend();
    printf("  V32BFmode blend test completed\n");
#endif
    
    total_checksum += test_v16si_blend();
    printf("  V16SImode blend test completed\n");
    
    total_checksum += test_v8di_blend();
    printf("  V8DImode blend test completed\n");
    
    total_checksum += test_v8df_blend();
    printf("  V8DFmode blend test completed\n");
    
    total_checksum += test_v16sf_blend();
    printf("  V16SFmode blend test completed\n");
    
    total_checksum += test_mixed_blend_loop();
    printf("  Mixed data type loop test completed\n");
    
#else
    printf("AVX-512BW not detected. Using scalar fallbacks.\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v16sf_blend();
#endif
#else
    printf("AVX-512F not detected. Using scalar fallbacks.\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v16sf_blend();
#endif
    
    printf("\nTotal checksum: %lu\n", total_checksum);
    printf("Test completed successfully.\n");
    
    return (int)(total_checksum % 256);
}
