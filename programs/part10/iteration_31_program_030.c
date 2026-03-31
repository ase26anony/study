/* Test program to trigger AVX-512 blend RTL expansion patterns in i386-expand.cc
   Specifically targets lines 4303-4326 covering various vector modes */
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Feature guards for compilation on non-AVX-512 systems */
#ifdef __AVX512F__
#ifdef __AVX512BW__

/* Helper function to create masks from patterns */
static __mmask64 create_mask64(uint64_t pattern) {
    return _mm512_int2mask(pattern);
}

static __mmask32 create_mask32(uint32_t pattern) {
    return _mm512_int2mask(pattern);
}

static __mmask16 create_mask16(uint16_t pattern) {
    return _mm512_int2mask(pattern);
}

/* ========== V64QImode (64-byte integers) ========== */
__attribute__((noinline))
int test_v64qi_blend(int seed) {
    /* Aligned arrays for 64-byte vectors */
    __attribute__((aligned(64))) int8_t src1[64];
    __attribute__((aligned(64))) int8_t src2[64];
    __attribute__((aligned(64))) int8_t result[64];
    volatile int8_t* volatile_result = result; /* Prevent optimization */
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i + seed);
        src2[i] = (int8_t)(i - seed);
    }
    
    /* Load vectors */
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create alternating mask: 0xAAAAAAAAAAAAAAAA for 64 bits */
    __mmask64 mask = create_mask64(0xAAAAAAAAAAAAAAAAULL);
    
    /* Blend using intrinsic - should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store with volatile pointer to prevent optimization */
    _mm512_store_epi32((void*)volatile_result, blended);
    
    /* Use result in computation */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    
    /* Artificial dependency to prevent dead code elimination */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

/* ========== V32HImode (32 half-word integers) ========== */
__attribute__((noinline))
int test_v32hi_blend(int seed) {
    __attribute__((aligned(64))) int16_t src1[32];
    __attribute__((aligned(64))) int16_t src2[32];
    __attribute__((aligned(64))) int16_t result[32];
    volatile int16_t* volatile_result = result;
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 3 + seed);
        src2[i] = (int16_t)(i * 5 - seed);
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create checkerboard mask: 0x55555555 for 32 bits */
    __mmask32 mask = create_mask32(0x55555555);
    
    /* Blend - should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_epi32((void*)volatile_result, blended);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* ========== V32HFmode (32 half-precision floats) ========== */
__attribute__((noinline))
int test_v32hf_blend(int seed) {
    __attribute__((aligned(64))) uint16_t src1[32]; /* _Float16 as uint16_t */
    __attribute__((aligned(64))) uint16_t src2[32];
    __attribute__((aligned(64))) uint16_t result[32];
    volatile uint16_t* volatile_result = result;
    
    for (int i = 0; i < 32; i++) {
        /* Simple half-float pattern */
        src1[i] = (uint16_t)((i + seed) & 0x7FFF);
        src2[i] = (uint16_t)((i - seed) & 0x7FFF);
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Mask with alternating pattern */
    __mmask32 mask = create_mask32(0x33333333);
    
    /* Blend using integer intrinsic for half-float */
    /* Should trigger gen_avx512bw_blendmv32hf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_epi32((void*)volatile_result, blended);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* ========== V32BFmode (32 bfloat16 floats) ========== */
__attribute__((noinline))
int test_v32bf_blend(int seed) {
    __attribute__((aligned(64))) uint16_t src1[32]; /* bfloat16 as uint16_t */
    __attribute__((aligned(64))) uint16_t src2[32];
    __attribute__((aligned(64))) uint16_t result[32];
    volatile uint16_t* volatile_result = result;
    
    for (int i = 0; i < 32; i++) {
        /* bfloat16 pattern */
        src1[i] = (uint16_t)(((i + seed) << 8) & 0x7F80);
        src2[i] = (uint16_t)(((i - seed) << 8) & 0x7F80);
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Mask with pattern */
    __mmask32 mask = create_mask32(0xCCCCCCCC);
    
    /* Blend using integer intrinsic for bfloat16 */
    /* Should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_epi32((void*)volatile_result, blended);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ========== AVX512F-only tests ========== */
#ifdef __AVX512F__

/* ========== V16SImode (16 single-word integers) ========== */
__attribute__((noinline))
int test_v16si_blend(int seed) {
    __attribute__((aligned(64))) int32_t src1[16];
    __attribute__((aligned(64))) int32_t src2[16];
    __attribute__((aligned(64))) int32_t result[16];
    volatile int32_t* volatile_result = result;
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 7 + seed;
        src2[i] = i * 11 - seed;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask: alternating groups of 4 */
    __mmask16 mask = create_mask16(0xF0F0);
    
    /* Blend - should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_store_epi32((void*)volatile_result, blended);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* ========== V8DImode (8 double-word integers) ========== */
__attribute__((noinline))
int test_v8di_blend(int seed) {
    __attribute__((aligned(64))) int64_t src1[8];
    __attribute__((aligned(64))) int64_t src2[8];
    __attribute__((aligned(64))) int64_t result[8];
    volatile int64_t* volatile_result = result;
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (int64_t)i * 13 + seed;
        src2[i] = (int64_t)i * 17 - seed;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Mask: alternating pattern */
    __mmask8 mask = create_mask16(0xAA) & 0xFF;
    
    /* Blend - should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_epi32((void*)volatile_result, blended);
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (int)result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* ========== V8DFmode (8 double-precision floats) ========== */
__attribute__((noinline))
int test_v8df_blend(int seed) {
    __attribute__((aligned(64))) double src1[8];
    __attribute__((aligned(64))) double src2[8];
    __attribute__((aligned(64))) double result[8];
    volatile double* volatile_result = result;
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (double)(i + seed) * 1.5;
        src2[i] = (double)(i - seed) * 2.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __m512d cmp_val = _mm512_set1_pd(seed);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend with scalar broadcast */
    __m512d scalar = _mm512_set1_pd(seed * 3.0);
    __m512d blended = _mm512_mask_blend_pd(mask, v1, scalar);
    
    /* Should trigger gen_avx512f_blendmv8df */
    
    _mm512_store_pd((void*)volatile_result, blended);
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (int)result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* ========== V16SFmode (16 single-precision floats) ========== */
__attribute__((noinline))
int test_v16sf_blend(int seed) {
    __attribute__((aligned(64))) float src1[16];
    __attribute__((aligned(64))) float src2[16];
    __attribute__((aligned(64))) float result[16];
    volatile float* volatile_result = result;
    
    for (int i = 0; i < 16; i++) {
        src1[i] = (float)(i + seed) * 0.5f;
        src2[i] = (float)(i - seed) * 1.5f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(seed);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend with arithmetic result */
    __m512 added = _mm512_add_ps(v1, v2);
    __m512 blended = _mm512_mask_blend_ps(mask, v1, added);
    
    /* Should trigger gen_avx512f_blendmv16sf */
    
    _mm512_store_ps((void*)volatile_result, blended);
    
    /* Use in loop with argc dependency */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return (int)sum;
}

#endif /* __AVX512F__ */

/* ========== Main driver ========== */
int main(int argc, char* argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int total_result = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("Testing AVX-512BW blend patterns...\n");
    
    /* Test in loop to prevent optimization */
    int loop_count = argc > 2 ? atoi(argv[2]) : 3;
    for (int i = 0; i < loop_count; i++) {
        total_result += test_v64qi_blend(seed + i);
        total_result += test_v32hi_blend(seed + i);
        total_result += test_v32hf_blend(seed + i);
        total_result += test_v32bf_blend(seed + i);
    }
    
    printf("AVX-512BW blend tests completed.\n");
#endif /* __AVX512BW__ */
    
    printf("Testing AVX-512F blend patterns...\n");
    
    /* Test AVX-512F patterns */
    total_result += test_v16si_blend(seed);
    total_result += test_v8di_blend(seed);
    total_result += test_v8df_blend(seed);
    total_result += test_v16sf_blend(seed);
    
    printf("AVX-512F blend tests completed.\n");
    printf("Total checksum: %d\n", total_result);
    
#else
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
#endif
    
    return total_result != 0 ? 0 : 1;
}
