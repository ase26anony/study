/* test_avx512_blend.c - Coverage for AVX-512 blend RTL patterns in i386-expand.cc */
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

/* Volatile store to prevent optimization */
static inline void force_store(void* addr, __m512i val) {
    _mm512_store_si512((__m512i*)addr, val);
}

static inline void force_store_ps(void* addr, __m512 val) {
    _mm512_store_ps((float*)addr, val);
}

static inline void force_store_pd(void* addr, __m512d val) {
    _mm512_store_pd((double*)addr, val);
}

/* ==================== V64QI (64x int8) ==================== */
#ifdef __AVX512BW__
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
    force_store((void*)volatile_dst, blended);
    
    /* Use in reduction */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += volatile_dst[i];
    }
    
    /* Loop with argc dependency */
    int loop_count = (argc > 1) ? 100 : 10;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512i temp = _mm512_add_epi8(v1, _mm512_set1_epi8(iter));
        blended = _mm512_mask_blend_epi8(mask, blended, temp);
        force_store((void*)dst, blended);
    }
    
    /* Final reduction */
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    return sum & 0xFF; /* Return non-zero result */
}
#endif

/* ==================== V32HI (32x int16) ==================== */
#ifdef __AVX512BW__
static int test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t dst[32];
    volatile ALIGN_64 int16_t volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 100);
        src2[i] = (int16_t)(i * 200 - 50);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i threshold = _mm512_set1_epi16(1000);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, threshold);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(0x7FFF);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, broadcast, v1);
    
    force_store((void*)volatile_dst, blended);
    
    /* Use in arithmetic chain */
    __m512i added = _mm512_add_epi16(v1, v2);
    blended = _mm512_mask_blend_epi16(mask, blended, added);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += volatile_dst[i];
        sum += ((int16_t*)&blended)[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum & 0xFFFF;
}
#endif

/* ==================== V32HF (32x half precision) ==================== */
#ifdef __AVX512BW__
#ifdef __AVX512FP16__
static int test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 dst[32];
    volatile ALIGN_64 _Float16 volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f - 1.0f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask: select where src1 > 10.0 */
    __m512h ten = _mm512_set1_ph((_Float16)10.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, ten, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v2, v1);
    
    _mm512_store_ph((_Float16*)volatile_dst, blended);
    
    /* Reduction */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += volatile_dst[i];
    }
    
    /* Blend with arithmetic result */
    __m512h multiplied = _mm512_mul_ph(v1, v2);
    blended = _mm512_mask_blend_ph(mask, blended, multiplied);
    _mm512_store_ph(dst, blended);
    
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return (int)sum;
}
#endif
#endif

/* ==================== V32BF (32x bfloat16) ==================== */
#ifdef __AVX512BW__
#ifdef __AVX512BF16__
static int test_v32bf_blend(int argc) {
    ALIGN_64 __bfloat16 src1[32];
    ALIGN_64 __bfloat16 src2[32];
    ALIGN_64 __bfloat16 dst[32];
    
    /* Initialize bfloat16 values */
    for (int i = 0; i < 32; i++) {
        float f1 = i * 0.5f;
        float f2 = i * 0.75f + 0.25f;
        src1[i] = (__bfloat16)f1;
        src2[i] = (__bfloat16)f2;
    }
    
    /* Load as integers for blending */
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using integer comparison */
    __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
    
    /* Blend using epi16 intrinsic (bfloat16 uses 16-bit lanes)
       This should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Store and compute checksum */
    _mm512_store_si512((__m512i*)dst, blended);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)dst[i];
    }
    
    return sum & 0xFFFF;
}
#endif
#endif

/* ==================== V16SI (16x int32) ==================== */
#ifdef __AVX512F__
static int test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t dst[16];
    volatile ALIGN_64 int32_t volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000 - 500;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __m512i threshold = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, threshold);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v2, v1);
    
    _mm512_store_epi32((void*)volatile_dst, blended);
    
    /* Use in loop with argc dependency */
    int loop_count = argc * 10 + 1;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512i temp = _mm512_add_epi32(v1, _mm512_set1_epi32(iter));
        blended = _mm512_mask_blend_epi32(mask, blended, temp);
    }
    
    _mm512_store_epi32(dst, blended);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += volatile_dst[i];
        sum += dst[i];
    }
    
    return sum;
}
#endif

/* ==================== V8DI (8x int64) ==================== */
#ifdef __AVX512F__
static long long test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (int64_t)i * 1000000LL;
        src2[i] = (int64_t)i * 2000000LL - 500000LL;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask: select where i % 2 == 0 */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((i % 2) == 0) {
            mask |= (1 << i);
        }
    }
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v2, v1);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi64(0x7FFFFFFFFFFFFFFFLL);
    blended = _mm512_mask_blend_epi64(mask, blended, broadcast);
    
    _mm512_store_epi64(dst, blended);
    
    /* Reduction */
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    /* Memory barrier */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}
#endif

/* ==================== V8DF (8x double) ==================== */
#ifdef __AVX512F__
static double test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    volatile ALIGN_64 double volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 2.5 - 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using floating comparison */
    __m512d threshold = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, threshold, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v2, v1);
    
    _mm512_store_pd(volatile_dst, blended);
    
    /* Blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(v1, v2);
    blended = _mm512_mask_blend_pd(mask, blended, multiplied);
    
    _mm512_store_pd(dst, blended);
    
    /* Reduction */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += volatile_dst[i];
        sum += dst[i];
    }
    
    return sum;
}
#endif

/* ==================== V16SF (16x float) ==================== */
#ifdef __AVX512F__
static float test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    volatile ALIGN_64 float volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 0.75f + 0.25f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 threshold = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, threshold, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v2, v1);
    
    _mm512_store_ps(volatile_dst, blended);
    
    /* Use in loop with side effects */
    int loop_count = (argc > 0) ? argc : 5;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512 temp = _mm512_add_ps(v1, _mm512_set1_ps(iter * 0.1f));
        blended = _mm512_mask_blend_ps(mask, blended, temp);
    }
    
    _mm512_store_ps(dst, blended);
    
    /* Reduction with volatile access */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += volatile_dst[i];
        sum += dst[i];
    }
    
    return sum;
}
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char** argv) {
    int total_result = 0;
    
    printf("Testing AVX-512 blend patterns...\n");
    
#if HAS_AVX512BW
    printf("AVX512BW supported\n");
#ifdef __AVX512BW__
    total_result += test_v64qi_blend(argc);
    printf("V64QI blend test completed\n");
    
    total_result += test_v32hi_blend(argc);
    printf("V32HI blend test completed\n");
    
#ifdef __AVX512FP16__
    total_result += test_v32hf_blend(argc);
    printf("V32HF blend test completed\n");
#else
    printf("V32HF blend test skipped (AVX512-FP16 not enabled)\n");
#endif
    
#ifdef __AVX512BF16__
    total_result += test_v32bf_blend(argc);
    printf("V32BF blend test completed\n");
#else
    printf("V32BF blend test skipped (AVX512-BF16 not enabled)\n");
#endif
#endif
#else
    printf("AVX512BW not supported\n");
#endif
    
#if HAS_AVX512F
    printf("AVX512F supported\n");
#ifdef __AVX512F__
    total_result += test_v16si_blend(argc);
    printf("V16SI blend test completed\n");
    
    total_result += (int)test_v8di_blend(argc);
    printf("V8DI blend test completed\n");
    
    total_result += (int)test_v8df_blend(argc);
    printf("V8DF blend test completed\n");
    
    total_result += (int)test_v16sf_blend(argc);
    printf("V16SF blend test completed\n");
#endif
#else
    printf("AVX512F not supported\n");
#endif
    
    printf("Total checksum: %d\n", total_result);
    
    /* Use result to affect return code */
    return (total_result != 0) ? 0 : 1;
}
