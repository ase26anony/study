/* test_avx512_blend.c - Coverage for AVX-512 blend patterns in i386-expand.cc */
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
static inline void force_store(void* dest, __m512i val) {
    _mm512_store_si512((__m512i*)dest, val);
}

static inline void force_store_ps(void* dest, __m512 val) {
    _mm512_store_ps((float*)dest, val);
}

static inline void force_store_pd(void* dest, __m512d val) {
    _mm512_store_pd((double*)dest, val);
}

/* ==================== V64QI (64 x int8_t) ==================== */
#ifdef __AVX512BW__
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
    
    /* Create mask: select from v1 where (i % 3 == 0) */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i % 3) == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* The key intrinsic that should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v2, v1);
    
    /* Use result in non-trivial way */
    _mm512_store_si512((__m512i*)result, blended);
    
    /* Store to volatile to prevent optimization */
    force_store((void*)volatile_result, blended);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    
    /* Loop with argc dependency */
    int loop_count = (argc > 1) ? 100 : 10;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512i temp = _mm512_add_epi8(v1, _mm512_set1_epi8(iter));
        blended = _mm512_mask_blend_epi8(mask, blended, temp);
        _mm512_store_si512((__m512i*)result, blended);
    }
    
    return sum + loop_count;
}
#endif

/* ==================== V32HI (32 x int16_t) ==================== */
#ifdef __AVX512BW__
static int test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 100);
        src2[i] = (int16_t)(i * 200 - 50);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i compare_val = _mm512_set1_epi16(500);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, compare_val);
    
    /* Should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(999);
    blended = _mm512_mask_blend_epi16(0xAAAAAAAA, blended, broadcast);
    
    _mm512_store_si512((__m512i*)result, blended);
    
    /* Reduction */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}
#endif

/* ==================== V32HF (32 x _Float16) ==================== */
#ifdef __AVX512BW__
#ifdef __AVX512FP16__
static int test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f - 1.0f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask */
    __m512h threshold = _mm512_set1_ph(10.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, threshold, _CMP_GT_OQ);
    
    /* Should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v2, v1);
    
    /* Additional blend with arithmetic result */
    __m512h multiplied = _mm512_mul_ph(v1, _mm512_set1_ph(2.0f));
    blended = _mm512_mask_blend_ph(0x55555555, blended, multiplied);
    
    _mm512_store_ph(result, blended);
    
    /* Compute checksum */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    
    return (int)sum;
}
#endif
#endif

/* ==================== V32BF (32 x bfloat16) ==================== */
#ifdef __AVX512BW__
#ifdef __AVX512BF16__
static int test_v32bf_blend(int argc) {
    ALIGN_64 __bfloat16 src1[32];
    ALIGN_64 __bfloat16 src2[32];
    ALIGN_64 __bfloat16 result[32];
    
    /* Initialize bfloat16 values */
    for (int i = 0; i < 32; i++) {
        src1[i] = (__bfloat16)(i * 1.25f);
        src2[i] = (__bfloat16)(i * 2.75f);
    }
    
    /* Load as integers for blending */
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask - blend every other element */
    __mmask32 mask = 0xAAAAAAAA;  /* 1010... pattern */
    
    /* Use integer blend on bfloat16 data - should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)result, blended);
    
    /* Compute simple checksum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        /* Convert to float for checksum */
        float val;
        memcpy(&val, &result[i], sizeof(__bfloat16));
        sum += (int)(val * 100);
    }
    
    return sum;
}
#endif
#endif

/* ==================== V16SI (16 x int32_t) ==================== */
#ifdef __AVX512F__
static int test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000 - 500;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i compare_val = _mm512_set1_epi32(5000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, compare_val);
    
    /* Should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v2, v1);
    
    /* Blend with arithmetic result */
    __m512i added = _mm512_add_epi32(v1, _mm512_set1_epi32(100));
    blended = _mm512_mask_blend_epi32(0xAAAA, blended, added);
    
    _mm512_store_si512((__m512i*)result, blended);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    return sum;
}
#endif

/* ==================== V8DI (8 x int64_t) ==================== */
#ifdef __AVX512F__
static int test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = i * 20000LL - 5000LL;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask */
    __m512i compare_val = _mm512_set1_epi64(30000LL);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, compare_val);
    
    /* Should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)result, blended);
    
    /* Reduction with overflow */
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    return (int)(sum & 0x7FFFFFFF);
}
#endif

/* ==================== V8DF (8 x double) ==================== */
#ifdef __AVX512F__
static int test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 2.5 - 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __m512d threshold = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, threshold, _CMP_GT_OQ);
    
    /* Should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v2, v1);
    
    /* Blend with multiplied result */
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(1.5));
    blended = _mm512_mask_blend_pd(0xAA, blended, multiplied);
    
    _mm512_store_pd(result, blended);
    
    /* Compute checksum */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    return (int)(sum * 100.0);
}
#endif

/* ==================== V16SF (16 x float) ==================== */
#ifdef __AVX512F__
static int test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1.1f;
        src2[i] = i * 2.2f - 0.5f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 threshold = _mm512_set1_ps(8.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, threshold, _CMP_GT_OQ);
    
    /* Should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v2, v1);
    
    /* Blend with arithmetic result in loop */
    int loop_count = (argc > 2) ? 50 : 20;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512 temp = _mm512_add_ps(v1, _mm512_set1_ps(iter * 0.1f));
        blended = _mm512_mask_blend_ps(0x5555, blended, temp);
    }
    
    _mm512_store_ps(result, blended);
    
    /* Reduction */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    return (int)(sum * 100.0f);
}
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char** argv) {
    int total_checksum = 0;
    
    printf("Testing AVX-512 blend patterns...\n");
    
#if HAS_AVX512F || HAS_AVX512BW
    printf("AVX-512 extensions detected.\n");
    
    /* Seed RNG for reproducible results */
    srand(42);
    
#ifdef __AVX512BW__
    printf("Testing V64QI blend...\n");
    total_checksum += test_v64qi_blend(argc);
    
    printf("Testing V32HI blend...\n");
    total_checksum += test_v32hi_blend(argc);
    
#ifdef __AVX512FP16__
    printf("Testing V32HF blend...\n");
    total_checksum += test_v32hf_blend(argc);
#endif
    
#ifdef __AVX512BF16__
    printf("Testing V32BF blend...\n");
    total_checksum += test_v32bf_blend(argc);
#endif
#endif /* __AVX512BW__ */
    
#ifdef __AVX512F__
    printf("Testing V16SI blend...\n");
    total_checksum += test_v16si_blend(argc);
    
    printf("Testing V8DI blend...\n");
    total_checksum += test_v8di_blend(argc);
    
    printf("Testing V8DF blend...\n");
    total_checksum += test_v8df_blend(argc);
    
    printf("Testing V16SF blend...\n");
    total_checksum += test_v16sf_blend(argc);
#endif /* __AVX512F__ */
    
    printf("Total checksum: %d\n", total_checksum);
    
#else
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
#endif
    
    /* Use result to prevent dead code elimination */
    if (total_checksum == 0) {
        printf("Warning: All blends produced zero checksum\n");
    }
    
    return total_checksum & 0xFF;
}
