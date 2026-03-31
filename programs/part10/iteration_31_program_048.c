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

/* Prevent optimization */
static volatile int g_volatile_counter = 0;

/* ==================== V64QI (64x int8) ==================== */
#if HAS_AVX512BW
static int test_v64qi_blend(int argc) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 int8_t dst[64];
    volatile ALIGN_64 int8_t volatile_dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i * 3);
        src2[i] = (int8_t)(i * 5 + 1);
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask: select elements where (i % 3 == 0) */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i % 3) == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* Critical blend that should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Use in arithmetic to prevent optimization */
    __m512i multiplied = _mm512_mullo_epi16(_mm512_and_si512(blended, _mm512_set1_epi8(0x7F)), 
                                           _mm512_set1_epi8(2));
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi8(0x40);
    __m512i final_blend = _mm512_mask_blend_epi8(mask ^ 0xFFFFFFFFFFFFFFFFULL, multiplied, broadcast);
    
    /* Store with volatile to force side effect */
    _mm512_store_si512((__m512i*)volatile_dst, final_blend);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += volatile_dst[i];
    }
    
    /* Loop with argc dependency */
    int loop_count = (argc > 1) ? 100 : 10;
    for (int i = 0; i < loop_count; i++) {
        __m512i temp = _mm512_mask_blend_epi8(mask, v1, v2);
        _mm512_store_si512((__m512i*)dst, temp);
        sum += dst[i % 64];
    }
    
    return sum & 0xFF;
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
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi16(300);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Use in reduction */
    __m512i sum_vec = _mm512_add_epi16(blended, _mm512_set1_epi16(1));
    
    /* Blend with arithmetic result */
    __m512i multiplied = _mm512_mullo_epi16(sum_vec, _mm512_set1_epi16(3));
    __m512i final = _mm512_mask_blend_epi16(mask, blended, multiplied);
    
    _mm512_store_si512((__m512i*)dst, final);
    
    /* Compute reduction */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum & 0xFFFF;
}
#else
static int test_v32hi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HF (32x half precision) ==================== */
#if HAS_AVX512BW
#include <x86intrin.h>
static int test_v32hf_blend(int argc) {
    ALIGN_64 uint16_t src1_data[32];  /* Half precision as uint16 */
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 uint16_t dst[32];
    
    /* Initialize half-precision pattern */
    for (int i = 0; i < 32; i++) {
        /* Simple pattern: 1.0, 2.0, 3.0, ... */
        src1_data[i] = 0x3C00 + (i & 0x7);  /* 1.0 + small increment */
        src2_data[i] = 0x4000 + (i & 0x7);  /* 2.0 + small increment */
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((__m512i*)src2_data);
    
    /* Create mask: alternating pattern */
    __mmask32 mask = 0xAAAAAAAA;  /* 10101010... pattern */
    
    /* For half precision, we need to use _mm512_mask_blend_ph */
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512i blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    /* Store result */
    _mm512_store_si512((__m512i*)dst, blended);
    
    /* Use in loop with argc */
    int sum = 0;
    int iterations = (argc > 2) ? 50 : 20;
    for (int i = 0; i < iterations; i++) {
        __m512i temp = _mm512_mask_blend_ph(mask ^ (1 << (i % 32)), v1, v2);
        _mm512_store_si512((__m512i*)dst, temp);
        sum += dst[i % 32];
    }
    
    return sum & 0x7FFF;
}
#else
static int test_v32hf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32BF (32x bfloat16) ==================== */
#if HAS_AVX512BW
static int test_v32bf_blend(int argc) {
    ALIGN_64 uint16_t src1_data[32];  /* BF16 as uint16 */
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 uint16_t dst[32];
    
    /* Initialize bfloat16 pattern */
    for (int i = 0; i < 32; i++) {
        /* BF16 representation of small floats */
        src1_data[i] = (uint16_t)(0x4000 + i);  /* ~2.0 + epsilon */
        src2_data[i] = (uint16_t)(0x4040 + i);  /* ~3.0 + epsilon */
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((__m512i*)src2_data);
    
    /* Create mask using comparison on integer representation */
    __m512i threshold = _mm512_set1_epi16(0x4080);  /* ~4.0 in BF16 */
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, threshold);
    
    /* BF16 uses same blend as int16 - should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Additional blend with broadcast */
    __m512i broadcast = _mm512_set1_epi16(0x3F80);  /* ~1.0 in BF16 */
    __m512i final = _mm512_mask_blend_epi16(mask ^ 0xFFFFFFFF, blended, broadcast);
    
    _mm512_store_si512((__m512i*)dst, final);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    /* Memory barrier */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum & 0xFFFF;
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
    volatile ALIGN_64 int32_t volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000 + 500;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    /* Use in arithmetic chain */
    __m512i added = _mm512_add_epi32(blended, _mm512_set1_epi32(1));
    __m512i multiplied = _mm512_mullo_epi32(added, _mm512_set1_epi32(3));
    
    /* Blend again with different mask */
    __mmask16 mask2 = _mm512_cmpeq_epi32_mask(_mm512_and_si512(v1, _mm512_set1_epi32(1)), 
                                             _mm512_setzero_si512());
    __m512i final = _mm512_mask_blend_epi32(mask2, multiplied, blended);
    
    _mm512_store_si512((__m512i*)volatile_dst, final);
    
    /* Loop with blend inside */
    int sum = 0;
    int loop_count = (argc > 3) ? 100 : 30;
    for (int i = 0; i < loop_count; i++) {
        __m512i temp = _mm512_mask_blend_epi32(mask ^ (1 << (i % 16)), v1, v2);
        _mm512_store_si512((__m512i*)dst, temp);
        sum += dst[i % 16];
    }
    
    return sum;
}
#else
static int test_v16si_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DI (8x int64) ==================== */
#if HAS_AVX512F
static int test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = i * 20000LL + 5000;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask */
    __m512i cmp_val = _mm512_set1_epi64(30000);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, cmp_val);
    
    /* Should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512i shifted = _mm512_slli_epi64(blended, 1);
    __m512i final = _mm512_mask_blend_epi64(mask ^ 0xFF, blended, shifted);
    
    _mm512_store_si512((__m512i*)dst, final);
    
    /* Reduction */
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return (int)(sum & 0x7FFFFFFF);
}
#else
static int test_v8di_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DF (8x double) ==================== */
#if HAS_AVX512F
static int test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 2.5 + 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using floating comparison */
    __m512d cmp_val = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(blended, _mm512_set1_pd(2.0));
    __m512d final = _mm512_mask_blend_pd(mask ^ 0xFF, blended, multiplied);
    
    _mm512_store_pd(dst, final);
    
    /* Use in loop */
    double sum = 0.0;
    int iterations = (argc > 4) ? 40 : 15;
    for (int i = 0; i < iterations; i++) {
        __m512d temp = _mm512_mask_blend_pd(mask ^ (1 << (i % 8)), v1, v2);
        _mm512_store_pd(dst, temp);
        sum += dst[i % 8];
    }
    
    return (int)(sum * 100.0);
}
#else
static int test_v8df_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V16SF (16x float) ==================== */
#if HAS_AVX512F
static int test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    volatile ALIGN_64 float volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 1.5f + 0.25f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using floating comparison */
    __m512 cmp_val = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    /* Complex blend chain */
    __m512 added = _mm512_add_ps(blended, _mm512_set1_ps(1.0f));
    __m512 multiplied = _mm512_mul_ps(added, _mm512_set1_ps(0.5f));
    
    /* Blend with broadcast scalar */
    __m512 broadcast = _mm512_set1_ps(10.0f);
    __m512 final = _mm512_mask_blend_ps(mask, multiplied, broadcast);
    
    _mm512_store_ps(volatile_dst, final);
    
    /* Loop with argc-dependent iterations */
    float sum = 0.0f;
    int loop_count = (argc > 5) ? 60 : 25;
    for (int i = 0; i < loop_count; i++) {
        __m512 temp = _mm512_mask_blend_ps(mask ^ (1 << (i % 16)), v1, v2);
        _mm512_store_ps(dst, temp);
        sum += dst[i % 16];
    }
    
    return (int)(sum * 10.0f);
}
#else
static int test_v16sf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char *argv[]) {
    int total_result = 0;
    
#if HAS_AVX512F || HAS_AVX512BW
    printf("Testing AVX-512 blend patterns...\n");
    
    /* Seed random for reproducible results */
    srand(42);
    
    /* Call all test functions */
    total_result ^= test_v64qi_blend(argc);
    total_result ^= test_v32hi_blend(argc);
    total_result ^= test_v32hf_blend(argc);
    total_result ^= test_v32bf_blend(argc);
    total_result ^= test_v16si_blend(argc);
    total_result ^= test_v8di_blend(argc);
    total_result ^= test_v8df_blend(argc);
    total_result ^= test_v16sf_blend(argc);
    
    /* Use volatile to prevent optimization of final result */
    g_volatile_counter = total_result;
    
    printf("Result: %d (0x%08x)\n", total_result, total_result);
    
    /* Additional loop to ensure blends are used */
    for (int i = 0; i < (argc > 1 ? atoi(argv[1]) % 100 : 10); i++) {
        total_result += i;
    }
    
    return total_result & 0xFF;
#else
    printf("AVX-512 not supported on this platform\n");
    return 0;
#endif
}
