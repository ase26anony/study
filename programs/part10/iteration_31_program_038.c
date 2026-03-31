/* test_avx512_blend.c - Coverage for i386-expand.cc lines 4303-4326 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Alignment for AVX-512 vectors */
#define ALIGN_64 __attribute__((aligned(64)))

/* Feature guards */
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

/* ==================== V64QI (64x int8) ==================== */
#if HAS_AVX512BW
static int test_v64qi_blend(int argc) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 int8_t dst[64];
    volatile ALIGN_64 int8_t volatile_dst[64]; /* Prevent optimization */
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i * 3);
        src2[i] = (int8_t)(i * 5 + 1);
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create blend mask: select from v1 where i % 2 == 0 */
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
    
    /* Use in reduction */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += volatile_dst[i];
    }
    
    /* Loop with argc dependency */
    int loop_count = (argc > 1) ? 100 : 10;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512i temp = _mm512_set1_epi8(iter);
        blended = _mm512_mask_blend_epi8(mask, blended, temp);
        _mm512_store_si512((__m512i*)dst, blended);
        
        /* Artificial dependency */
        __asm__ volatile("" : : "r"(dst[0]) : "memory");
    }
    
    return sum + dst[0];
}
#else
static int test_v64qi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HI (32x int16) ==================== */
#if HAS_AVX512BW
static int test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 100);
        src2[i] = (int16_t)(i * 200 + 50);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i compare_val = _mm512_set1_epi16(1600);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, compare_val);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(999);
    blended = _mm512_mask_blend_epi16(0xAAAAAAAA, blended, broadcast);
    
    _mm512_store_si512((__m512i*)dst, blended);
    
    /* Reduction with loop */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    /* argc-dependent computation */
    if (argc > 2) {
        __m512i add_result = _mm512_add_epi16(blended, _mm512_set1_epi16(argc));
        blended = _mm512_mask_blend_epi16(0x55555555, blended, add_result);
        _mm512_store_si512((__m512i*)dst, blended);
    }
    
    return sum + dst[0];
}
#else
static int test_v32hi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HF (32x half precision) ==================== */
#if HAS_AVX512BW
#include <x86intrin.h> /* For _Float16 if available */
static int test_v32hf_blend(int argc) {
    ALIGN_64 uint16_t src1_data[32]; /* Store as uint16 for _Float16 */
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 uint16_t dst_data[32];
    
    /* Initialize with simple pattern */
    for (int i = 0; i < 32; i++) {
        src1_data[i] = (uint16_t)(i * 0x0400); /* Approx i * 1.0 in FP16 */
        src2_data[i] = (uint16_t)(i * 0x0800 + 0x0400); /* i * 2.0 + 1.0 */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2_data);
    
    /* Create alternating mask */
    __mmask32 mask = 0xAAAAAAAA;
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)dst_data, blended);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)dst_data[i];
    }
    
    (void)argc; /* Mark used */
    return sum;
}
#else
static int test_v32hf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32BF (32x bfloat16) ==================== */
#if HAS_AVX512BW
static int test_v32bf_blend(int argc) {
    ALIGN_64 uint16_t src1_data[32]; /* BF16 stored as uint16 */
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 uint16_t dst_data[32];
    
    /* BF16 pattern */
    for (int i = 0; i < 32; i++) {
        /* Simple BF16 values (exponent 127, mantissa based on i) */
        src1_data[i] = (uint16_t)(0x3F00 + (i & 0x7F));
        src2_data[i] = (uint16_t)(0x4000 + (i & 0x7F));
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2_data);
    
    /* Mask from comparison */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (src1_data[i] > src2_data[i]) {
            mask |= (1U << i);
        }
    }
    
    /* Use integer blend for BF16 - should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Blend with arithmetic result */
    __m512i added = _mm512_add_epi16(v1, _mm512_set1_epi16(0x0100));
    blended = _mm512_mask_blend_epi16(0x55555555, blended, added);
    
    _mm512_store_si512((__m512i*)dst_data, blended);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)dst_data[i];
    }
    
    (void)argc;
    return sum;
}
#else
static int test_v32bf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V16SI (16x int32) ==================== */
#if HAS_AVX512F
static int test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000 + 500;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison intrinsic */
    __m512i compare = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, compare);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v2, v1);
    
    /* Store to volatile array */
    volatile ALIGN_64 int32_t volatile_dst[16];
    _mm512_store_epi32((void*)volatile_dst, blended);
    
    /* Use in argc-dependent loop */
    int result = 0;
    int iterations = (argc > 0) ? argc * 2 : 5;
    
    for (int i = 0; i < iterations; i++) {
        __m512i temp = _mm512_set1_epi32(i);
        blended = _mm512_mask_blend_epi32(0xAAAA, blended, temp);
        
        /* Force side effect */
        __asm__ volatile("" : : "r"(blended) : "memory");
    }
    
    _mm512_store_epi32(dst, blended);
    
    for (int i = 0; i < 16; i++) {
        result += dst[i] + volatile_dst[i];
    }
    
    return result;
}
#else
static int test_v16si_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DI (8x int64) ==================== */
#if HAS_AVX512F
static long long test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (int64_t)i * 10000LL;
        src2[i] = (int64_t)i * 20000LL + 5000LL;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask - select even indices */
    __mmask8 mask = 0xAA; /* 0b10101010 */
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v2, v1);
    
    /* Blend with arithmetic result */
    __m512i multiplied = _mm512_mullo_epi64(v1, _mm512_set1_epi64(3));
    blended = _mm512_mask_blend_epi64(0x55, blended, multiplied);
    
    _mm512_store_epi64(dst, blended);
    
    /* Compute sum */
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    /* argc-dependent additional blend */
    if (argc > 1) {
        __m512i arg_vec = _mm512_set1_epi64(argc);
        blended = _mm512_mask_blend_epi64(0xF0, blended, arg_vec);
        _mm512_store_epi64(dst, blended);
    }
    
    return sum + dst[0];
}
#else
static long long test_v8di_blend(int argc) { (void)argc; return 0LL; }
#endif

/* ==================== V8DF (8x double) ==================== */
#if HAS_AVX512F
static double test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (double)i * 1.5;
        src2[i] = (double)i * 2.5 + 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using floating comparison */
    __m512d threshold = _mm512_set1_pd(6.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, threshold, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v2, v1);
    
    /* Blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(1.5));
    blended = _mm512_mask_blend_pd(0xAA, blended, multiplied);
    
    _mm512_store_pd(dst, blended);
    
    /* Reduction sum */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    /* Loop with volatile store */
    volatile ALIGN_64 double volatile_store[8];
    for (int i = 0; i < (argc % 4 + 1); i++) {
        __m512d temp = _mm512_set1_pd((double)i);
        blended = _mm512_mask_blend_pd(0x0F, blended, temp);
        _mm512_store_pd((double*)volatile_store, blended);
    }
    
    return sum + volatile_store[0];
}
#else
static double test_v8df_blend(int argc) { (void)argc; return 0.0; }
#endif

/* ==================== V16SF (16x float) ==================== */
#if HAS_AVX512F
static float test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = (float)i * 0.5f;
        src2[i] = (float)i * 1.5f + 0.25f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask: select where src1 > src2 */
    __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v2, v1);
    
    /* Multiple blend operations */
    __m512 added = _mm512_add_ps(v1, _mm512_set1_ps(10.0f));
    blended = _mm512_mask_blend_ps(0xAAAA, blended, added);
    
    __m512 multiplied = _mm512_mul_ps(v2, _mm512_set1_ps(2.0f));
    blended = _mm512_mask_blend_ps(0x5555, blended, multiplied);
    
    _mm512_store_ps(dst, blended);
    
    /* Compute sum with argc dependency */
    float sum = 0.0f;
    int elements = (argc > 0) ? 16 : 8;
    for (int i = 0; i < elements; i++) {
        sum += dst[i];
    }
    
    /* Additional volatile operation */
    volatile ALIGN_64 float volatile_array[16];
    _mm512_store_ps((float*)volatile_array, blended);
    
    return sum + volatile_array[0];
}
#else
static float test_v16sf_blend(int argc) { (void)argc; return 0.0f; }
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char *argv[]) {
    int total_result = 0;
    
    printf("Testing AVX-512 blend pattern coverage...\n");
    
#if HAS_AVX512F || HAS_AVX512BW
    printf("AVX-512 extensions detected.\n");
    
    /* Call all test functions and accumulate results */
    total_result += test_v64qi_blend(argc);
    total_result += test_v32hi_blend(argc);
    total_result += test_v32hf_blend(argc);
    total_result += test_v32bf_blend(argc);
    total_result += test_v16si_blend(argc);
    
    long long di_result = test_v8di_blend(argc);
    total_result += (int)(di_result & 0xFFFFFFFF) + (int)(di_result >> 32);
    
    double df_result = test_v8df_blend(argc);
    total_result += (int)df_result;
    
    float sf_result = test_v16sf_blend(argc);
    total_result += (int)sf_result;
    
    printf("Total checksum: %d\n", total_result);
    
    /* Use result to affect return code */
    return (total_result != 0) ? 0 : 1;
#else
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
    return 2;
#endif
}
