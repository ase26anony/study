/* avx512_blend_coverage_test.c
 * Test program to cover AVX-512 blend instruction expansion in GCC's i386-expand.cc
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_coverage_test.c -o avx512_blend_coverage_test
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Helper function to generate dynamic masks */
static inline __mmask64 generate_mask64(int seed) {
    return (__mmask64)((seed * 1103515245U + 12345U) & 0xFFFFFFFFFFFFFFFFULL);
}

static inline __mmask32 generate_mask32(int seed) {
    return (__mmask32)((seed * 1103515245U + 12345U) & 0xFFFFFFFF);
}

static inline __mmask16 generate_mask16(int seed) {
    return (__mmask16)((seed * 1103515245U + 12345U) & 0xFFFF);
}

static inline __mmask8 generate_mask8(int seed) {
    return (__mmask8)((seed * 1103515245U + 12345U) & 0xFF);
}

/* V64QImode - 64-byte integer blend */
#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
static void test_v64qimode(uint64_t* checksum) {
    __m512i a = _mm512_set_epi8(
        1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
        17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
        33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
        49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64
    );
    
    __m512i b = _mm512_set_epi8(
        64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,
        48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,
        32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,
        16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1
    );
    
    /* Generate dynamic mask based on runtime value */
    __mmask64 mask = generate_mask64(42);
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Store and compute checksum to prevent optimization */
    alignas(64) uint8_t output[64];
    _mm512_store_si512(output, result);
    
    for (int i = 0; i < 64; i++) {
        *checksum += output[i];
    }
}
#endif

/* V32HImode - 32 half-word integer blend */
#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
static void test_v32himode(uint64_t* checksum) {
    __m512i a = _mm512_set_epi16(
        1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
        17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32
    );
    
    __m512i b = _mm512_set_epi16(
        32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,
        16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1
    );
    
    /* Generate dynamic mask */
    __mmask32 mask = generate_mask32(123);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    alignas(64) uint16_t output[32];
    _mm512_store_si512(output, result);
    
    for (int i = 0; i < 32; i++) {
        *checksum += output[i];
    }
}
#endif

/* V32HFmode - 32 half-precision float blend (AVX512-FP16) */
#ifdef __AVX512FP16__
__attribute__((target("avx512fp16")))
static void test_v32hfmode(uint64_t* checksum) {
    __m512h a = _mm512_set_ph(
        1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f, 4.5f,
        5.0f, 5.5f, 6.0f, 6.5f, 7.0f, 7.5f, 8.0f, 8.5f,
        9.0f, 9.5f, 10.0f, 10.5f, 11.0f, 11.5f, 12.0f, 12.5f,
        13.0f, 13.5f, 14.0f, 14.5f, 15.0f, 15.5f, 16.0f, 16.5f
    );
    
    __m512h b = _mm512_set_ph(
        16.5f, 16.0f, 15.5f, 15.0f, 14.5f, 14.0f, 13.5f, 13.0f,
        12.5f, 12.0f, 11.5f, 11.0f, 10.5f, 10.0f, 9.5f, 9.0f,
        8.5f, 8.0f, 7.5f, 7.0f, 6.5f, 6.0f, 5.5f, 5.0f,
        4.5f, 4.0f, 3.5f, 3.0f, 2.5f, 2.0f, 1.5f, 1.0f
    );
    
    /* Generate dynamic mask */
    __mmask32 mask = generate_mask32(456);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    alignas(64) _Float16 output[32];
    _mm512_store_ph(output, result);
    
    /* Convert to uint for checksum */
    for (int i = 0; i < 32; i++) {
        *checksum += (uint64_t)(output[i] * 1000);
    }
}
#endif

/* V32BFmode - 32 bfloat16 blend (AVX512-BF16) */
#ifdef __AVX512BF16__
__attribute__((target("avx512bf16")))
static void test_v32bfmode(uint64_t* checksum) {
    /* Create bfloat16 arrays */
    alignas(64) __bf16 a_data[32], b_data[32];
    
    for (int i = 0; i < 32; i++) {
        a_data[i] = (__bf16)(i * 0.5f);
        b_data[31 - i] = (__bf16)(i * 0.5f);
    }
    
    __m512bh a = _mm512_load_si512(a_data);
    __m512bh b = _mm512_load_si512(b_data);
    
    /* Generate dynamic mask */
    __mmask32 mask = generate_mask32(789);
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    alignas(64) __bf16 output[32];
    _mm512_store_si512(output, result);
    
    for (int i = 0; i < 32; i++) {
        *checksum += (uint64_t)((float)output[i] * 1000);
    }
}
#endif

/* V16SImode - 16 single-word integer blend */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v16simode(uint64_t* checksum) {
    __m512i a = _mm512_set_epi32(
        1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
    );
    
    __m512i b = _mm512_set_epi32(
        16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1
    );
    
    /* Generate dynamic mask using comparison */
    __m512i cmp_a = _mm512_set1_epi32(8);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(a, cmp_a);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    alignas(64) int32_t output[16];
    _mm512_store_si512(output, result);
    
    for (int i = 0; i < 16; i++) {
        *checksum += output[i];
    }
}
#endif

/* V8DImode - 8 double-word integer blend */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v8dimode(uint64_t* checksum) {
    __m512i a = _mm512_set_epi64(1,2,3,4,5,6,7,8);
    __m512i b = _mm512_set_epi64(8,7,6,5,4,3,2,1);
    
    /* Generate dynamic mask */
    __mmask8 mask = generate_mask8(321);
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    alignas(64) int64_t output[8];
    _mm512_store_si512(output, result);
    
    for (int i = 0; i < 8; i++) {
        *checksum += output[i];
    }
}
#endif

/* V8DFmode - 8 double-precision float blend */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v8dfmode(uint64_t* checksum) {
    __m512d a = _mm512_set_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d b = _mm512_set_pd(8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    
    /* Generate dynamic mask using comparison */
    __m512d cmp_val = _mm512_set1_pd(4.5);
    __mmask8 mask = _mm512_cmp_pd_mask(a, cmp_val, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    alignas(64) double output[8];
    _mm512_store_pd(output, result);
    
    for (int i = 0; i < 8; i++) {
        *checksum += (uint64_t)(output[i] * 1000);
    }
}
#endif

/* V16SFmode - 16 single-precision float blend */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v16sfmode(uint64_t* checksum) {
    __m512 a = _mm512_set_ps(
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f
    );
    
    __m512 b = _mm512_set_ps(
        16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f,
        8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f
    );
    
    /* Generate dynamic mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(8.5f);
    __mmask16 mask = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    alignas(64) float output[16];
    _mm512_store_ps(output, result);
    
    for (int i = 0; i < 16; i++) {
        *checksum += (uint64_t)(output[i] * 1000);
    }
}
#endif

int main() {
    uint64_t checksum = 0;
    
    printf("Testing AVX-512 blend instruction expansion coverage...\n");
    
#ifdef __AVX512BW__
    test_v64qimode(&checksum);
    printf("V64QImode test completed\n");
    
    test_v32himode(&checksum);
    printf("V32HImode test completed\n");
#endif

#ifdef __AVX512FP16__
    test_v32hfmode(&checksum);
    printf("V32HFmode test completed\n");
#endif

#ifdef __AVX512BF16__
    test_v32bfmode(&checksum);
    printf("V32BFmode test completed\n");
#endif

#ifdef __AVX512F__
    test_v16simode(&checksum);
    printf("V16SImode test completed\n");
    
    test_v8dimode(&checksum);
    printf("V8DImode test completed\n");
    
    test_v8dfmode(&checksum);
    printf("V8DFmode test completed\n");
    
    test_v16sfmode(&checksum);
    printf("V16SFmode test completed\n");
#endif
    
    printf("Final checksum: %lu\n", checksum);
    printf("All AVX-512 blend tests completed successfully!\n");
    
    return 0;
}

#ifdef __cplusplus
}
#endif
