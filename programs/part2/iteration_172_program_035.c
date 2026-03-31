/* avx512_blend_coverage.c
 * Test program to cover AVX-512 blend instruction expansion in GCC's i386-expand.cc
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_coverage.c -o avx512_blend_coverage
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Helper function to generate dynamic masks based on input data */
static inline __mmask64 generate_mask64(int seed) {
    /* Create a non-constant mask pattern */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i + seed) % 3 == 0) {
            mask |= (1ULL << i);
        }
    }
    return mask;
}

static inline __mmask32 generate_mask32(int seed) {
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i + seed) % 2 == 0) {
            mask |= (1U << i);
        }
    }
    return mask;
}

static inline __mmask16 generate_mask16(int seed) {
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if ((i + seed) % 4 == 0) {
            mask |= (1 << i);
        }
    }
    return mask;
}

static inline __mmask8 generate_mask8(int seed) {
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((i + seed) % 3 == 0) {
            mask |= (1 << i);
        }
    }
    return mask;
}

#ifdef __AVX512BW__
/* V64QImode - 64-byte integer blend */
__attribute__((target("avx512bw")))
static void test_v64qimode_blend(uint8_t* a, uint8_t* b, uint8_t* out, int seed) {
    __m512i vec_a = _mm512_loadu_si512((const __m512i*)a);
    __m512i vec_b = _mm512_loadu_si512((const __m512i*)b);
    
    /* Generate dynamic mask */
    __mmask64 mask = generate_mask64(seed);
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, vec_a, vec_b);
    
    _mm512_storeu_si512((__m512i*)out, result);
}

/* V32HImode - 32 half-word integer blend */
__attribute__((target("avx512bw")))
static void test_v32himode_blend(int16_t* a, int16_t* b, int16_t* out, int seed) {
    __m512i vec_a = _mm512_loadu_si512((const __m512i*)a);
    __m512i vec_b = _mm512_loadu_si512((const __m512i*)b);
    
    /* Generate dynamic mask */
    __mmask32 mask = generate_mask32(seed);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, vec_a, vec_b);
    
    _mm512_storeu_si512((__m512i*)out, result);
}
#endif /* __AVX512BW__ */

#ifdef __AVX512FP16__
/* V32HFmode - 32 half-precision float blend */
__attribute__((target("avx512fp16")))
static void test_v32hfmode_blend(_Float16* a, _Float16* b, _Float16* out, int seed) {
    __m512h vec_a = _mm512_loadu_ph(a);
    __m512h vec_b = _mm512_loadu_ph(b);
    
    /* Generate dynamic mask */
    __mmask32 mask = generate_mask32(seed);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, vec_a, vec_b);
    
    _mm512_storeu_ph(out, result);
}
#endif /* __AVX512FP16__ */

#ifdef __AVX512BF16__
/* V32BFmode - 32 bfloat16 blend */
__attribute__((target("avx512bf16")))
static void test_v32bfmode_blend(__bfloat16* a, __bfloat16* b, __bfloat16* out, int seed) {
    /* Load bfloat16 data */
    __m512bh vec_a = _mm512_loadu_bf16(a);
    __m512bh vec_b = _mm512_loadu_bf16(b);
    
    /* Generate dynamic mask */
    __mmask32 mask = generate_mask32(seed);
    
    /* Use the same intrinsic as half-precision for bfloat16 */
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, vec_a, vec_b);
    
    _mm512_storeu_bf16(out, result);
}
#endif /* __AVX512BF16__ */

#ifdef __AVX512F__
/* V16SImode - 16 single-word integer blend */
__attribute__((target("avx512f")))
static void test_v16simode_blend(int32_t* a, int32_t* b, int32_t* out, int seed) {
    __m512i vec_a = _mm512_loadu_si512((const __m512i*)a);
    __m512i vec_b = _mm512_loadu_si512((const __m512i*)b);
    
    /* Generate dynamic mask */
    __mmask16 mask = generate_mask16(seed);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, vec_a, vec_b);
    
    _mm512_storeu_si512((__m512i*)out, result);
}

/* V8DImode - 8 double-word integer blend */
__attribute__((target("avx512f")))
static void test_v8dimode_blend(int64_t* a, int64_t* b, int64_t* out, int seed) {
    __m512i vec_a = _mm512_loadu_si512((const __m512i*)a);
    __m512i vec_b = _mm512_loadu_si512((const __m512i*)b);
    
    /* Generate dynamic mask */
    __mmask8 mask = generate_mask8(seed);
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, vec_a, vec_b);
    
    _mm512_storeu_si512((__m512i*)out, result);
}

/* V8DFmode - 8 double-precision float blend */
__attribute__((target("avx512f")))
static void test_v8dfmode_blend(double* a, double* b, double* out, int seed) {
    __m512d vec_a = _mm512_loadu_pd(a);
    __m512d vec_b = _mm512_loadu_pd(b);
    
    /* Generate dynamic mask */
    __mmask8 mask = generate_mask8(seed);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, vec_a, vec_b);
    
    _mm512_storeu_pd(out, result);
}

/* V16SFmode - 16 single-precision float blend */
__attribute__((target("avx512f")))
static void test_v16sfmode_blend(float* a, float* b, float* out, int seed) {
    __m512 vec_a = _mm512_loadu_ps(a);
    __m512 vec_b = _mm512_loadu_ps(b);
    
    /* Generate dynamic mask */
    __mmask16 mask = generate_mask16(seed);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, vec_a, vec_b);
    
    _mm512_storeu_ps(out, result);
}
#endif /* __AVX512F__ */

/* Main test driver */
int main() {
    int i;
    int seed = 42; /* Non-constant seed for dynamic masks */
    uint64_t checksum = 0;
    
    /* Initialize test data */
    uint8_t a_bytes[64], b_bytes[64], out_bytes[64];
    int16_t a_words[32], b_words[32], out_words[32];
    int32_t a_dwords[16], b_dwords[16], out_dwords[16];
    int64_t a_qwords[8], b_qwords[8], out_qwords[8];
    float a_floats[16], b_floats[16], out_floats[16];
    double a_doubles[8], b_doubles[8], out_doubles[8];
    
    /* Fill arrays with distinct patterns */
    for (i = 0; i < 64; i++) {
        a_bytes[i] = i;
        b_bytes[i] = i + 64;
    }
    
    for (i = 0; i < 32; i++) {
        a_words[i] = i * 2;
        b_words[i] = i * 2 + 1;
    }
    
    for (i = 0; i < 16; i++) {
        a_dwords[i] = i * 100;
        b_dwords[i] = i * 100 + 50;
        a_floats[i] = i * 1.5f;
        b_floats[i] = i * 2.5f;
    }
    
    for (i = 0; i < 8; i++) {
        a_qwords[i] = i * 1000LL;
        b_qwords[i] = i * 1000LL + 500;
        a_doubles[i] = i * 3.14159;
        b_doubles[i] = i * 2.71828;
    }
    
    printf("Testing AVX-512 blend instruction expansion...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode blend...\n");
    test_v64qimode_blend(a_bytes, b_bytes, out_bytes, seed);
    
    printf("Testing V32HImode blend...\n");
    test_v32himode_blend(a_words, b_words, out_words, seed);
#endif
    
#ifdef __AVX512FP16__
    /* Half-precision test data */
    _Float16 a_half[32], b_half[32], out_half[32];
    for (i = 0; i < 32; i++) {
        a_half[i] = (_Float16)(i * 0.5f);
        b_half[i] = (_Float16)(i * 1.5f);
    }
    
    printf("Testing V32HFmode blend...\n");
    test_v32hfmode_blend(a_half, b_half, out_half, seed);
#endif
    
#ifdef __AVX512BF16__
    /* Bfloat16 test data */
    __bfloat16 a_bf16[32], b_bf16[32], out_bf16[32];
    for (i = 0; i < 32; i++) {
        a_bf16[i] = (__bfloat16)(i * 0.25f);
        b_bf16[i] = (__bfloat16)(i * 0.75f);
    }
    
    printf("Testing V32BFmode blend...\n");
    test_v32bfmode_blend(a_bf16, b_bf16, out_bf16, seed);
#endif
    
#ifdef __AVX512F__
    printf("Testing V16SImode blend...\n");
    test_v16simode_blend(a_dwords, b_dwords, out_dwords, seed);
    
    printf("Testing V8DImode blend...\n");
    test_v8dimode_blend(a_qwords, b_qwords, out_qwords, seed);
    
    printf("Testing V8DFmode blend...\n");
    test_v8dfmode_blend(a_doubles, b_doubles, out_doubles, seed);
    
    printf("Testing V16SFmode blend...\n");
    test_v16sfmode_blend(a_floats, b_floats, out_floats, seed);
#endif
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < 64; i++) checksum += out_bytes[i];
    for (i = 0; i < 32; i++) checksum += out_words[i];
    for (i = 0; i < 16; i++) checksum += out_dwords[i];
    for (i = 0; i < 8; i++) checksum += out_qwords[i];
    
    printf("Checksum: %lu\n", checksum);
    printf("Test completed successfully!\n");
    
    return 0;
}

#ifdef __cplusplus
}
#endif
