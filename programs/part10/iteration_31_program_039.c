/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targeting uncovered lines in i386-expand.cc: 4303-4326
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw test_avx512_blend.c -o test_avx512_blend
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Force alignment for all arrays */
#define ALIGN_64 __attribute__((aligned(64)))

/* Feature detection guards */
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

/* Prevent optimization of critical values */
static volatile int g_volatile_counter = 0;

/* ========== V64QImode (64 x 8-bit integers) ========== */
#if HAS_AVX512BW
static int test_v64qi_blend(void) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 int8_t dst[64];
    volatile ALIGN_64 int8_t volatile_dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i * 3);
        src2[i] = (int8_t)(i * 5 + 1);
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create mask: select elements where (i % 3 == 0) */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i % 3) == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Also use in computation */
    __m512i multiplied = _mm512_mullo_epi16(blended, _mm512_set1_epi8(2));
    __m512i final = _mm512_mask_blend_epi8(mask ^ 0xFFFFFFFFFFFFFFFFULL, 
                                          multiplied, blended);
    
    _mm512_store_si512((__m512i*)dst, final);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    /* Create artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum + g_volatile_counter;
}
#else
static int test_v64qi_blend(void) { return 0; }
#endif

/* ========== V32HImode (32 x 16-bit integers) ========== */
#if HAS_AVX512BW
static int test_v32hi_blend(void) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 100);
        src2[i] = (int16_t)(i * 200 + 50);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Generate mask using comparison */
    __m512i compare_val = _mm512_set1_epi16(500);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, compare_val);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(999);
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, broadcast);
    
    /* Use in loop with volatile counter */
    for (int iter = 0; iter < (g_volatile_counter & 3) + 1; iter++) {
        blended = _mm512_mask_blend_epi16(mask, blended, 
                                         _mm512_add_epi16(blended, v2));
    }
    
    _mm512_store_si512((__m512i*)dst, blended);
    
    /* Reduction */
    int64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return (int)(sum & 0x7FFFFFFF);
}
#else
static int test_v32hi_blend(void) { return 0; }
#endif

/* ========== V32HFmode (32 x half-precision floats) ========== */
#if HAS_AVX512BW
#include <x86intrin.h>
static int test_v32hf_blend(void) {
    ALIGN_64 uint16_t src1_data[32];  /* Half floats as uint16_t */
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 uint16_t dst[32];
    
    /* Initialize half floats (simple pattern) */
    for (int i = 0; i < 32; i++) {
        src1_data[i] = 0x3C00 | (i & 0x1F);  /* ~1.0 with variations */
        src2_data[i] = 0x4000 | (i & 0x1F);  /* ~2.0 with variations */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2_data);
    
    /* Create alternating mask */
    __mmask32 mask = 0xAAAAAAAA;  /* 10101010... pattern */
    
    /* Blend half floats using epi16 intrinsic */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Additional operation and blend */
    __m512i added = _mm512_add_epi16(blended, _mm512_set1_epi16(0x0400));
    blended = _mm512_mask_blend_epi16(mask ^ 0xFFFFFFFF, blended, added);
    
    _mm512_store_si512((__m512i*)dst, blended);
    
    /* Checksum */
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return (int)sum;
}
#else
static int test_v32hf_blend(void) { return 0; }
#endif

/* ========== V32BFmode (32 x bfloat16) ========== */
#if HAS_AVX512BW
static int test_v32bf_blend(void) {
    ALIGN_64 uint16_t src1[32];  /* BF16 as uint16_t */
    ALIGN_64 uint16_t src2[32];
    ALIGN_64 uint16_t dst[32];
    
    /* BF16 pattern: sign=0, exponent=127 (bias 0), mantissa variations */
    for (int i = 0; i < 32; i++) {
        src1[i] = (0x7F << 7) | (i & 0x7F);  /* ~1.0 in BF16 */
        src2[i] = (0x80 << 7) | (i & 0x7F);  /* ~2.0 in BF16 */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Mask based on LSB of data */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((src1[i] & 1) == 0) {
            mask |= (1U << i);
        }
    }
    
    /* Blend BF16 values using epi16 intrinsic */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512i shifted = _mm512_srli_epi16(blended, 1);
    blended = _mm512_mask_blend_epi16(mask ^ 0xFFFFFFFF, blended, shifted);
    
    _mm512_store_si512((__m512i*)dst, blended);
    
    /* Compute hash */
    uint32_t hash = 0;
    for (int i = 0; i < 32; i++) {
        hash = (hash << 1) ^ dst[i];
    }
    
    return (int)hash;
}
#else
static int test_v32bf_blend(void) { return 0; }
#endif

/* ========== V16SImode (16 x 32-bit integers) ========== */
#if HAS_AVX512F
static int test_v16si_blend(void) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t dst[16];
    volatile ALIGN_64 int32_t volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000 + 500;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Generate mask using comparison */
    __m512i threshold = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, threshold);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    /* Store to volatile */
    _mm512_store_epi32((void*)volatile_dst, blended);
    
    /* Use in loop with side effect */
    __m512i result = blended;
    for (int i = 0; i < (g_volatile_counter & 7) + 1; i++) {
        result = _mm512_mask_blend_epi32(mask, result,
                                        _mm512_add_epi32(result, v1));
    }
    
    _mm512_store_epi32(dst, result);
    
    /* Reduction with overflow */
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    /* Force memory barrier */
    __asm__ volatile("" : : "r"(sum), "m"(dst) : "memory");
    
    return (int)(sum & 0x7FFFFFFF);
}
#else
static int test_v16si_blend(void) { return 0; }
#endif

/* ========== V8DImode (8 x 64-bit integers) ========== */
#if HAS_AVX512F
static int test_v8di_blend(void) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1000000LL * i;
        src2[i] = 2000000LL * i + 500000;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Mask: select even elements */
    __mmask8 mask = 0x55;  /* 01010101 */
    
    /* Blend with scalar broadcast */
    __m512i scalar = _mm512_set1_epi64(9999999);
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, scalar);
    
    /* Additional blend in loop */
    for (int iter = 0; iter < 2; iter++) {
        blended = _mm512_mask_blend_epi64(mask ^ 0xFF, blended,
                                         _mm512_add_epi64(blended, v2));
    }
    
    _mm512_store_epi64(dst, blended);
    
    /* Compute checksum */
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return (int)(sum & 0x7FFFFFFF);
}
#else
static int test_v8di_blend(void) { return 0; }
#endif

/* ========== V8DFmode (8 x double-precision floats) ========== */
#if HAS_AVX512F
static int test_v8df_blend(void) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1.0 * i;
        src2[i] = 2.0 * i + 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Generate mask using floating comparison */
    __m512d threshold = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, threshold, _CMP_GT_OQ);
    
    /* Blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(1.5));
    __m512d blended = _mm512_mask_blend_pd(mask, v1, multiplied);
    
    /* Additional blend */
    __m512d added = _mm512_add_pd(blended, v2);
    blended = _mm512_mask_blend_pd(mask ^ 0xFF, blended, added);
    
    _mm512_store_pd(dst, blended);
    
    /* Reduction */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    /* Use result */
    __asm__ volatile("" : "+x"(sum) : : "memory");
    
    return (int)(sum * 1000.0);
}
#else
static int test_v8df_blend(void) { return 0; }
#endif

/* ========== V16SFmode (16 x single-precision floats) ========== */
#if HAS_AVX512F
static int test_v16sf_blend(void) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    volatile ALIGN_64 float volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = 0.1f * i;
        src2[i] = 0.2f * i + 0.05f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Complex mask generation */
    __m512 compare_val = _mm512_set1_ps(0.8f);
    __mmask16 mask1 = _mm512_cmp_ps_mask(v1, compare_val, _CMP_LT_OQ);
    __mmask16 mask2 = _mm512_cmp_ps_mask(v2, compare_val, _CMP_GT_OQ);
    __mmask16 mask = mask1 ^ mask2;
    
    /* Blend two vectors */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    /* Store to volatile */
    _mm512_store_ps((void*)volatile_dst, blended);
    
    /* Blend with operation result in loop */
    __m512 result = blended;
    for (int i = 0; i < 3; i++) {
        __m512 temp = _mm512_mul_ps(result, _mm512_set1_ps(1.1f));
        result = _mm512_mask_blend_ps(mask, result, temp);
        mask = ~mask;  /* Flip mask each iteration */
    }
    
    _mm512_store_ps(dst, result);
    
    /* Compute sum */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return (int)(sum * 1000.0f);
}
#else
static int test_v16sf_blend(void) { return 0; }
#endif

/* ========== Main driver ========== */
int main(int argc, char *argv[]) {
    int total_result = 0;
    
    printf("Testing AVX-512 blend intrinsics coverage...\n");
    
#if HAS_AVX512F || HAS_AVX512BW
    printf("AVX-512 support detected.\n");
    
    /* Initialize volatile counter based on argc */
    g_volatile_counter = argc;
    
    /* Run all blend tests */
    total_result ^= test_v64qi_blend();
    total_result ^= test_v32hi_blend();
    total_result ^= test_v32hf_blend();
    total_result ^= test_v32bf_blend();
    total_result ^= test_v16si_blend();
    total_result ^= test_v8di_blend();
    total_result ^= test_v8df_blend();
    total_result ^= test_v16sf_blend();
    
    printf("All blend tests completed. Result hash: %d\n", total_result);
    
    /* Use result to affect control flow */
    if (total_result != 0) {
        printf("Non-zero result indicates active computation.\n");
    }
    
    return total_result & 0xFF;
#else
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
    return 1;
#endif
}
