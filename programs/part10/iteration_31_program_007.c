/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targeting uncovered lines 4303-4326 in i386-expand.cc
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Feature detection macros */
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
    ALIGN_64 int8_t result[64];
    volatile ALIGN_64 int8_t volatile_result[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i * 3);
        src2[i] = (int8_t)(i * 5 + 1);
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi8(100);
    __mmask64 mask = _mm512_cmp_epi8_mask(v1, cmp_val, _MM_CMPINT_LT);
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store to volatile to prevent optimization */
    force_store((void*)volatile_result, blended);
    
    /* Use in reduction */
    int64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += volatile_result[i];
    }
    
    /* Loop with argc dependency */
    int loop_count = (argc > 1) ? argc : 4;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512i temp = _mm512_add_epi8(v1, _mm512_set1_epi8(iter));
        blended = _mm512_mask_blend_epi8(mask, temp, v2);
        force_store((void*)result, blended);
        
        /* Create artificial dependency */
        __asm__ volatile("" : : "r"(result) : "memory");
    }
    
    return (uint64_t)sum;
}
#endif

/* ==================== V32HI (32x int16) ==================== */
#if HAS_AVX512BW
static uint64_t test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 7);
        src2[i] = (int16_t)(i * 11 - 3);
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask - alternate pattern */
    __mmask32 mask = 0xAAAAAAAA; /* 10101010... pattern */
    
    /* Blend - should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(42);
    blended = _mm512_mask_blend_epi16(mask, blended, broadcast);
    
    /* Reduction */
    int64_t sum = 0;
    ALIGN_64 int16_t temp[32];
    force_store((void*)temp, blended);
    
    for (int i = 0; i < 32; i++) {
        sum += temp[i];
    }
    
    /* Loop with arithmetic operation */
    int iterations = (argc % 8) + 1;
    for (int i = 0; i < iterations; i++) {
        __m512i add_result = _mm512_add_epi16(v1, _mm512_set1_epi16(i));
        blended = _mm512_mask_blend_epi16(mask, add_result, v2);
        __asm__ volatile("" : : "r"(blended) : "memory");
    }
    
    return (uint64_t)sum;
}
#endif

/* ==================== V32HF (32x half precision) ==================== */
#if HAS_AVX512BW && defined(__AVX512FP16__)
static uint64_t test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f - 1.0f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask using comparison */
    __m512h cmp_val = _mm512_set1_ph(16.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    /* Store and reduce */
    _mm512_store_ph(result, blended);
    
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
    ALIGN_64 __bf16 result[32];
    
    /* Initialize bfloat16 values */
    for (int i = 0; i < 32; i++) {
        float f1 = i * 1.25f;
        float f2 = i * 1.75f - 0.5f;
        src1[i] = (__bf16)f1;
        src2[i] = (__bf16)f2;
    }
    
    /* Load as integers for blending */
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask - checkerboard pattern */
    __mmask32 mask = 0x55555555; /* 01010101... pattern */
    
    /* Blend using epi16 intrinsic on bfloat16 data
     * Should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Store back */
    force_store((void*)result, blended);
    
    /* Simple checksum */
    uint32_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)result[i];
    }
    
    return checksum;
}
#endif

/* ==================== V16SI (16x int32) ==================== */
#if HAS_AVX512F
static uint64_t test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 13;
        src2[i] = i * 17 + 5;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(100);
    __mmask16 mask = _mm512_cmp_epi32_mask(v1, cmp_val, _MM_CMPINT_LT);
    
    /* Blend - should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512i mul_result = _mm512_mullo_epi32(v1, _mm512_set1_epi32(3));
    blended = _mm512_mask_blend_epi32(mask, blended, mul_result);
    
    /* Store to volatile array */
    volatile ALIGN_64 int32_t volatile_store[16];
    force_store((void*)volatile_store, blended);
    
    /* Reduction */
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += volatile_store[i];
    }
    
    return (uint64_t)sum;
}
#endif

/* ==================== V8DI (8x int64) ==================== */
#if HAS_AVX512F
static uint64_t test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1000000LL;
        src2[i] = i * 2000000LL + 123456;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask - specific pattern */
    __mmask8 mask = 0xAA; /* 10101010 */
    
    /* Blend - should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    /* Use in loop with argc dependency */
    int loop_count = (argc > 2) ? (argc % 5) + 1 : 3;
    for (int i = 0; i < loop_count; i++) {
        __m512i shifted = _mm512_slli_epi64(v1, 1);
        blended = _mm512_mask_blend_epi64(mask, blended, shifted);
        __asm__ volatile("" : : "r"(blended) : "memory");
    }
    
    force_store((void*)result, blended);
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum ^= (uint64_t)result[i];
    }
    
    return checksum;
}
#endif

/* ==================== V8DF (8x double) ==================== */
#if HAS_AVX512F
static uint64_t test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.414;
        src2[i] = i * 2.718 + 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using floating comparison */
    __m512d cmp_val = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    /* Blend with arithmetic operation */
    __m512d mul_result = _mm512_mul_pd(v1, _mm512_set1_pd(1.5));
    blended = _mm512_mask_blend_pd(mask, blended, mul_result);
    
    /* Store and reduce */
    force_store_pd(result, blended);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    return (uint64_t)(sum * 1000);
}
#endif

/* ==================== V16SF (16x float) ==================== */
#if HAS_AVX512F
static uint64_t test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.785f;  /* ~π/4 */
        src2[i] = i * 1.571f + 0.5f;  /* ~π/2 */
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(10.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    /* Additional blend with broadcast value */
    __m512 broadcast = _mm512_set1_ps(3.14159f);
    blended = _mm512_mask_blend_ps(mask, blended, broadcast);
    
    /* Store to volatile */
    volatile ALIGN_64 float volatile_store[16];
    force_store_ps((void*)volatile_store, blended);
    
    /* Reduction with loop dependency */
    float sum = 0.0f;
    int elements = (argc % 16) + 1;
    for (int i = 0; i < elements; i++) {
        sum += volatile_store[i % 16];
    }
    
    return (uint64_t)(sum * 1000);
}
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char** argv) {
    uint64_t final_hash = 0;
    
    printf("Testing AVX-512 blend intrinsics coverage...\n");
    
#if HAS_AVX512F || HAS_AVX512BW
    printf("AVX-512 extensions detected.\n");
    
    /* Seed RNG for reproducible results */
    srand(0x12345678);
    
    /* Call test functions based on available extensions */
#if HAS_AVX512BW
    printf("Testing V64QI (int8) blend...\n");
    final_hash ^= test_v64qi_blend(argc);
    
    printf("Testing V32HI (int16) blend...\n");
    final_hash ^= test_v32hi_blend(argc);
    
#ifdef __AVX512FP16__
    printf("Testing V32HF (half) blend...\n");
    final_hash ^= test_v32hf_blend(argc);
#endif
    
#ifdef __AVX512BF16__
    printf("Testing V32BF (bfloat16) blend...\n");
    final_hash ^= test_v32bf_blend(argc);
#endif
#endif  /* HAS_AVX512BW */
    
#if HAS_AVX512F
    printf("Testing V16SI (int32) blend...\n");
    final_hash ^= test_v16si_blend(argc);
    
    printf("Testing V8DI (int64) blend...\n");
    final_hash ^= test_v8di_blend(argc);
    
    printf("Testing V8DF (double) blend...\n");
    final_hash ^= test_v8df_blend(argc);
    
    printf("Testing V16SF (float) blend...\n");
    final_hash ^= test_v16sf_blend(argc);
#endif
    
    printf("Final hash: 0x%016llx\n", (unsigned long long)final_hash);
    
#else
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
    return 1;
#endif
    
    return 0;
}
