/* test_avx512_blend.c - Coverage test for AVX-512 blend RTL patterns */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

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

/* Volatile memory barrier to prevent optimization */
#define DO_NOT_OPTIMIZE(value) \
    __asm__ volatile("" : : "r"(value) : "memory")

/* ==================== V64QI (64 x int8_t) ==================== */
#if HAS_AVX512BW
static int test_v64qi_blend(int argc) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 int8_t dst[64];
    volatile int8_t volatile_dst[64] ALIGN_64;
    
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
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Also use in computation */
    __m512i scaled = _mm512_add_epi8(blended, _mm512_set1_epi8(10));
    __m512i result = _mm512_mask_blend_epi8(mask ^ 0xFFFFFFFFFFFFFFFFULL, 
                                           scaled, blended);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    /* Loop with argc dependency */
    int loop_count = (argc > 1) ? (argc % 10) + 1 : 5;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512i temp = _mm512_load_si512((const __m512i*)src1);
        __m512i temp2 = _mm512_load_si512((const __m512i*)src2);
        __m512i temp_blend = _mm512_mask_blend_epi8(mask, temp2, temp);
        _mm512_store_si512((__m512i*)volatile_dst, temp_blend);
        DO_NOT_OPTIMIZE(volatile_dst[0]);
    }
    
    return sum & 0xFF;
}
#else
static int test_v64qi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HI (32 x int16_t) ==================== */
#if HAS_AVX512BW
static int test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t dst[32];
    volatile int16_t volatile_dst[32] ALIGN_64;
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 100);
        src2[i] = (int16_t)(i * 200 - 50);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i compare_val = _mm512_set1_epi16(1000);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, compare_val);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(42);
    __m512i result = _mm512_mask_blend_epi16(mask, blended, broadcast);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    /* Reduction */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    /* Loop with side effect */
    if (argc > 0) {
        __m512i temp = _mm512_mask_blend_epi16(mask, v1, v2);
        DO_NOT_OPTIMIZE(temp);
    }
    
    return sum & 0xFFFF;
}
#else
static int test_v32hi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HF (32 x _Float16) ==================== */
#if HAS_AVX512BW
#include <float.h>
static int test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 dst[32];
    volatile _Float16 volatile_dst[32] ALIGN_64;
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f - 1.0f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask using float comparison */
    __m512h threshold = _mm512_set1_ph(10.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, threshold, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v2, v1);
    
    _mm512_store_ph((void*)volatile_dst, blended);
    
    /* Blend with arithmetic result */
    __m512h added = _mm512_add_ph(v1, v2);
    __m512h result = _mm512_mask_blend_ph(mask, blended, added);
    
    _mm512_store_ph(dst, result);
    
    /* Compute checksum */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)dst[i];
    }
    
    /* Loop with argc */
    for (int i = 0; i < (argc % 3) + 1; i++) {
        __m512h temp = _mm512_mask_blend_ph(mask, v1, v2);
        DO_NOT_OPTIMIZE(temp);
    }
    
    return (int)sum;
}
#else
static int test_v32hf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32BF (32 x bfloat16) ==================== */
#if HAS_AVX512BW
static int test_v32bf_blend(int argc) {
    ALIGN_64 uint16_t src1[32];  /* bfloat16 as uint16_t */
    ALIGN_64 uint16_t src2[32];
    ALIGN_64 uint16_t dst[32];
    volatile uint16_t volatile_dst[32] ALIGN_64;
    
    /* Initialize bfloat16 patterns */
    for (int i = 0; i < 32; i++) {
        /* Simple bfloat16 pattern: sign=0, exponent=127 (1.0), mantissa=i */
        src1[i] = (uint16_t)(0x3F80 + (i & 0x7F));
        src2[i] = (uint16_t)(0x4000 + (i & 0x7F)); /* 2.0 + small variation */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask: select where i % 3 == 0 */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i % 3) == 0) {
            mask |= (1U << i);
        }
    }
    
    /* Use integer blend for bfloat16 - should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Additional blend operation */
    __m512i pattern = _mm512_set1_epi16(0x3F80); /* bfloat16 1.0 */
    __m512i result = _mm512_mask_blend_epi16(mask, blended, pattern);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    /* Checksum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    /* Loop with side effects */
    volatile int loop = argc;
    while (loop-- > 0) {
        __m512i temp = _mm512_mask_blend_epi16(mask, v1, v2);
        DO_NOT_OPTIMIZE(temp);
    }
    
    return sum;
}
#else
static int test_v32bf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V16SI (16 x int32_t) ==================== */
#if HAS_AVX512F
static int test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t dst[16];
    volatile int32_t volatile_dst[16] ALIGN_64;
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000 - 500;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i compare_val = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, compare_val);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Blend with arithmetic operation */
    __m512i multiplied = _mm512_mullo_epi32(v1, _mm512_set1_epi32(2));
    __m512i result = _mm512_mask_blend_epi32(mask, blended, multiplied);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    /* Reduction */
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    /* Loop with argc dependency */
    for (int i = 0; i < (argc % 5) + 2; i++) {
        __m512i temp = _mm512_mask_blend_epi32(mask, v1, v2);
        DO_NOT_OPTIMIZE(temp);
    }
    
    return (int)(sum & 0x7FFFFFFF);
}
#else
static int test_v16si_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DI (8 x int64_t) ==================== */
#if HAS_AVX512F
static int test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    volatile int64_t volatile_dst[8] ALIGN_64;
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (int64_t)i * 10000LL;
        src2[i] = (int64_t)i * 20000LL - 5000LL;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask: select where i % 2 == 1 */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((i % 2) == 1) {
            mask |= (1U << i);
        }
    }
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi64(999999);
    __m512i result = _mm512_mask_blend_epi64(mask, blended, broadcast);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    /* Checksum */
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    /* Loop */
    volatile int loops = argc + 1;
    while (loops--) {
        __m512i temp = _mm512_mask_blend_epi64(mask, v2, v1);
        DO_NOT_OPTIMIZE(temp);
    }
    
    return (int)(sum & 0x7FFFFFFF);
}
#else
static int test_v8di_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DF (8 x double) ==================== */
#if HAS_AVX512F
static int test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    volatile double volatile_dst[8] ALIGN_64;
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (double)i * 1.1;
        src2[i] = (double)i * 2.2 - 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __m512d threshold = _mm512_set1_pd(4.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, threshold, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v2, v1);
    
    _mm512_store_pd(volatile_dst, blended);
    
    /* Blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(1.5));
    __m512d result = _mm512_mask_blend_pd(mask, blended, multiplied);
    
    _mm512_store_pd(dst, result);
    
    /* Reduction */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    /* Loop with side effects */
    for (int i = 0; i < (argc % 4) + 1; i++) {
        __m512d temp = _mm512_mask_blend_pd(mask, v1, v2);
        DO_NOT_OPTIMIZE(temp);
    }
    
    return (int)(sum * 100.0);
}
#else
static int test_v8df_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V16SF (16 x float) ==================== */
#if HAS_AVX512F
static int test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    volatile float volatile_dst[16] ALIGN_64;
    
    for (int i = 0; i < 16; i++) {
        src1[i] = (float)i * 0.5f;
        src2[i] = (float)i * 1.5f - 0.25f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 threshold = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, threshold, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v2, v1);
    
    _mm512_store_ps(volatile_dst, blended);
    
    /* Blend with multiple sources */
    __m512 added = _mm512_add_ps(v1, v2);
    __m512 result = _mm512_mask_blend_ps(mask, blended, added);
    
    _mm512_store_ps(dst, result);
    
    /* Reduction */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    /* Complex loop with blend inside */
    volatile int counter = argc;
    while (counter-- > 0) {
        __m512 temp1 = _mm512_load_ps(src1);
        __m512 temp2 = _mm512_load_ps(src2);
        __m512 temp_blend = _mm512_mask_blend_ps(mask, temp2, temp1);
        DO_NOT_OPTIMIZE(temp_blend);
    }
    
    return (int)(sum * 100.0f);
}
#else
static int test_v16sf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char *argv[]) {
    int total_result = 0;
    
    printf("Testing AVX-512 blend intrinsics coverage...\n");
    
#if HAS_AVX512F || HAS_AVX512BW
    printf("AVX-512 extensions detected.\n");
    
    /* Call all test functions */
    total_result ^= test_v64qi_blend(argc);
    total_result ^= test_v32hi_blend(argc);
    total_result ^= test_v32hf_blend(argc);
    total_result ^= test_v32bf_blend(argc);
    total_result ^= test_v16si_blend(argc);
    total_result ^= test_v8di_blend(argc);
    total_result ^= test_v8df_blend(argc);
    total_result ^= test_v16sf_blend(argc);
    
    printf("All blend tests completed. Final hash: %d\n", total_result);
    
    /* Additional verification loop */
    volatile int verify = total_result;
    for (int i = 0; i < (argc % 3); i++) {
        verify = (verify * 31) ^ i;
    }
    
    return verify & 0xFF;
#else
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
    return 1;
#endif
}
