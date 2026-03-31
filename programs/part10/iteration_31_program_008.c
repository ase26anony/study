/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targets uncovered lines in i386-expand.cc (4303-4326)
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw test_avx512_blend.c -o test_avx512_blend
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __AVX512F__

/* V64QI mode - requires AVX512BW */
#ifdef __AVX512BW__
static int test_v64qi_blend(volatile int argc) {
    __attribute__((aligned(64))) int8_t src1[64];
    __attribute__((aligned(64))) int8_t src2[64];
    __attribute__((aligned(64))) int8_t dst[64];
    
    // Initialize with pattern
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 - i;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Create mask using comparison - blend where src1[i] > 32
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, _mm512_set1_epi8(32));
    
    // This should trigger gen_avx512bw_blendmv64qi
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    // Store to volatile array to prevent optimization
    _mm512_store_si512((__m512i*)dst, blended);
    
    // Use result in computation
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    // Loop with argc dependency
    for (volatile int j = 0; j < (argc & 3); j++) {
        __m512i temp = _mm512_mask_blend_epi8(mask, v2, v1);
        _mm512_store_si512((__m512i*)dst, temp);
        asm volatile("" : : "r"(dst) : "memory");
    }
    
    return sum;
}

/* V32HI mode - requires AVX512BW */
static int test_v32hi_blend(volatile int argc) {
    __attribute__((aligned(64))) int16_t src1[32];
    __attribute__((aligned(64))) int16_t src2[32];
    __attribute__((aligned(64))) int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 2;
        src2[i] = 1000 - i * 3;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Create mask - blend where src1[i] < src2[i]
    __mmask32 mask = _mm512_cmplt_epi16_mask(v1, v2);
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    // Blend with broadcasted scalar
    __m512i broadcast = _mm512_set1_epi16(42);
    __m512i blended2 = _mm512_mask_blend_epi16(mask, blended, broadcast);
    
    _mm512_store_si512((__m512i*)dst, blended2);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    // Artificial dependency on argc
    if (argc > 1) {
        __m512i temp = _mm512_mask_blend_epi16(mask ^ 0xAAAAAAAA, v1, v2);
        asm volatile("" : : "r"(temp) : "memory");
    }
    
    return sum;
}

/* V32HF mode - requires AVX512BW */
static int test_v32hf_blend(volatile int argc) {
    __attribute__((aligned(64))) _Float16 src1[32];
    __attribute__((aligned(64))) _Float16 src2[32];
    __attribute__((aligned(64))) _Float16 dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 1.5f;
        src2[i] = 50.0f - i * 0.7f;
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    // Create mask using floating comparison
    __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_LT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    // Blend with arithmetic result
    __m512h added = _mm512_add_ph(v1, v2);
    __m512h blended2 = _mm512_mask_blend_ph(mask, blended, added);
    
    _mm512_store_ph(dst, blended2);
    
    float sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    // Loop with volatile counter
    volatile int count = argc & 7;
    while (count--) {
        __m512h temp = _mm512_mask_blend_ph(mask, v2, v1);
        asm volatile("" : : "r"(temp) : "memory");
    }
    
    return (int)sum;
}

/* V32BF mode - requires AVX512BW */
static int test_v32bf_blend(volatile int argc) {
    __attribute__((aligned(64))) uint16_t src1[32];  // bfloat16 as uint16_t
    __attribute__((aligned(64))) uint16_t src2[32];
    __attribute__((aligned(64))) uint16_t dst[32];
    
    // Simple pattern for bfloat16
    for (int i = 0; i < 32; i++) {
        src1[i] = i << 7;  // Approximate bfloat16 representation
        src2[i] = (31 - i) << 7;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Create mask - blend where i < 16
    __mmask32 mask = 0x0000FFFF;  // Lower 16 bits set
    
    // Use epi16 blend for bfloat16 (same size)
    // This should trigger gen_avx512bw_blendmv32bf
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, blended);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    // Side effect to prevent dead code elimination
    if (argc > 0) {
        __m512i temp = _mm512_mask_blend_epi16(mask, v2, v1);
        _mm512_store_si512((__m512i*)dst, temp);
    }
    
    return sum;
}
#endif  /* __AVX512BW__ */

/* V16SI mode - requires AVX512F */
static int test_v16si_blend(volatile int argc) {
    __attribute__((aligned(64))) int32_t src1[16];
    __attribute__((aligned(64))) int32_t src2[16];
    __attribute__((aligned(64))) int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 100;
        src2[i] = 2000 - i * 50;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    // Create mask using comparison
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, _mm512_set1_epi32(800));
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    // Blend with arithmetic operation
    __m512i multiplied = _mm512_mullo_epi32(v1, _mm512_set1_epi32(2));
    __m512i blended2 = _mm512_mask_blend_epi32(mask, blended, multiplied);
    
    _mm512_store_epi32(dst, blended2);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    // Artificial dependency
    asm volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

/* V8DI mode - requires AVX512F */
static long test_v8di_blend(volatile int argc) {
    __attribute__((aligned(64))) int64_t src1[8];
    __attribute__((aligned(64))) int64_t src2[8];
    __attribute__((aligned(64))) int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1LL << i;
        src2[i] = 1000LL * i;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    // Create mask - blend where src1 < 64
    __mmask8 mask = _mm512_cmplt_epi64_mask(v1, _mm512_set1_epi64(64));
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    // Blend with broadcasted value
    __m512i broadcast = _mm512_set1_epi64(999);
    __m512i blended2 = _mm512_mask_blend_epi64(mask, blended, broadcast);
    
    _mm512_store_epi64(dst, blended2);
    
    long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    // Loop with argc dependency
    for (int j = 0; j < (argc & 3); j++) {
        __m512i temp = _mm512_mask_blend_epi64(mask, v2, v1);
        asm volatile("" : : "r"(temp) : "memory");
    }
    
    return sum;
}

/* V8DF mode - requires AVX512F */
static double test_v8df_blend(volatile int argc) {
    __attribute__((aligned(64))) double src1[8];
    __attribute__((aligned(64))) double src2[8];
    __attribute__((aligned(64))) double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.1;
        src2[i] = 10.0 / (i + 1);
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    // Create mask using floating comparison
    __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    // Blend with arithmetic result
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(1.5));
    __m512d blended2 = _mm512_mask_blend_pd(mask, blended, multiplied);
    
    _mm512_store_pd(dst, blended2);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    // Prevent optimization
    volatile double vol_sum = sum;
    asm volatile("" : : "r"(vol_sum) : "memory");
    
    return sum;
}

/* V16SF mode - requires AVX512F */
static float test_v16sf_blend(volatile int argc) {
    __attribute__((aligned(64))) float src1[16];
    __attribute__((aligned(64))) float src2[16];
    __attribute__((aligned(64))) float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = 20.0f - i * 0.3f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    // Create mask using multiple comparisons
    __m512 threshold = _mm512_set1_ps(5.0f);
    __mmask16 mask1 = _mm512_cmp_ps_mask(v1, threshold, _CMP_GT_OQ);
    __mmask16 mask2 = _mm512_cmp_ps_mask(v2, threshold, _CMP_LT_OQ);
    __mmask16 mask = mask1 & mask2;
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    // Complex blend chain
    __m512 added = _mm512_add_ps(v1, v2);
    __m512 blended2 = _mm512_mask_blend_ps(mask, blended, added);
    
    __m512 sqrt_val = _mm512_sqrt_ps(_mm512_abs_ps(blended2));
    __m512 final_blend = _mm512_mask_blend_ps(mask, blended2, sqrt_val);
    
    _mm512_store_ps(dst, final_blend);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    // Loop with side effects
    volatile int iterations = (argc % 4) + 1;
    for (volatile int i = 0; i < iterations; i++) {
        __m512 temp = _mm512_mask_blend_ps(mask, v2, v1);
        asm volatile("" : : "r"(temp) : "memory");
    }
    
    return sum;
}

#endif  /* __AVX512F__ */

int main(int argc, char *argv[]) {
    int total_result = 0;
    
#ifdef __AVX512F__
    printf("AVX-512F supported, testing blends...\n");
    
    // Test AVX512F blends
    total_result += test_v16si_blend(argc);
    total_result += (int)test_v8di_blend(argc);
    total_result += (int)test_v8df_blend(argc);
    total_result += (int)test_v16sf_blend(argc);
    
#ifdef __AVX512BW__
    printf("AVX-512BW supported, testing byte/word blends...\n");
    
    // Test AVX512BW blends
    total_result += test_v64qi_blend(argc);
    total_result += test_v32hi_blend(argc);
    total_result += test_v32hf_blend(argc);
    total_result += test_v32bf_blend(argc);
#else
    printf("AVX-512BW not supported, skipping byte/word blends\n");
#endif
    
    printf("All AVX-512 blend tests completed. Result hash: %d\n", total_result);
    
#else
    printf("AVX-512 not supported on this platform\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw test_avx512_blend.c\n");
#endif
    
    return total_result != 0 ? 0 : 1;
}
