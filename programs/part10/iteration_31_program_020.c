/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targets uncovered lines 4303-4326 in i386-expand.cc
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Feature guards for AVX-512 extensions */
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

#ifdef __AVX512VL__
#define HAS_AVX512VL 1
#else
#define HAS_AVX512VL 0
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

/* ==================== V64QI Mode (AVX512BW) ==================== */
#if HAS_AVX512BW
static int test_v64qi_blend(int argc) {
    ALIGN_64 uint8_t src1[64];
    ALIGN_64 uint8_t src2[64];
    ALIGN_64 uint8_t dst[64];
    volatile ALIGN_64 uint8_t volatile_dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (uint8_t)(i * 3);
        src2[i] = (uint8_t)(i * 5 + 1);
    }
    
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    /* Create mask using comparison - elements where src1[i] < 128 */
    __mmask64 mask = _mm512_cmplt_epu8_mask(v1, _mm512_set1_epi8(128));
    
    /* Blend with mask - this should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store to volatile to prevent optimization */
    force_store((void*)volatile_dst, blended);
    
    /* Also use in computation */
    __m512i sum = _mm512_add_epi8(blended, _mm512_set1_epi8(1));
    
    /* Loop with argc dependency */
    int loop_count = (argc > 1) ? 64 : 32;
    __m512i accum = _mm512_setzero_si512();
    
    for (int i = 0; i < loop_count; i++) {
        __m512i temp = _mm512_mask_blend_epi8(mask, 
            _mm512_set1_epi8(i),
            _mm512_set1_epi8(i * 2));
        accum = _mm512_add_epi8(accum, temp);
    }
    
    /* Compute checksum */
    _mm512_store_si512(dst, accum);
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += dst[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(checksum) : "memory");
    
    return checksum & 0xFF;
}
#else
static int test_v64qi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HI Mode (AVX512BW) ==================== */
#if HAS_AVX512BW
static int test_v32hi_blend(int argc) {
    ALIGN_64 uint16_t src1[32];
    ALIGN_64 uint16_t src2[32];
    ALIGN_64 uint16_t dst[32];
    volatile ALIGN_64 uint16_t volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (uint16_t)(i * 100);
        src2[i] = (uint16_t)(i * 200 + 50);
    }
    
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    /* Create mask - select where src1[i] < 1600 */
    __mmask32 mask = _mm512_cmplt_epi16_mask(v1, _mm512_set1_epi16(1600));
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    force_store((void*)volatile_dst, blended);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(999);
    __m512i blended2 = _mm512_mask_blend_epi16(mask, blended, broadcast);
    
    /* Use in arithmetic */
    __m512i multiplied = _mm512_mullo_epi16(blended2, _mm512_set1_epi16(2));
    
    /* Loop with side effects */
    int result = 0;
    int iterations = (argc > 2) ? 100 : 50;
    
    for (int i = 0; i < iterations; i++) {
        __m512i temp = _mm512_mask_blend_epi16(
            mask,
            multiplied,
            _mm512_set1_epi16(i)
        );
        _mm512_store_si512(dst, temp);
        result += dst[i % 32];
    }
    
    __asm__ volatile("" : : "r"(result) : "memory");
    return result & 0xFFFF;
}
#else
static int test_v32hi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HF Mode (AVX512BW) ==================== */
#if HAS_AVX512BW && defined(__AVX512FP16__)
static int test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 dst[32];
    volatile ALIGN_64 _Float16 volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f + 0.5f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask using comparison */
    __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_LT_OQ);
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_ph((void*)volatile_dst, blended);
    
    /* Reduction */
    __m512h sum = _mm512_setzero_ph();
    int loop = (argc > 3) ? 32 : 16;
    
    for (int i = 0; i < loop; i++) {
        __m512h temp = _mm512_mask_blend_ph(
            mask,
            blended,
            _mm512_set1_ph((_Float16)(i * 0.1f))
        );
        sum = _mm512_add_ph(sum, temp);
    }
    
    _mm512_store_ph(dst, sum);
    int checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (int)(dst[i] * 100);
    }
    
    __asm__ volatile("" : : "r"(checksum) : "memory");
    return checksum;
}
#else
static int test_v32hf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32BF Mode (AVX512BW) ==================== */
#if HAS_AVX512BW && defined(__AVX512BF16__)
#include <x86intrin.h>
static int test_v32bf_blend(int argc) {
    ALIGN_64 __bfloat16 src1[32];
    ALIGN_64 __bfloat16 src2[32];
    ALIGN_64 __bfloat16 dst[32];
    volatile ALIGN_64 __bfloat16 volatile_dst[32];
    
    /* Initialize bfloat16 data */
    for (int i = 0; i < 32; i++) {
        src1[i] = (__bfloat16)(i * 1.25f);
        src2[i] = (__bfloat16)(i * 2.75f + 0.25f);
    }
    
    /* Load as epi16 since bfloat16 is stored as 16-bit integers */
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    /* Create mask - compare as 16-bit integers */
    __mmask32 mask = _mm512_cmplt_epi16_mask(v1, _mm512_set1_epi16(0x4000)); /* ~2.0 in bfloat16 */
    
    /* Blend using epi16 intrinsic - should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((void*)volatile_dst, blended);
    
    /* Use in computation */
    __m512i result = _mm512_setzero_si512();
    int iterations = (argc > 4) ? 64 : 32;
    
    for (int i = 0; i < iterations; i++) {
        __m512i temp = _mm512_mask_blend_epi16(
            mask,
            blended,
            _mm512_set1_epi16(i * 100)
        );
        result = _mm512_add_epi16(result, temp);
    }
    
    _mm512_store_si512(dst, result);
    int checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += dst[i];
    }
    
    __asm__ volatile("" : : "r"(checksum) : "memory");
    return checksum & 0xFFFF;
}
#else
static int test_v32bf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V16SI Mode (AVX512F) ==================== */
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
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask */
    __mmask16 mask = _mm512_cmplt_epi32_mask(v1, _mm512_set1_epi32(8000));
    
    /* Blend - should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_store_epi32((void*)volatile_dst, blended);
    
    /* Blend with arithmetic result */
    __m512i added = _mm512_add_epi32(v1, _mm512_set1_epi32(100));
    __m512i blended2 = _mm512_mask_blend_epi32(mask, blended, added);
    
    /* Reduction */
    int sum = 0;
    int loop = (argc > 5) ? 100 : 50;
    
    for (int i = 0; i < loop; i++) {
        __m512i temp = _mm512_mask_blend_epi32(
            mask,
            blended2,
            _mm512_set1_epi32(i)
        );
        _mm512_store_epi32(dst, temp);
        sum += dst[i % 16];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#else
static int test_v16si_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DI Mode (AVX512F) ==================== */
#if HAS_AVX512F
static long test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    volatile ALIGN_64 int64_t volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = i * 20000LL + 5000LL;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask */
    __mmask8 mask = _mm512_cmplt_epi64_mask(v1, _mm512_set1_epi64(40000));
    
    /* Blend - should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_epi64((void*)volatile_dst, blended);
    
    /* Complex pattern with loop */
    long result = 0;
    int iterations = (argc > 6) ? 40 : 20;
    
    for (int i = 0; i < iterations; i++) {
        __m512i temp1 = _mm512_mask_blend_epi64(
            mask,
            blended,
            _mm512_set1_epi64(i * 1000LL)
        );
        
        __m512i temp2 = _mm512_add_epi64(temp1, _mm512_set1_epi64(1));
        
        __m512i final = _mm512_mask_blend_epi64(mask, temp1, temp2);
        
        _mm512_store_epi64(dst, final);
        result += dst[i % 8];
    }
    
    __asm__ volatile("" : : "r"(result) : "memory");
    return result;
}
#else
static long test_v8di_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DF Mode (AVX512F) ==================== */
#if HAS_AVX512F
static double test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    volatile ALIGN_64 double volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.1;
        src2[i] = i * 2.2 + 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask */
    __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_store_pd(volatile_dst, blended);
    
    /* Blend with arithmetic operation */
    __m512d multiplied = _mm512_mul_pd(blended, _mm512_set1_pd(1.5));
    __m512d blended2 = _mm512_mask_blend_pd(mask, blended, multiplied);
    
    /* Reduction */
    __m512d sum = _mm512_setzero_pd();
    int loop = (argc > 7) ? 25 : 12;
    
    for (int i = 0; i < loop; i++) {
        __m512d temp = _mm512_mask_blend_pd(
            mask,
            blended2,
            _mm512_set1_pd(i * 0.25)
        );
        sum = _mm512_add_pd(sum, temp);
    }
    
    _mm512_store_pd(dst, sum);
    double result = 0.0;
    for (int i = 0; i < 8; i++) {
        result += dst[i];
    }
    
    __asm__ volatile("" : : "r"(result) : "memory");
    return result;
}
#else
static double test_v8df_blend(int argc) { (void)argc; return 0.0; }
#endif

/* ==================== V16SF Mode (AVX512F) ==================== */
#if HAS_AVX512F
static float test_v16sf_blend(int argc) {
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
    
    /* Create mask */
    __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_store_ps(volatile_dst, blended);
    
    /* Multiple blend operations */
    __m512 added = _mm512_add_ps(blended, _mm512_set1_ps(10.0f));
    __m512 blended2 = _mm512_mask_blend_ps(mask, blended, added);
    
    __m512 multiplied = _mm512_mul_ps(blended2, _mm512_set1_ps(2.0f));
    __m512 blended3 = _mm512_mask_blend_ps(mask, blended2, multiplied);
    
    /* Loop with argc-dependent iterations */
    __m512 accum = _mm512_setzero_ps();
    int iterations = (argc > 8) ? 30 : 15;
    
    for (int i = 0; i < iterations; i++) {
        __m512 temp = _mm512_mask_blend_ps(
            mask,
            blended3,
            _mm512_set1_ps(i * 0.1f)
        );
        accum = _mm512_add_ps(accum, temp);
    }
    
    _mm512_store_ps(dst, accum);
    float result = 0.0f;
    for (int i = 0; i < 16; i++) {
        result += dst[i];
    }
    
    __asm__ volatile("" : : "r"(result) : "memory");
    return result;
}
#else
static float test_v16sf_blend(int argc) { (void)argc; return 0.0f; }
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char** argv) {
    int total_checksum = 0;
    
    printf("Testing AVX-512 blend intrinsics coverage...\n");
    
    #if HAS_AVX512BW || HAS_AVX512F
    printf("AVX-512 extensions detected:\n");
    #if HAS_AVX512F
    printf("  AVX512F enabled\n");
    #endif
    #if HAS_AVX512BW
    printf("  AVX512BW enabled\n");
    #endif
    
    /* Call all test functions with argc for control flow variation */
    total_checksum += test_v64qi_blend(argc);
    total_checksum += test_v32hi_blend(argc);
    total_checksum += test_v32hf_blend(argc);
    total_checksum += test_v32bf_blend(argc);
    total_checksum += test_v16si_blend(argc);
    
    long di_result = test_v8di_blend(argc);
    total_checksum += (int)(di_result & 0xFFFFFFFF);
    
    double df_result = test_v8df_blend(argc);
    total_checksum += (int)(df_result * 1000);
    
    float sf_result = test_v16sf_blend(argc);
    total_checksum += (int)(sf_result * 1000);
    
    printf("Total checksum: %d\n", total_checksum);
    
    #else
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
    #endif
    
    /* Use argc to affect control flow */
    if (argc > 1) {
        return total_checksum % 255;
    } else {
        return 0;
    }
}
