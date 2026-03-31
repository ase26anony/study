/* test_avx512_blend.c - Coverage for AVX-512 blend RTL expansion patterns */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

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

/* Alignment helper */
#define ALIGN_64 __attribute__((aligned(64)))

/* Volatile store to prevent optimization */
static inline void force_store(void* ptr, __m512i val) {
    _mm512_store_si512((__m512i*)ptr, val);
}

static inline void force_store_ps(void* ptr, __m512 val) {
    _mm512_store_ps((float*)ptr, val);
}

static inline void force_store_pd(void* ptr, __m512d val) {
    _mm512_store_pd((double*)ptr, val);
}

/* ==================== V64QI (64x int8) ==================== */
#ifdef __AVX512BW__
static uint64_t test_v64qi_blend(int argc) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 volatile int8_t dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 - i;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask: blend where src1[i] < 32 */
    __mmask64 mask = _mm512_cmplt_epi8_mask(v1, _mm512_set1_epi8(32));
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Use result in non-trivial way */
    force_store((void*)dst, blended);
    
    /* Reduction to prevent dead code elimination */
    __m512i sum_vec = _mm512_sad_epu8(blended, _mm512_setzero_si512());
    uint64_t sum = _mm512_reduce_add_epi64(sum_vec);
    
    /* Loop with argc dependency */
    for (int i = 0; i < (argc % 4 + 1); i++) {
        __m512i temp = _mm512_add_epi8(blended, _mm512_set1_epi8(i));
        __asm__ volatile("" : : "r"(temp) : "memory");
    }
    
    return sum;
}
#else
static uint64_t test_v64qi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HI (32x int16) ==================== */
#ifdef __AVX512BW__
static uint64_t test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 volatile int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = i * 200;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, _mm512_set1_epi16(1500));
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    force_store((void*)dst, blended);
    
    /* Horizontal sum */
    __m512i sum_vec = _mm512_madd_epi16(blended, _mm512_set1_epi16(1));
    uint64_t sum = _mm512_reduce_add_epi32(sum_vec);
    
    /* Side effect */
    volatile int dummy = argc;
    __asm__ volatile("" : : "r"(blended), "r"(dummy) : "memory");
    
    return sum;
}
#else
static uint64_t test_v32hi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HF (32x half precision) ==================== */
#ifdef __AVX512BW__
#include <x86intrin.h>
static uint64_t test_v32hf_blend(int argc) {
    ALIGN_64 uint16_t src1_data[32];  /* Half precision as uint16 */
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 volatile uint16_t dst[32];
    
    /* Initialize half floats */
    for (int i = 0; i < 32; i++) {
        src1_data[i] = 0x3C00 | (i & 0x1F);  /* ~1.0 with variations */
        src2_data[i] = 0x4000 | (i & 0x1F);  /* ~2.0 with variations */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2_data);
    
    /* Create mask: blend where index is even */
    __mmask32 mask = 0xAAAAAAAA;  /* 1010... pattern */
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    force_store((void*)dst, blended);
    
    /* Reduction */
    __m512i sum_vec = _mm512_sad_epu8(blended, _mm512_setzero_si512());
    uint64_t sum = _mm512_reduce_add_epi64(sum_vec);
    
    /* Loop with side effect */
    for (int i = 0; i < argc % 3; i++) {
        __m512i temp = _mm512_add_epi16(blended, _mm512_set1_epi16(i));
        __asm__ volatile("" : : "r"(temp) : "memory");
    }
    
    return sum;
}
#else
static uint64_t test_v32hf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32BF (32x bfloat16) ==================== */
#ifdef __AVX512BW__
static uint64_t test_v32bf_blend(int argc) {
    ALIGN_64 uint16_t src1_data[32];  /* BF16 as uint16 */
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 volatile uint16_t dst[32];
    
    /* Initialize bfloat16 values */
    for (int i = 0; i < 32; i++) {
        src1_data[i] = (0x3F80 + i) & 0xFFFF;  /* ~1.0f in BF16 */
        src2_data[i] = (0x4000 + i) & 0xFFFF;  /* ~2.0f in BF16 */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2_data);
    
    /* Create mask using arithmetic */
    __mmask32 mask = _mm512_cmpeq_epi16_mask(
        _mm512_and_si512(v1, _mm512_set1_epi16(1)),
        _mm512_setzero_si512()
    );
    
    /* Use integer blend for BF16 - should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    force_store((void*)dst, blended);
    
    /* Compute checksum */
    __m512i sum_vec = _mm512_madd_epi16(blended, _mm512_set1_epi16(1));
    uint64_t sum = _mm512_reduce_add_epi32(sum_vec);
    
    volatile int counter = argc;
    __asm__ volatile("" : : "r"(blended), "r"(counter) : "memory");
    
    return sum;
}
#else
static uint64_t test_v32bf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V16SI (16x int32) ==================== */
#ifdef __AVX512F__
static uint64_t test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 volatile int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask with comparison */
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, _mm512_set1_epi32(8000));
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    force_store((void*)dst, blended);
    
    /* Horizontal sum */
    __m512i sum_vec = _mm512_add_epi32(blended, _mm512_setzero_si512());
    uint64_t sum = _mm512_reduce_add_epi32(sum_vec);
    
    /* Arithmetic with blend result */
    __m512i mul = _mm512_mullo_epi32(blended, _mm512_set1_epi32(argc));
    __asm__ volatile("" : : "r"(mul) : "memory");
    
    return sum;
}
#else
static uint64_t test_v16si_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DI (8x int64) ==================== */
#ifdef __AVX512F__
static uint64_t test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 volatile int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1000000LL * i;
        src2[i] = 2000000LL * i;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask */
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, _mm512_set1_epi64(3000000));
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    force_store((void*)dst, blended);
    
    /* Reduction */
    __m512i sum_vec = _mm512_add_epi64(blended, _mm512_setzero_si512());
    uint64_t sum = _mm512_reduce_add_epi64(sum_vec);
    
    /* Loop with blend dependency */
    volatile int loop_count = argc % 5;
    for (int i = 0; i < loop_count; i++) {
        __m512i temp = _mm512_add_epi64(blended, _mm512_set1_epi64(i));
        __asm__ volatile("" : : "r"(temp) : "memory");
    }
    
    return sum;
}
#else
static uint64_t test_v8di_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DF (8x double) ==================== */
#ifdef __AVX512F__
static double test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 volatile double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1.0 * i;
        src2[i] = 2.0 * i;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask: blend where src1 > 3.0 */
    __mmask8 mask = _mm512_cmp_pd_mask(v1, _mm512_set1_pd(3.0), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    force_store_pd((void*)dst, blended);
    
    /* Reduction sum */
    __m512d sum_vec = _mm512_add_pd(blended, _mm512_setzero_pd());
    double sum = _mm512_reduce_add_pd(sum_vec);
    
    /* Arithmetic operation with blend result */
    __m512d mul = _mm512_mul_pd(blended, _mm512_set1_pd(argc + 1.0));
    __asm__ volatile("" : : "r"(mul) : "memory");
    
    return sum;
}
#else
static double test_v8df_blend(int argc) { (void)argc; return 0.0; }
#endif

/* ==================== V16SF (16x float) ==================== */
#ifdef __AVX512F__
static float test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 volatile float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = 0.5f * i;
        src2[i] = 1.5f * i;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask: blend where src1 < 4.0f */
    __mmask16 mask = _mm512_cmp_ps_mask(v1, _mm512_set1_ps(4.0f), _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    force_store_ps((void*)dst, blended);
    
    /* Horizontal sum */
    __m512 sum_vec = _mm512_add_ps(blended, _mm512_setzero_ps());
    float sum = _mm512_reduce_add_ps(sum_vec);
    
    /* Use in computation */
    __m512 scaled = _mm512_mul_ps(blended, _mm512_set1_ps(argc * 0.1f));
    __asm__ volatile("" : : "r"(scaled) : "memory");
    
    return sum;
}
#else
static float test_v16sf_blend(int argc) { (void)argc; return 0.0f; }
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char* argv[]) {
    uint64_t total_hash = 0;
    
    printf("Testing AVX-512 Blend Coverage...\n");
    
#if HAS_AVX512BW
    printf("AVX512BW enabled - testing V64QI, V32HI, V32HF, V32BF\n");
    total_hash ^= test_v64qi_blend(argc);
    total_hash ^= test_v32hi_blend(argc);
    total_hash ^= test_v32hf_blend(argc);
    total_hash ^= test_v32bf_blend(argc);
#else
    printf("AVX512BW not available\n");
#endif
    
#if HAS_AVX512F
    printf("AVX512F enabled - testing V16SI, V8DI, V8DF, V16SF\n");
    total_hash ^= test_v16si_blend(argc);
    total_hash ^= test_v8di_blend(argc);
    
    /* Handle floating point results */
    double df_result = test_v8df_blend(argc);
    float sf_result = test_v16sf_blend(argc);
    
    /* Mix floating point into integer hash */
    union { double d; uint64_t u; } du = { .d = df_result };
    union { float f; uint32_t u; } fu = { .f = sf_result };
    total_hash ^= du.u;
    total_hash ^= (uint64_t)fu.u;
#else
    printf("AVX512F not available\n");
#endif
    
    printf("Final hash: 0x%016lx\n", total_hash);
    
    /* Use result to affect return code */
    return (int)(total_hash & 0x7FFFFFFF) % 256;
}
