/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targeting uncovered lines 4303-4326 in i386-expand.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Feature guards for compilation on non-AVX-512 systems */
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
    _mm512_store_si512((__m512i*)dest, val);
}

static inline void force_store_ps(void* dest, __m512 val) {
    _mm512_store_ps((float*)dest, val);
}

static inline void force_store_pd(void* dest, __m512d val) {
    _mm512_store_pd((double*)dest, val);
}

/* ==================== V64QI (64x int8) ==================== */
#if HAS_AVX512BW
static int test_v64qi_blend(int argc) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 int8_t result[64];
    volatile ALIGN_64 int8_t volatile_result[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i * 3);
        src2[i] = (int8_t)(i * 5 + 1);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask: select from v1 where i % 2 == 0 */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i % 2) == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v2, v1);
    
    /* Store to volatile to prevent optimization */
    force_store((void*)volatile_result, blended);
    
    /* Also store to regular array */
    _mm512_store_si512((__m512i*)result, blended);
    
    /* Use result in computation */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    
    /* Loop with argc dependency */
    int loop_count = (argc > 1) ? 100 : 10;
    for (int iter = 0; iter < loop_count; iter++) {
        /* Blend with broadcast scalar */
        __m512i broadcast = _mm512_set1_epi8((int8_t)iter);
        __m512i temp = _mm512_mask_blend_epi8(mask ^ 0xAAAAAAAAAAAAAAAAULL, 
                                             blended, broadcast);
        
        /* Create artificial dependency */
        __asm__ volatile("" : : "r"(temp) : "memory");
        
        sum += _mm512_reduce_add_epi8(temp);
    }
    
    return sum & 0xFF; /* Return checksum */
}
#endif

/* ==================== V32HI (32x int16) ==================== */
#if HAS_AVX512BW
static int test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t result[32];
    volatile ALIGN_64 int16_t volatile_result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 100);
        src2[i] = (int16_t)(i * 200 + 50);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i compare_val = _mm512_set1_epi16(1000);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, compare_val);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    force_store((void*)volatile_result, blended);
    _mm512_store_si512((__m512i*)result, blended);
    
    /* Use in arithmetic operation */
    __m512i added = _mm512_add_epi16(blended, _mm512_set1_epi16(10));
    __m512i final = _mm512_mask_blend_epi16(mask, blended, added);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    sum += _mm512_reduce_add_epi16(final);
    
    return sum & 0xFFFF;
}
#endif

/* ==================== V32HF (32x half precision) ==================== */
#if HAS_AVX512BW
#include <x86intrin.h> /* For _Float16 if available */
static int test_v32hf_blend(int argc) {
    ALIGN_64 uint16_t src1_data[32]; /* Store as uint16 for half float */
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 uint16_t result_data[32];
    
    /* Initialize half floats (stored as 16-bit integers) */
    for (int i = 0; i < 32; i++) {
        src1_data[i] = (uint16_t)(i * 0x0400); /* Approx i * 0.25 */
        src2_data[i] = (uint16_t)(i * 0x0800 + 0x0400); /* Approx i * 0.5 + 0.25 */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2_data);
    
    /* Create alternating mask */
    __mmask32 mask = 0xAAAAAAAA; /* 1010... pattern */
    
    /* For half precision, we need to use the appropriate intrinsic.
     * _mm512_mask_blend_ph is the intrinsic for half precision blend.
     * We'll cast through __m512h if available, otherwise use epi16.
     */
    #ifdef __AVX512FP16__
    /* If compiler supports AVX512-FP16 directly */
    __m512h h1 = _mm512_castsi512_ph(v1);
    __m512h h2 = _mm512_castsi512_ph(v2);
    __m512h blended_h = _mm512_mask_blend_ph(mask, h2, h1);
    __m512i blended = _mm512_castph_si512(blended_h);
    #else
    /* Fallback: use epi16 blend on the integer representation */
    /* This should still trigger the V32HFmode pattern if the types are right */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    #endif
    
    _mm512_store_si512((__m512i*)result_data, blended);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result_data[i];
    }
    
    /* Loop with blend inside */
    int loop_count = argc > 2 ? 50 : 20;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512i iter_vec = _mm512_set1_epi16((int16_t)iter);
        blended = _mm512_mask_blend_epi16(mask >> (iter & 0x1F), blended, iter_vec);
        sum += _mm512_reduce_add_epi16(blended);
    }
    
    return sum & 0xFFFF;
}
#endif

/* ==================== V32BF (32x bfloat16) ==================== */
#if HAS_AVX512BW
static int test_v32bf_blend(int argc) {
    ALIGN_64 uint16_t src1_data[32]; /* BF16 stored as 16-bit */
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 uint16_t result_data[32];
    
    /* Initialize bfloat16 patterns */
    for (int i = 0; i < 32; i++) {
        /* Simple bfloat16 pattern: exponent = 127 (1.0), mantissa = i */
        src1_data[i] = (uint16_t)(0x3F80 + (i & 0x7F));
        src2_data[i] = (uint16_t)(0x4000 + (i & 0x7F)); /* 2.0 + i/128 */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2_data);
    
    /* Create mask based on comparison */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((src1_data[i] & 0x1) == 0) { /* Even mantissa */
            mask |= (1U << i);
        }
    }
    
    /* BF16 uses the same integer blend as V32HI for blending */
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)result_data, blended);
    
    /* Use in computation with arithmetic */
    __m512i added = _mm512_add_epi16(blended, _mm512_set1_epi16(0x0100));
    __m512i final_blend = _mm512_mask_blend_epi16(mask ^ 0x55555555, blended, added);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result_data[i];
    }
    sum += _mm512_reduce_add_epi16(final_blend);
    
    return sum & 0xFFFF;
}
#endif

/* ==================== V16SI (16x int32) ==================== */
#if HAS_AVX512F
static int test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t result[16];
    volatile ALIGN_64 int32_t volatile_result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000 + 500;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison intrinsic */
    __m512i compare = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, compare);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v2, v1);
    
    force_store((void*)volatile_result, blended);
    _mm512_store_si512((__m512i*)result, blended);
    
    /* Blend with arithmetic result */
    __m512i multiplied = _mm512_mullo_epi32(blended, _mm512_set1_epi32(3));
    __m512i final = _mm512_mask_blend_epi32(mask ^ 0xAAAA, blended, multiplied);
    
    int sum = _mm512_reduce_add_epi32(final);
    
    /* Loop with argc-dependent iterations */
    int loop_count = (argc > 3) ? 100 : 30;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512i iter_vec = _mm512_set1_epi32(iter);
        final = _mm512_mask_blend_epi32((mask >> (iter & 0xF)) & 0xFFFF, 
                                       final, iter_vec);
        sum += _mm512_reduce_add_epi32(final);
    }
    
    return sum;
}
#endif

/* ==================== V8DI (8x int64) ==================== */
#if HAS_AVX512F
static long long test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (int64_t)i * 10000LL;
        src2[i] = (int64_t)i * 20000LL + 5000LL;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask: select where i % 2 == 0 */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((i % 2) == 0) {
            mask |= (1 << i);
        }
    }
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)result, blended);
    
    /* Use in computation */
    __m512i shifted = _mm512_slli_epi64(blended, 2);
    __m512i final = _mm512_mask_blend_epi64(mask ^ 0x55, blended, shifted);
    
    /* Reduce sum */
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    /* Add reduced sum from final vector */
    __m256i low = _mm512_castsi512_si256(final);
    __m256i high = _mm512_extracti64x4_epi64(final, 1);
    __m256i sum256 = _mm256_add_epi64(low, high);
    
    long long temp[4];
    _mm256_store_si256((__m256i*)temp, sum256);
    sum += temp[0] + temp[1] + temp[2] + temp[3];
    
    return sum;
}
#endif

/* ==================== V8DF (8x double) ==================== */
#if HAS_AVX512F
static double test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double result[8];
    volatile ALIGN_64 double volatile_result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 2.5 + 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __m512d compare = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, compare, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v2, v1);
    
    force_store_pd((void*)volatile_result, blended);
    _mm512_store_pd(result, blended);
    
    /* Blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(blended, _mm512_set1_pd(1.5));
    __m512d final = _mm512_mask_blend_pd(mask ^ 0xAA, blended, multiplied);
    
    /* Horizontal sum */
    __m256d low = _mm512_castpd512_pd256(final);
    __m256d high = _mm512_extractf64x4_pd(final, 1);
    __m256d sum256 = _mm256_add_pd(low, high);
    __m128d sum128 = _mm_add_pd(_mm256_castpd256_pd128(sum256),
                               _mm256_extractf128_pd(sum256, 1));
    sum128 = _mm_add_pd(sum128, _mm_permute_pd(sum128, 1));
    
    double sum;
    _mm_store_sd(&sum, sum128);
    
    /* Add array sum */
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    return sum;
}
#endif

/* ==================== V16SF (16x float) ==================== */
#if HAS_AVX512F
static float test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float result[16];
    volatile ALIGN_64 float volatile_result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 1.5f + 0.25f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 compare = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, compare, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v2, v1);
    
    force_store_ps((void*)volatile_result, blended);
    _mm512_store_ps(result, blended);
    
    /* Blend with arithmetic result */
    __m512 added = _mm512_add_ps(blended, _mm512_set1_ps(10.0f));
    __m512 final = _mm512_mask_blend_ps(mask ^ 0xAAAA, blended, added);
    
    /* Horizontal sum */
    __m256 low = _mm512_castps512_ps256(final);
    __m256 high = _mm512_extractf32x8_ps(final, 1);
    __m256 sum256 = _mm256_add_ps(low, high);
    __m128 sum128 = _mm_add_ps(_mm256_castps256_ps128(sum256),
                              _mm256_extractf128_ps(sum256, 1));
    sum128 = _mm_add_ps(sum128, _mm_permute_ps(sum128, 0x4E));
    sum128 = _mm_add_ps(sum128, _mm_permute_ps(sum128, 0xB1));
    
    float sum;
    _mm_store_ss(&sum, sum128);
    
    /* Add array sum and loop computation */
    int loop_count = (argc > 4) ? 80 : 40;
    for (int iter = 0; iter < loop_count; iter++) {
        for (int i = 0; i < 16; i++) {
            sum += result[i] * 0.01f;
        }
        /* Create dependency */
        __asm__ volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char** argv) {
    int total_checksum = 0;
    
    printf("Testing AVX-512 blend intrinsics coverage...\n");
    
    #if HAS_AVX512F || HAS_AVX512BW
    printf("AVX-512 extensions detected.\n");
    
    #if HAS_AVX512BW
    printf("Testing V64QI (64x int8) blend...\n");
    total_checksum += test_v64qi_blend(argc);
    
    printf("Testing V32HI (32x int16) blend...\n");
    total_checksum += test_v32hi_blend(argc);
    
    printf("Testing V32HF (32x half float) blend...\n");
    total_checksum += test_v32hf_blend(argc);
    
    printf("Testing V32BF (32x bfloat16) blend...\n");
    total_checksum += test_v32bf_blend(argc);
    #endif
    
    #if HAS_AVX512F
    printf("Testing V16SI (16x int32) blend...\n");
    total_checksum += test_v16si_blend(argc);
    
    printf("Testing V8DI (8x int64) blend...\n");
    total_checksum += (int)test_v8di_blend(argc);
    
    printf("Testing V8DF (8x double) blend...\n");
    total_checksum += (int)test_v8df_blend(argc);
    
    printf("Testing V16SF (16x float) blend...\n");
    total_checksum += (int)test_v16sf_blend(argc);
    #endif
    
    printf("Total checksum: %d\n", total_checksum);
    
    #else
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with -mavx512f -mavx512bw to enable tests.\n");
    #endif
    
    /* Use argc to affect control flow */
    if (argc > 1) {
        return total_checksum & 0xFF;
    } else {
        return 0;
    }
}
