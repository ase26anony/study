/* test_avx512_blend.c
 * 
 * This program specifically targets the AVX-512 blend RTL expansion patterns
 * in GCC's i386 backend (i386-expand.cc lines 4303-4326).
 * It uses AVX-512 blend intrinsics with masks to trigger the compiler's
 * internal switch statement for each vector mode.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Feature guards to prevent compilation errors */
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
static inline void force_store_volatile(void* dest, __m512i src) {
    _mm512_store_si512((__m512i*)dest, src);
}

static inline void force_store_volatile_ps(void* dest, __m512 src) {
    _mm512_store_ps((float*)dest, src);
}

static inline void force_store_volatile_pd(void* dest, __m512d src) {
    _mm512_store_pd((double*)dest, src);
}

/* ==================== V64QImode (64 x 8-bit integers) ==================== */
#if HAS_AVX512BW
static uint64_t test_v64qi_blend(void) {
    ALIGN_64 uint8_t src1[64];
    ALIGN_64 uint8_t src2[64];
    ALIGN_64 uint8_t result[64];
    volatile ALIGN_64 uint8_t volatile_result[64];
    
    /* Initialize with pattern data */
    for (int i = 0; i < 64; i++) {
        src1[i] = i * 3;
        src2[i] = i * 5 + 1;
    }
    
    /* Load vectors */
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask: select from v1 where (i % 3 == 0), else from v2 */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i % 3) == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* This intrinsic should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v2, v1);
    
    /* Store to volatile to prevent optimization */
    force_store_volatile((void*)volatile_result, blended);
    
    /* Also store normally for reduction */
    _mm512_store_si512((__m512i*)result, blended);
    
    /* Compute checksum to ensure live computation */
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    
    /* Create artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}
#endif

/* ==================== V32HImode (32 x 16-bit integers) ==================== */
#if HAS_AVX512BW
static uint64_t test_v32hi_blend(void) {
    ALIGN_64 uint16_t src1[32];
    ALIGN_64 uint16_t src2[32];
    ALIGN_64 uint16_t result[32];
    volatile ALIGN_64 uint16_t volatile_result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 7;
        src2[i] = i * 11 + 2;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison intrinsic */
    __m512i cmp_val = _mm512_set1_epi16(100);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Blend with broadcasted scalar */
    __m512i broadcast = _mm512_set1_epi16(0xABCD);
    __m512i blended = _mm512_mask_blend_epi16(mask, broadcast, v1);
    
    /* Store to volatile */
    force_store_volatile((void*)volatile_result, blended);
    _mm512_store_si512((__m512i*)result, blended);
    
    /* Use in reduction */
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}
#endif

/* ==================== V32HFmode (32 x half-precision floats) ==================== */
#if HAS_AVX512BW && defined(__AVX512FP16__)
static float test_v32hf_blend(void) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 result[32];
    volatile ALIGN_64 _Float16 volatile_result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 1.5f;
        src2[i] = i * 2.5f + 1.0f;
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask: select where src1 > 20.0 */
    __m512h threshold = _mm512_set1_ph(20.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, threshold, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v2, v1);
    
    /* Store results */
    _mm512_store_ph((void*)volatile_result, blended);
    _mm512_store_ph(result, blended);
    
    /* Reduction */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#endif

/* ==================== V32BFmode (32 x bfloat16) ==================== */
#if HAS_AVX512BW && defined(__AVX512BF16__)
static float test_v32bf_blend(void) {
    ALIGN_64 __bfloat16 src1[32];
    ALIGN_64 __bfloat16 src2[32];
    ALIGN_64 __bfloat16 result[32];
    volatile ALIGN_64 __bfloat16 volatile_result[32];
    
    /* Initialize bfloat16 values */
    for (int i = 0; i < 32; i++) {
        src1[i] = bfloat16_from_float(i * 1.2f);
        src2[i] = bfloat16_from_float(i * 2.3f + 0.5f);
    }
    
    /* Load as integers since blend intrinsics work on integer representation */
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create alternating mask pattern */
    __mmask32 mask = 0xAAAAAAAA; /* 10101010... pattern */
    
    /* Use epi16 blend on bfloat16 data */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Store results */
    force_store_volatile((void*)volatile_result, blended);
    _mm512_store_si512((__m512i*)result, blended);
    
    /* Reduction */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(result[i]);
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#endif

/* ==================== V16SImode (16 x 32-bit integers) ==================== */
#if HAS_AVX512F
static uint64_t test_v16si_blend(void) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t result[16];
    volatile ALIGN_64 int32_t volatile_result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 13;
        src2[i] = i * 17 + 3;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(50);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Blend with arithmetic result */
    __m512i added = _mm512_add_epi32(v1, _mm512_set1_epi32(100));
    __m512i blended = _mm512_mask_blend_epi32(mask, added, v2);
    
    /* Store to volatile */
    force_store_volatile((void*)volatile_result, blended);
    _mm512_store_si512((__m512i*)result, blended);
    
    /* Reduction with loop dependency */
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#endif

/* ==================== V8DImode (8 x 64-bit integers) ==================== */
#if HAS_AVX512F
static uint64_t test_v8di_blend(void) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t result[8];
    volatile ALIGN_64 int64_t volatile_result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 23LL;
        src2[i] = i * 29LL + 5;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask: select where index is even */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((i % 2) == 0) {
            mask |= (1 << i);
        }
    }
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v2, v1);
    
    /* Store results */
    force_store_volatile((void*)volatile_result, blended);
    _mm512_store_si512((__m512i*)result, blended);
    
    /* Reduction */
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#endif

/* ==================== V8DFmode (8 x double precision) ==================== */
#if HAS_AVX512F
static double test_v8df_blend(void) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double result[8];
    volatile ALIGN_64 double volatile_result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.7;
        src2[i] = i * 2.9 + 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __m512d threshold = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, threshold, _CMP_GT_OQ);
    
    /* Blend with multiplied result */
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(2.0));
    __m512d blended = _mm512_mask_blend_pd(mask, multiplied, v2);
    
    /* Store results */
    force_store_volatile_pd((void*)volatile_result, blended);
    _mm512_store_pd(result, blended);
    
    /* Reduction */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#endif

/* ==================== V16SFmode (16 x single precision) ==================== */
#if HAS_AVX512F
static float test_v16sf_blend(void) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float result[16];
    volatile ALIGN_64 float volatile_result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1.1f;
        src2[i] = i * 2.2f + 0.3f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 threshold = _mm512_set1_ps(8.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, threshold, _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v2, v1);
    
    /* Store results with volatile */
    force_store_volatile_ps((void*)volatile_result, blended);
    _mm512_store_ps(result, blended);
    
    /* Reduction in loop with artificial dependency */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char** argv) {
    uint64_t total_checksum = 0;
    
    printf("Testing AVX-512 blend pattern coverage...\n");
    
    /* Use argc to create runtime variability in loop counts */
    int loop_count = (argc > 1) ? (argc % 5) + 1 : 3;
    
    /* Run tests multiple times to ensure coverage in loops */
    for (int iter = 0; iter < loop_count; iter++) {
        #if HAS_AVX512BW
        total_checksum += test_v64qi_blend();
        total_checksum += test_v32hi_blend();
        
        #ifdef __AVX512FP16__
        total_checksum += (uint64_t)test_v32hf_blend();
        #endif
        
        #ifdef __AVX512BF16__
        total_checksum += (uint64_t)test_v32bf_blend();
        #endif
        #endif
        
        #if HAS_AVX512F
        total_checksum += test_v16si_blend();
        total_checksum += test_v8di_blend();
        total_checksum += (uint64_t)test_v8df_blend();
        total_checksum += (uint64_t)test_v16sf_blend();
        #endif
    }
    
    printf("Total checksum: %lu\n", total_checksum);
    
    #if !HAS_AVX512F && !HAS_AVX512BW
    printf("AVX-512 not supported on this platform. Compile with -mavx512f -mavx512bw\n");
    #endif
    
    return (int)(total_checksum % 256);
}
