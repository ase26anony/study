/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targeting uncovered lines 4303-4326 in i386-expand.cc
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

/* Test V64QImode - requires AVX512BW */
__attribute__((noinline))
uint64_t test_v64qi_blend(void) {
    __attribute__((aligned(64))) int8_t src1[64];
    __attribute__((aligned(64))) int8_t src2[64];
    __attribute__((aligned(64))) int8_t dst[64];
    volatile __attribute__((aligned(64))) int8_t volatile_dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 - i;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create alternating mask: 0xAAAAAAAAAAAAAAAA */
    __mmask64 mask = create_mask64(0xAAAAAAAAAAAAAAAAULL);
    
    /* Blend using intrinsic - should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, result);
    
    /* Also use in computation */
    __m512i sum = _mm512_add_epi8(result, _mm512_set1_epi8(1));
    _mm512_store_si512((__m512i*)dst, sum);
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += dst[i];
    }
    
    return checksum;
}

/* Test V32HImode - requires AVX512BW */
__attribute__((noinline))
uint64_t test_v32hi_blend(void) {
    __attribute__((aligned(64))) int16_t src1[32];
    __attribute__((aligned(64))) int16_t src2[32];
    __attribute__((aligned(64))) int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 2;
        src2[i] = i * 3;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi16(32);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Blend - should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Use result in arithmetic and store */
    __m512i scaled = _mm512_mullo_epi16(result, _mm512_set1_epi16(2));
    _mm512_store_si512((__m512i*)dst, scaled);
    
    /* Compute checksum with artificial dependency */
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += dst[i];
    }
    __asm__ volatile("" : : "r"(checksum) : "memory");
    
    return checksum;
}

/* Test V32HFmode - half precision float, requires AVX512BW */
__attribute__((noinline))
uint64_t test_v32hf_blend(void) {
    __attribute__((aligned(64))) uint16_t src1[32];  /* Half floats as uint16_t */
    __attribute__((aligned(64))) uint16_t src2[32];
    __attribute__((aligned(64))) uint16_t dst[32];
    
    /* Initialize with simple half-float pattern */
    for (int i = 0; i < 32; i++) {
        src1[i] = 0x3C00 | (i & 0x1F);  /* ~1.0 with variations */
        src2[i] = 0x4000 | (i & 0x1F);  /* ~2.0 with variations */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask: select elements where index is even */
    __mmask32 mask = 0xAAAAAAAA;  /* 10101010... pattern */
    
    /* Blend half floats - should trigger gen_avx512bw_blendmv32hf */
    __m512i result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += dst[i];
    }
    
    return checksum;
}

/* Test V32BFmode - bfloat16, requires AVX512BW */
__attribute__((noinline))
uint64_t test_v32bf_blend(void) {
    __attribute__((aligned(64))) uint16_t src1[32];  /* bfloat16 as uint16_t */
    __attribute__((aligned(64))) uint16_t src2[32];
    __attribute__((aligned(64))) uint16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (i << 8) | 0x3F;  /* Simple bfloat pattern */
        src2[i] = ((i + 16) << 8) | 0x40;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Use _mm512_mask_blend_epi16 for bfloat16 */
    __mmask32 mask = 0x55555555;  /* 01010101... pattern */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += dst[i];
    }
    
    return checksum;
}

#endif /* __AVX512BW__ */

/* Test V16SImode - requires AVX512F */
__attribute__((noinline))
uint64_t test_v16si_blend(void) {
    __attribute__((aligned(64))) int32_t src1[16];
    __attribute__((aligned(64))) int32_t src2[16];
    __attribute__((aligned(64))) int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 100;
        src2[i] = i * 200;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(800);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Blend - should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    /* Blend with broadcasted scalar */
    __m512i scalar = _mm512_set1_epi32(999);
    __m512i result2 = _mm512_mask_blend_epi32(0xF0F0, result, scalar);
    
    _mm512_store_si512((__m512i*)dst, result2);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += dst[i];
    }
    
    return checksum;
}

/* Test V8DImode - requires AVX512F */
__attribute__((noinline))
uint64_t test_v8di_blend(void) {
    __attribute__((aligned(64))) int64_t src1[8];
    __attribute__((aligned(64))) int64_t src2[8];
    __attribute__((aligned(64))) int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1000LL * i;
        src2[i] = 2000LL * i;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask */
    __mmask8 mask = 0xAA;  /* 10101010 */
    
    /* Blend - should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += dst[i];
    }
    
    return checksum;
}

/* Test V8DFmode - requires AVX512F */
__attribute__((noinline))
double test_v8df_blend(void) {
    __attribute__((aligned(64))) double src1[8];
    __attribute__((aligned(64))) double src2[8];
    __attribute__((aligned(64))) double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1.0 * i;
        src2[i] = 2.0 * i;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __m512d cmp_val = _mm512_set1_pd(4.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512d mul = _mm512_mul_pd(v1, _mm512_set1_pd(1.5));
    __m512d result2 = _mm512_mask_blend_pd(0x0F, result, mul);
    
    _mm512_store_pd(dst, result2);
    
    /* Reduction */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* Test V16SFmode - requires AVX512F */
__attribute__((noinline))
float test_v16sf_blend(void) {
    __attribute__((aligned(64))) float src1[16];
    __attribute__((aligned(64))) float src2[16];
    __attribute__((aligned(64))) float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = 0.5f * i;
        src2[i] = 1.5f * i;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    /* Use in loop with volatile control */
    volatile int iterations = 4;
    __m512 accum = _mm512_setzero_ps();
    
    for (int i = 0; i < iterations; i++) {
        __m512 temp = _mm512_add_ps(result, _mm512_set1_ps(i * 0.1f));
        accum = _mm512_mask_blend_ps(0xFF00, accum, temp);
    }
    
    _mm512_store_ps(dst, accum);
    
    /* Reduction */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return sum;
}

#endif /* __AVX512F__ */

int main(int argc, char *argv[]) {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
    printf("AVX-512F supported, testing blend intrinsics...\n");
    
    /* Call tests with different control flow based on argc */
    int loop_count = (argc > 1) ? atoi(argv[1]) : 1;
    if (loop_count < 1) loop_count = 1;
    
    for (int i = 0; i < loop_count; i++) {
        total_checksum += test_v16si_blend();
        total_checksum += (uint64_t)test_v8di_blend();
        
        double df_result = test_v8df_blend();
        total_checksum += (uint64_t)df_result;
        
        float sf_result = test_v16sf_blend();
        total_checksum += (uint64_t)sf_result;
        
#ifdef __AVX512BW__
        total_checksum += test_v64qi_blend();
        total_checksum += test_v32hi_blend();
        total_checksum += test_v32hf_blend();
        total_checksum += test_v32bf_blend();
#endif
    }
    
    printf("Total checksum: %lu\n", total_checksum);
#else
    printf("AVX-512 not supported on this platform\n");
#endif
    
    return (int)(total_checksum & 0x7FFFFFFF);
}
