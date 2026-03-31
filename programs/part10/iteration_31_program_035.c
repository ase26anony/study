/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targeting uncovered lines 4303-4326 in i386-expand.cc
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Feature guards for compilation */
#ifdef __AVX512F__
#define HAS_AVX512F 1
#else
#define HAS_AVX512F 0
#endif

#ifdef __AVX512BW__
#define HAS_AVX512BW 1
#else
#define HAS_AVX512BW 0
#endif

/* Alignment helper */
#define ALIGN_64 __attribute__((aligned(64)))

/* Volatile store to prevent optimization */
static inline void force_store(void* dest, __m512i val) {
    _mm512_store_si512(dest, val);
}

static inline void force_store_ps(void* dest, __m512 val) {
    _mm512_store_ps(dest, val);
}

static inline void force_store_pd(void* dest, __m512d val) {
    _mm512_store_pd(dest, val);
}

/* ==================== V64QI (64x int8) ==================== */
#if HAS_AVX512BW
static uint64_t test_v64qi_blend(int argc) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 volatile int8_t result[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 - i;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi8(32);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, cmp_val);
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store with volatile to prevent optimization */
    force_store((void*)result, blended);
    
    /* Use in reduction */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    
    /* Loop with argc dependency */
    int loop_count = (argc > 1) ? argc : 10;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512i temp = _mm512_load_si512((__m512i*)src1);
        __m512i blended2 = _mm512_mask_blend_epi8(mask, temp, v2);
        force_store((void*)result, blended2);
        sum += result[iter % 64];
    }
    
    return (uint64_t)sum;
}
#endif

/* ==================== V32HI (32x int16) ==================== */
#if HAS_AVX512BW
static uint64_t test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 volatile int16_t result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = i * 200;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask - blend even indices from v1, odd from v2 */
    __mmask32 mask = 0xAAAAAAAA;  /* 1010... pattern */
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    force_store((void*)result, blended);
    
    /* Reduction with side effect */
    int64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    /* Additional blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(999);
    __m512i blended2 = _mm512_mask_blend_epi16(mask, v1, broadcast);
    force_store((void*)result, blended2);
    
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    return (uint64_t)sum;
}
#endif

/* ==================== V32HF (32x half precision) ==================== */
#if HAS_AVX512BW && defined(__AVX512FP16__)
static uint64_t test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 volatile _Float16 result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.5f);
        src2[i] = (_Float16)(i * 1.5f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask using comparison */
    __m512h cmp_val = _mm512_set1_ph(8.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_ph((void*)result, blended);
    
    /* Reduction */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    
    return (uint64_t)(sum * 1000);
}
#endif

/* ==================== V32BF (32x bfloat16) ==================== */
#if HAS_AVX512BW && defined(__AVX512BF16__)
static uint64_t test_v32bf_blend(int argc) {
    ALIGN_64 __bf16 src1[32];
    ALIGN_64 __bf16 src2[32];
    ALIGN_64 volatile __bf16 result[32];
    
    /* Initialize bfloat16 values */
    for (int i = 0; i < 32; i++) {
        float f1 = i * 0.25f;
        float f2 = i * 0.75f;
        src1[i] = (__bf16)f1;
        src2[i] = (__bf16)f2;
    }
    
    /* Load as integers for blending */
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask - blend based on index parity */
    __mmask32 mask = 0x55555555;  /* 0101... pattern */
    
    /* Blend using epi16 intrinsic for bfloat16 - should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    force_store((void*)result, blended);
    
    /* Compute checksum */
    uint32_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += ((uint16_t*)result)[i];
    }
    
    return checksum;
}
#endif

/* ==================== V16SI (16x int32) ==================== */
#if HAS_AVX512F
static uint64_t test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 volatile int32_t result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Blend with mask - should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    force_store((void*)result, blended);
    
    /* Use in arithmetic operation then blend */
    __m512i added = _mm512_add_epi32(v1, _mm512_set1_epi32(100));
    __m512i blended2 = _mm512_mask_blend_epi32(mask, v1, added);
    force_store((void*)result, blended2);
    
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    return (uint64_t)sum;
}
#endif

/* ==================== V8DI (8x int64) ==================== */
#if HAS_AVX512F
static uint64_t test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 volatile int64_t result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = i * 20000LL;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask - blend first 4 from v1, last 4 from v2 */
    __mmask8 mask = 0xF0;  /* 11110000 */
    
    /* Blend with mask - should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    force_store((void*)result, blended);
    
    /* Loop with volatile iteration count */
    volatile int iterations = (argc > 2) ? argc : 5;
    int64_t sum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        __m512i temp = _mm512_load_epi64(src1);
        __m512i blended_iter = _mm512_mask_blend_epi64(mask, temp, v2);
        force_store((void*)result, blended_iter);
        sum += result[iter % 8];
    }
    
    return (uint64_t)sum;
}
#endif

/* ==================== V8DF (8x double) ==================== */
#if HAS_AVX512F
static uint64_t test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 volatile double result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 2.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __m512d cmp_val = _mm512_set1_pd(6.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend with mask - should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    force_store_pd((void*)result, blended);
    
    /* Additional blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(1.5));
    __m512d blended2 = _mm512_mask_blend_pd(mask, v1, multiplied);
    force_store_pd((void*)result, blended2);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    /* Create artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return (uint64_t)(sum * 1000);
}
#endif

/* ==================== V16SF (16x float) ==================== */
#if HAS_AVX512F
static uint64_t test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 volatile float result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.75f;
        src2[i] = i * 1.25f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(6.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend with mask - should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    force_store_ps((void*)result, blended);
    
    /* Complex control flow with blend */
    float sum = 0.0f;
    int loop_count = (argc > 0) ? (argc % 10) + 1 : 3;
    
    for (int iter = 0; iter < loop_count; iter++) {
        __m512 temp = _mm512_load_ps(src1);
        __m512 offset = _mm512_set1_ps(iter * 0.1f);
        __m512 shifted = _mm512_add_ps(temp, offset);
        
        /* Alternate mask each iteration */
        __mmask16 iter_mask = (iter % 2) ? mask : ~mask & 0xFFFF;
        __m512 blended_iter = _mm512_mask_blend_ps(iter_mask, temp, shifted);
        
        force_store_ps((void*)result, blended_iter);
        
        for (int i = 0; i < 16; i++) {
            sum += result[i];
        }
    }
    
    return (uint64_t)(sum * 1000);
}
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char** argv) {
    uint64_t total_hash = 0;
    int tests_run = 0;
    
    printf("Testing AVX-512 blend intrinsics coverage...\n");
    
#if HAS_AVX512BW
    printf("AVX-512BW supported\n");
    total_hash ^= test_v64qi_blend(argc);
    tests_run++;
    printf("  V64QI blend test completed\n");
    
    total_hash ^= test_v32hi_blend(argc);
    tests_run++;
    printf("  V32HI blend test completed\n");
    
#if defined(__AVX512FP16__)
    total_hash ^= test_v32hf_blend(argc);
    tests_run++;
    printf("  V32HF blend test completed\n");
#endif
    
#if defined(__AVX512BF16__)
    total_hash ^= test_v32bf_blend(argc);
    tests_run++;
    printf("  V32BF blend test completed\n");
#endif
#endif
    
#if HAS_AVX512F
    printf("AVX-512F supported\n");
    total_hash ^= test_v16si_blend(argc);
    tests_run++;
    printf("  V16SI blend test completed\n");
    
    total_hash ^= test_v8di_blend(argc);
    tests_run++;
    printf("  V8DI blend test completed\n");
    
    total_hash ^= test_v8df_blend(argc);
    tests_run++;
    printf("  V8DF blend test completed\n");
    
    total_hash ^= test_v16sf_blend(argc);
    tests_run++;
    printf("  V16SF blend test completed\n");
#endif
    
    if (tests_run == 0) {
        printf("No AVX-512 tests could run. Required flags:\n");
        printf("  -mavx512f for V16SI, V8DI, V8DF, V16SF\n");
        printf("  -mavx512bw for V64QI, V32HI, V32HF, V32BF\n");
        printf("  -mavx512fp16 for V32HF\n");
        printf("  -mavx512bf16 for V32BF\n");
        return 1;
    }
    
    printf("\nAll tests completed. Total hash: 0x%016lx\n", total_hash);
    printf("Tests run: %d\n", tests_run);
    
    /* Use result to affect return code (prevents dead code elimination) */
    return (total_hash % 256) == 0 ? 0 : (int)(total_hash % 256);
}
