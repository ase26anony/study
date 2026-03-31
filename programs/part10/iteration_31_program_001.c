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

/* Test function for V64QI mode (requires AVX512BW) */
#if HAS_AVX512BW
static uint64_t test_v64qi_blend(void) {
    ALIGN_64 uint8_t src1[64];
    ALIGN_64 uint8_t src2[64];
    ALIGN_64 uint8_t result[64];
    volatile ALIGN_64 uint8_t volatile_result[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 255 - i;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask: blend where src1[i] < 128 */
    __mmask64 mask = _mm512_cmplt_epu8_mask(v1, _mm512_set1_epi8(128));
    
    /* Blend operation - should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v2, v1);
    
    /* Store to volatile to prevent optimization */
    force_store((void*)volatile_result, blended);
    
    /* Also store to regular array */
    _mm512_store_si512((__m512i*)result, blended);
    
    /* Compute checksum */
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    
    /* Use result in asm to create dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}
#endif

/* Test function for V32HI mode (requires AVX512BW) */
#if HAS_AVX512BW
static uint64_t test_v32hi_blend(void) {
    ALIGN_64 uint16_t src1[32];
    ALIGN_64 uint16_t src2[32];
    ALIGN_64 uint16_t result[32];
    volatile ALIGN_64 uint16_t volatile_result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = 30000 - i * 50;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask using comparison */
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, _mm512_set1_epi16(1500));
    
    /* Blend operation - should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Additional operation: blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(9999);
    __m512i blended2 = _mm512_mask_blend_epi16(mask, blended, broadcast);
    
    force_store((void*)volatile_result, blended2);
    _mm512_store_si512((__m512i*)result, blended2);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#endif

/* Test function for V32HF mode (requires AVX512BW) */
#if HAS_AVX512BW && defined(__AVX512FP16__)
static float test_v32hf_blend(void) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 result[32];
    volatile ALIGN_64 _Float16 volatile_result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.5f);
        src2[i] = (_Float16)(10.0f - i * 0.25f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask: blend where src1 > 5.0 */
    __mmask32 mask = _mm512_cmp_ph_mask(v1, _mm512_set1_ph(5.0f), _CMP_GT_OQ);
    
    /* Blend operation - should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v2, v1);
    
    force_store((void*)volatile_result, (_Float16*)blended);
    _mm512_store_ph(result, blended);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#endif

/* Test function for V32BF mode (bfloat16, requires AVX512BW) */
#if HAS_AVX512BW && defined(__AVX512BF16__)
static float test_v32bf_blend(void) {
    ALIGN_64 __bfloat16 src1[32];
    ALIGN_64 __bfloat16 src2[32];
    ALIGN_64 __bfloat16 result[32];
    volatile ALIGN_64 __bfloat16 volatile_result[32];
    
    /* Initialize bfloat16 values */
    for (int i = 0; i < 32; i++) {
        float f1 = i * 0.7f;
        float f2 = 15.0f - i * 0.3f;
        src1[i] = (__bfloat16)f1;
        src2[i] = (__bfloat16)f2;
    }
    
    /* Load as integers for blending */
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask based on comparison of integer representation */
    __mmask32 mask = _mm512_cmplt_epi16_mask(v1, _mm512_set1_epi16(0x4000)); /* 2.0 in bfloat16 */
    
    /* Blend using epi16 intrinsic - should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    force_store((void*)volatile_result, blended);
    _mm512_store_si512((__m512i*)result, blended);
    
    /* Compute sum as float */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#endif

/* Test function for V16SI mode (requires AVX512F) */
#if HAS_AVX512F
static uint64_t test_v16si_blend(void) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t result[16];
    volatile ALIGN_64 int32_t volatile_result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = 50000 - i * 2000;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, _mm512_set1_epi32(8000));
    
    /* Blend operation - should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v2, v1);
    
    /* Blend with arithmetic result */
    __m512i added = _mm512_add_epi32(v1, _mm512_set1_epi32(100));
    __m512i blended2 = _mm512_mask_blend_epi32(mask, blended, added);
    
    force_store((void*)volatile_result, blended2);
    _mm512_store_epi32(result, blended2);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#endif

/* Test function for V8DI mode (requires AVX512F) */
#if HAS_AVX512F
static uint64_t test_v8di_blend(void) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t result[8];
    volatile ALIGN_64 int64_t volatile_result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1000000LL * i;
        src2[i] = 10000000LL - 500000LL * i;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask */
    __mmask8 mask = _mm512_cmplt_epi64_mask(v1, _mm512_set1_epi64(3000000));
    
    /* Blend operation - should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v2, v1);
    
    force_store((void*)volatile_result, blended);
    _mm512_store_epi64(result, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#endif

/* Test function for V8DF mode (requires AVX512F) */
#if HAS_AVX512F
static double test_v8df_blend(void) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double result[8];
    volatile ALIGN_64 double volatile_result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = 10.0 - i * 0.7;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask: blend where src1 > 4.0 */
    __mmask8 mask = _mm512_cmp_pd_mask(v1, _mm512_set1_pd(4.0), _CMP_GT_OQ);
    
    /* Blend operation - should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v2, v1);
    
    /* Additional blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(2.0));
    __m512d blended2 = _mm512_mask_blend_pd(mask, blended, multiplied);
    
    force_store_pd((void*)volatile_result, blended2);
    _mm512_store_pd(result, blended2);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#endif

/* Test function for V16SF mode (requires AVX512F) */
#if HAS_AVX512F
static float test_v16sf_blend(void) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float result[16];
    volatile ALIGN_64 float volatile_result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.3f;
        src2[i] = 5.0f - i * 0.2f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using multiple conditions */
    __mmask16 mask1 = _mm512_cmp_ps_mask(v1, _mm512_set1_ps(2.0f), _CMP_LT_OQ);
    __mmask16 mask2 = _mm512_cmp_ps_mask(v1, _mm512_set1_ps(4.0f), _CMP_GT_OQ);
    __mmask16 mask = mask1 | mask2;
    
    /* Blend operation - should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v2, v1);
    
    /* Use in loop with argc-dependent iteration */
    int iterations = 3; /* Will be overridden by main's argc */
    for (int iter = 0; iter < iterations; iter++) {
        __m512 temp = _mm512_add_ps(blended, _mm512_set1_ps(iter * 0.1f));
        blended = _mm512_mask_blend_ps(mask, blended, temp);
    }
    
    force_store_ps((void*)volatile_result, blended);
    _mm512_store_ps(result, blended);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#endif

/* Main driver function */
int main(int argc, char* argv[]) {
    uint64_t total_checksum = 0;
    
    printf("Testing AVX-512 blend intrinsics...\n");
    
#if HAS_AVX512F || HAS_AVX512BW
    printf("AVX-512 extensions detected.\n");
    
    /* Use argc to create runtime variability */
    int loop_count = (argc > 1) ? atoi(argv[1]) % 5 + 1 : 2;
    
    /* Run tests multiple times with loop dependency */
    for (int i = 0; i < loop_count; i++) {
        
#if HAS_AVX512BW
        printf("Testing V64QI blend...\n");
        total_checksum += test_v64qi_blend();
        
        printf("Testing V32HI blend...\n");
        total_checksum += test_v32hi_blend();
        
#if defined(__AVX512FP16__)
        printf("Testing V32HF blend...\n");
        total_checksum += (uint64_t)test_v32hf_blend();
#endif
        
#if defined(__AVX512BF16__)
        printf("Testing V32BF blend...\n");
        total_checksum += (uint64_t)test_v32bf_blend();
#endif
#endif /* HAS_AVX512BW */
        
#if HAS_AVX512F
        printf("Testing V16SI blend...\n");
        total_checksum += test_v16si_blend();
        
        printf("Testing V8DI blend...\n");
        total_checksum += test_v8di_blend();
        
        printf("Testing V8DF blend...\n");
        total_checksum += (uint64_t)test_v8df_blend();
        
        printf("Testing V16SF blend...\n");
        total_checksum += (uint64_t)test_v16sf_blend();
#endif /* HAS_AVX512F */
    }
    
    printf("Total checksum: %lu\n", total_checksum);
    
#else
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with -mavx512f -mavx512bw for full coverage.\n");
#endif
    
    return (int)(total_checksum % 256);
}
