/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targets uncovered lines in i386-expand.cc (4303-4326)
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw test_avx512_blend.c -o test_avx512_blend
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* Helper to prevent optimization */
static volatile int g_volatile_counter = 0;

/* V64QImode - 64 x 8-bit integers */
__attribute__((noinline))
uint64_t test_v64qi_blend(void) {
    __attribute__((aligned(64))) int8_t src1[64];
    __attribute__((aligned(64))) int8_t src2[64];
    __attribute__((aligned(64))) int8_t dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 - i;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask: select from v1 where (i % 3 == 0), else from v2 */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i % 3) == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, v2, v1);
    
    /* Use result in non-trivial way */
    _mm512_store_si512((__m512i*)dst, result);
    
    /* Compute checksum */
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    /* Create artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum + g_volatile_counter;
}

/* V32HImode - 32 x 16-bit integers */
__attribute__((noinline))
uint64_t test_v32hi_blend(void) {
    __attribute__((aligned(64))) int16_t src1[32];
    __attribute__((aligned(64))) int16_t src2[32];
    __attribute__((aligned(64))) int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = i * 200;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi16(1500);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(999);
    result = _mm512_mask_blend_epi16(mask ^ 0xAAAAAAAA, result, broadcast);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V32HFmode - 32 x half-precision floats */
__attribute__((noinline))
uint64_t test_v32hf_blend(void) {
    __attribute__((aligned(64))) uint16_t src1_data[32];  /* Half floats as uint16_t */
    __attribute__((aligned(64))) uint16_t src2_data[32];
    __attribute__((aligned(64))) uint16_t dst_data[32];
    
    /* Initialize with simple half-float pattern */
    for (int i = 0; i < 32; i++) {
        /* Simple pattern: 1.0, 2.0, 3.0, ... */
        src1_data[i] = (i + 1) << 10;  /* Rough approximation */
        src2_data[i] = (32 - i) << 10;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2_data);
    
    /* Create alternating mask */
    __mmask32 mask = 0xAAAAAAAA;  /* 101010... pattern */
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512i result = _mm512_mask_blend_ph(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)dst_data, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst_data[i];
    }
    
    return sum;
}

/* V32BFmode - 32 x bfloat16 */
__attribute__((noinline))
uint64_t test_v32bf_blend(void) {
    __attribute__((aligned(64))) uint16_t src1_data[32];  /* BF16 as uint16_t */
    __attribute__((aligned(64))) uint16_t src2_data[32];
    __attribute__((aligned(64))) uint16_t dst_data[32];
    
    for (int i = 0; i < 32; i++) {
        src1_data[i] = i * 0x0400;  /* Simple bfloat16 pattern */
        src2_data[i] = 0x3F80 + i;  /* ~1.0 + small offset */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2_data);
    
    /* Create mask based on LSB */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((src1_data[i] & 1) == 0) {
            mask |= (1U << i);
        }
    }
    
    /* BF16 uses same intrinsic as epi16 */
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)dst_data, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst_data[i];
    }
    
    return sum;
}

#endif /* __AVX512BW__ */

/* V16SImode - 16 x 32-bit integers */
__attribute__((noinline))
uint64_t test_v16si_blend(void) {
    __attribute__((aligned(64))) int32_t src1[16];
    __attribute__((aligned(64))) int32_t src2[16];
    __attribute__((aligned(64))) int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, v2, v1);
    
    /* Use in arithmetic operation */
    __m512i add_result = _mm512_add_epi32(result, _mm512_set1_epi32(1));
    result = _mm512_mask_blend_epi32(mask ^ 0xAAAA, add_result, result);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V8DImode - 8 x 64-bit integers */
__attribute__((noinline))
uint64_t test_v8di_blend(void) {
    __attribute__((aligned(64))) int64_t src1[8];
    __attribute__((aligned(64))) int64_t src2[8];
    __attribute__((aligned(64))) int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1000000LL * i;
        src2[i] = 2000000LL * i;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask: select where src1 > 3000000 */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if (src1[i] > 3000000) {
            mask |= (1 << i);
        }
    }
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V8DFmode - 8 x double-precision floats */
__attribute__((noinline))
uint64_t test_v8df_blend(void) {
    __attribute__((aligned(64))) double src1[8];
    __attribute__((aligned(64))) double src2[8];
    __attribute__((aligned(64))) double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 2.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __m512d cmp_val = _mm512_set1_pd(6.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, v2, v1);
    
    /* Blend with arithmetic result */
    __m512d mul_result = _mm512_mul_pd(result, _mm512_set1_pd(2.0));
    result = _mm512_mask_blend_pd(mask ^ 0xAA, mul_result, result);
    
    _mm512_store_pd(dst, result);
    
    /* Compute integer hash from doubles */
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)(dst[i] * 1000);
    }
    
    return sum;
}

/* V16SFmode - 16 x single-precision floats */
__attribute__((noinline))
uint64_t test_v16sf_blend(void) {
    __attribute__((aligned(64))) float src1[16];
    __attribute__((aligned(64))) float src2[16];
    __attribute__((aligned(64))) float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 1.5f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, v2, v1);
    
    /* Use in loop with volatile to prevent optimization */
    for (int iter = 0; iter < g_volatile_counter + 1; iter++) {
        __m512 add_result = _mm512_add_ps(result, _mm512_set1_ps(0.1f));
        result = _mm512_mask_blend_ps(mask ^ 0xAAAA, add_result, result);
    }
    
    _mm512_store_ps(dst, result);
    
    /* Compute integer hash */
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint64_t)(dst[i] * 1000);
    }
    
    return sum;
}

#endif /* __AVX512F__ */

int main(int argc, char **argv) {
    uint64_t total_hash = 0;
    
#ifdef __AVX512F__
    printf("AVX-512F supported, testing blends...\n");
    
    /* Test AVX-512F modes */
    total_hash ^= test_v16si_blend();
    total_hash ^= test_v8di_blend();
    total_hash ^= test_v8df_blend();
    total_hash ^= test_v16sf_blend();
    
#ifdef __AVX512BW__
    printf("AVX-512BW supported, testing byte/word blends...\n");
    
    /* Test AVX-512BW modes */
    total_hash ^= test_v64qi_blend();
    total_hash ^= test_v32hi_blend();
    total_hash ^= test_v32hf_blend();
    total_hash ^= test_v32bf_blend();
#else
    printf("AVX-512BW not supported, skipping byte/word blend tests\n");
#endif
    
    printf("Final hash: 0x%016lx\n", total_hash);
    
    /* Use result to affect return code */
    return (int)(total_hash & 0x7FFFFFFF) % 256;
#else
    printf("AVX-512 not supported on this platform\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
    return 1;
#endif
}
