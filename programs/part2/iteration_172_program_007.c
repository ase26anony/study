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

/* Function to generate dynamic mask to prevent constant propagation */
static __mmask64 generate_mask64(int seed) {
    /* Create a pattern that changes with seed but isn't constant */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (((i + seed) & 3) < 2) { /* 50% pattern that varies with seed */
            mask |= (1ULL << i);
        }
    }
    return mask;
}

static __mmask32 generate_mask32(int seed) {
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (((i + seed) & 3) < 2) {
            mask |= (1U << i);
        }
    }
    return mask;
}

static __mmask16 generate_mask16(int seed) {
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if (((i + seed) & 3) < 2) {
            mask |= (1 << i);
        }
    }
    return mask;
}

static __mmask8 generate_mask8(int seed) {
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if (((i + seed) & 3) < 2) {
            mask |= (1 << i);
        }
    }
    return mask;
}

#ifdef __AVX512BW__
/* V64QImode - 64-byte integers */
__attribute__((target("avx512bw")))
static void test_v64qimode(int seed, uint8_t* a, uint8_t* b, uint8_t* out) {
    __m512i vec_a = _mm512_loadu_si512((const __m512i*)a);
    __m512i vec_b = _mm512_loadu_si512((const __m512i*)b);
    __mmask64 mask = generate_mask64(seed);
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, vec_a, vec_b);
    
    _mm512_storeu_si512((__m512i*)out, result);
}

/* V32HImode - 32 half-word integers */
__attribute__((target("avx512bw")))
static void test_v32himode(int seed, int16_t* a, int16_t* b, int16_t* out) {
    __m512i vec_a = _mm512_loadu_si512((const __m512i*)a);
    __m512i vec_b = _mm512_loadu_si512((const __m512i*)b);
    __mmask32 mask = generate_mask32(seed);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, vec_a, vec_b);
    
    _mm512_storeu_si512((__m512i*)out, result);
}
#endif /* __AVX512BW__ */

#ifdef __AVX512FP16__
/* V32HFmode - 32 half-precision floats */
__attribute__((target("avx512fp16")))
static void test_v32hfmode(int seed, _Float16* a, _Float16* b, _Float16* out) {
    __m512h vec_a = _mm512_loadu_ph(a);
    __m512h vec_b = _mm512_loadu_ph(b);
    __mmask32 mask = generate_mask32(seed);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, vec_a, vec_b);
    
    _mm512_storeu_ph(out, result);
}
#endif /* __AVX512FP16__ */

#ifdef __AVX512BF16__
/* V32BFmode - 32 bfloat16 floats */
__attribute__((target("avx512bf16")))
static void test_v32bfmode(int seed, __bf16* a, __bf16* b, __bf16* out) {
    /* Load bfloat16 data - need to cast through __m256bh */
    __m256bh vec_a = _mm256_loadu_si256((const __m256i*)a);
    __m256bh vec_b = _mm256_loadu_si256((const __m256i*)b);
    __mmask32 mask = generate_mask32(seed);
    
    /* Convert to __m512h for blending, then back */
    __m512h vec_a_h = _mm512_cvtne2ps_pbh(_mm512_setzero_ps(), 
                                         _mm512_castsi512_ps(_mm512_castsi256_si512(_mm256_castsi256_si512(_mm256_castsi256_si256(vec_a)))));
    __m512h vec_b_h = _mm512_cvtne2ps_pbh(_mm512_setzero_ps(), 
                                         _mm512_castsi512_ps(_mm512_castsi256_si512(_mm256_castsi256_si512(_mm256_castsi256_si256(vec_b)))));
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512h result_h = _mm512_mask_blend_ph(mask, vec_a_h, vec_b_h);
    
    /* Store back as bfloat16 */
    _mm512_storeu_ph((_Float16*)out, result_h);
}
#endif /* __AVX512BF16__ */

#ifdef __AVX512F__
/* V16SImode - 16 single-word integers */
__attribute__((target("avx512f")))
static void test_v16simode(int seed, int32_t* a, int32_t* b, int32_t* out) {
    __m512i vec_a = _mm512_loadu_si512((const __m512i*)a);
    __m512i vec_b = _mm512_loadu_si512((const __m512i*)b);
    __mmask16 mask = generate_mask16(seed);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, vec_a, vec_b);
    
    _mm512_storeu_si512((__m512i*)out, result);
}

/* V8DImode - 8 double-word integers */
__attribute__((target("avx512f")))
static void test_v8dimode(int seed, int64_t* a, int64_t* b, int64_t* out) {
    __m512i vec_a = _mm512_loadu_si512((const __m512i*)a);
    __m512i vec_b = _mm512_loadu_si512((const __m512i*)b);
    __mmask8 mask = generate_mask8(seed);
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, vec_a, vec_b);
    
    _mm512_storeu_si512((__m512i*)out, result);
}

/* V8DFmode - 8 double-precision floats */
__attribute__((target("avx512f")))
static void test_v8dfmode(int seed, double* a, double* b, double* out) {
    __m512d vec_a = _mm512_loadu_pd(a);
    __m512d vec_b = _mm512_loadu_pd(b);
    __mmask8 mask = generate_mask8(seed);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, vec_a, vec_b);
    
    _mm512_storeu_pd(out, result);
}

/* V16SFmode - 16 single-precision floats */
__attribute__((target("avx512f")))
static void test_v16sfmode(int seed, float* a, float* b, float* out) {
    __m512 vec_a = _mm512_loadu_ps(a);
    __m512 vec_b = _mm512_loadu_ps(b);
    __mmask16 mask = generate_mask16(seed);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, vec_a, vec_b);
    
    _mm512_storeu_ps(out, result);
}
#endif /* __AVX512F__ */

int main() {
    int seed = 42; /* Non-constant seed for dynamic masks */
    uint64_t checksum = 0;
    
    /* Test data initialization */
    uint8_t a_bytes[64], b_bytes[64], out_bytes[64];
    int16_t a_words[32], b_words[32], out_words[32];
    int32_t a_dwords[16], b_dwords[16], out_dwords[16];
    int64_t a_qwords[8], b_qwords[8], out_qwords[8];
    float a_floats[16], b_floats[16], out_floats[16];
    double a_doubles[8], b_doubles[8], out_doubles[8];
    
    /* Initialize with distinct patterns */
    for (int i = 0; i < 64; i++) {
        a_bytes[i] = i;
        b_bytes[i] = i + 64;
    }
    for (int i = 0; i < 32; i++) {
        a_words[i] = i * 2;
        b_words[i] = i * 2 + 1;
    }
    for (int i = 0; i < 16; i++) {
        a_dwords[i] = i * 100;
        b_dwords[i] = i * 100 + 50;
    }
    for (int i = 0; i < 8; i++) {
        a_qwords[i] = i * 1000LL;
        b_qwords[i] = i * 1000LL + 500;
    }
    for (int i = 0; i < 16; i++) {
        a_floats[i] = i * 1.5f;
        b_floats[i] = i * 1.5f + 0.75f;
    }
    for (int i = 0; i < 8; i++) {
        a_doubles[i] = i * 2.5;
        b_doubles[i] = i * 2.5 + 1.25;
    }
    
    /* Execute all blend tests in a loop to prevent dead code elimination */
    for (int iter = 0; iter < 10; iter++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
#ifdef __AVX512BW__
        test_v64qimode(seed, a_bytes, b_bytes, out_bytes);
        test_v32himode(seed, a_words, b_words, out_words);
#endif
        
#ifdef __AVX512F__
        test_v16simode(seed, a_dwords, b_dwords, out_dwords);
        test_v8dimode(seed, a_qwords, b_qwords, out_qwords);
        test_v8dfmode(seed, a_doubles, b_doubles, out_doubles);
        test_v16sfmode(seed, a_floats, b_floats, out_floats);
#endif
        
#ifdef __AVX512FP16__
        _Float16 a_half[32], b_half[32], out_half[32];
        for (int i = 0; i < 32; i++) {
            a_half[i] = (_Float16)(i * 0.5f);
            b_half[i] = (_Float16)(i * 0.5f + 0.25f);
        }
        test_v32hfmode(seed, a_half, b_half, out_half);
#endif
        
#ifdef __AVX512BF16__
        __bf16 a_bf16[32], b_bf16[32], out_bf16[32];
        for (int i = 0; i < 32; i++) {
            /* Simple bfloat16 pattern */
            uint16_t val = (i << 8) | (i & 0xFF);
            a_bf16[i] = *(__bf16*)&val;
            b_bf16[i] = *(__bf16*)&(uint16_t){val ^ 0x8000}; /* Flip sign bit */
        }
        test_v32bfmode(seed, a_bf16, b_bf16, out_bf16);
#endif
    }
    
    /* Compute checksum to ensure all operations have effect */
    for (int i = 0; i < 64; i++) checksum += out_bytes[i];
    for (int i = 0; i < 32; i++) checksum += out_words[i];
    for (int i = 0; i < 16; i++) checksum += out_dwords[i];
    for (int i = 0; i < 8; i++) checksum += out_qwords[i];
    for (int i = 0; i < 16; i++) checksum += (uint64_t)out_floats[i];
    for (int i = 0; i < 8; i++) checksum += (uint64_t)out_doubles[i];
    
    printf("Checksum: %lu\n", checksum);
    printf("All AVX-512 blend tests completed (if supported on this platform)\n");
    
    return 0;
}

#ifdef __cplusplus
}
#endif
