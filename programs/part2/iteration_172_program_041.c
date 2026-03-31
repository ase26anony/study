/* AVX-512 Blend Coverage Test for i386-expand.cc */
/* Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_test.c -o avx512_blend_test */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Prevent aggressive optimization */
static volatile int g_volatile_counter = 0;

/* Function to generate dynamic masks based on runtime data */
static __mmask64 generate_mask64(int seed) {
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (((seed + i) & 1) == 0) {
            mask |= (1ULL << i);
        }
    }
    return mask;
}

static __mmask32 generate_mask32(int seed) {
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (((seed + i) & 1) == 0) {
            mask |= (1U << i);
        }
    }
    return mask;
}

static __mmask16 generate_mask16(int seed) {
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if (((seed + i) & 1) == 0) {
            mask |= (1 << i);
        }
    }
    return mask;
}

static __mmask8 generate_mask8(int seed) {
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if (((seed + i) & 1) == 0) {
            mask |= (1 << i);
        }
    }
    return mask;
}

#ifdef __AVX512BW__
/* V64QImode - 64-byte integers */
__attribute__((target("avx512bw")))
static void test_v64qimode(uint8_t* out, const uint8_t* a, const uint8_t* b, int seed) {
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    __mmask64 mask = generate_mask64(seed);
    __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
}

/* V32HImode - 32 half-word integers */
__attribute__((target("avx512bw")))
static void test_v32himode(int16_t* out, const int16_t* a, const int16_t* b, int seed) {
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    __mmask32 mask = generate_mask32(seed);
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
}
#endif

#ifdef __AVX512FP16__
/* V32HFmode - 32 half-precision floats */
__attribute__((target("avx512fp16")))
static void test_v32hfmode(_Float16* out, const _Float16* a, const _Float16* b, int seed) {
    __m512h va = _mm512_loadu_ph(a);
    __m512h vb = _mm512_loadu_ph(b);
    
    __mmask32 mask = generate_mask32(seed);
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_storeu_ph(out, result);
}
#endif

#ifdef __AVX512BF16__
/* V32BFmode - 32 bfloat16 floats */
__attribute__((target("avx512bf16")))
static void test_v32bfmode(__bfloat16* out, const __bfloat16* a, const __bfloat16* b, int seed) {
    /* Load bfloat16 data - using memcpy to avoid strict aliasing */
    __m512bh va, vb;
    memcpy(&va, a, sizeof(va));
    memcpy(&vb, b, sizeof(vb));
    
    __mmask32 mask = generate_mask32(seed);
    __m512bh result = _mm512_mask_blend_ph(mask, va, vb);
    
    memcpy(out, &result, sizeof(result));
}
#endif

#ifdef __AVX512F__
/* V16SImode - 16 single-word integers */
__attribute__((target("avx512f")))
static void test_v16simode(int32_t* out, const int32_t* a, const int32_t* b, int seed) {
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    __mmask16 mask = generate_mask16(seed);
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
}

/* V8DImode - 8 double-word integers */
__attribute__((target("avx512f")))
static void test_v8dimode(int64_t* out, const int64_t* a, const int64_t* b, int seed) {
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    __mmask8 mask = generate_mask8(seed);
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
}

/* V8DFmode - 8 double-precision floats */
__attribute__((target("avx512f")))
static void test_v8dfmode(double* out, const double* a, const double* b, int seed) {
    __m512d va = _mm512_loadu_pd(a);
    __m512d vb = _mm512_loadu_pd(b);
    
    __mmask8 mask = generate_mask8(seed);
    __m512d result = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_storeu_pd(out, result);
}

/* V16SFmode - 16 single-precision floats */
__attribute__((target("avx512f")))
static void test_v16sfmode(float* out, const float* a, const float* b, int seed) {
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    
    __mmask16 mask = generate_mask16(seed);
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_storeu_ps(out, result);
}
#endif

/* Main test function that exercises all blend modes */
int main(void) {
    int seed = g_volatile_counter;
    long long total_checksum = 0;
    
    /* Initialize test data arrays */
    uint8_t a_bytes[64], b_bytes[64], out_bytes[64];
    int16_t a_words[32], b_words[32], out_words[32];
    int32_t a_dwords[16], b_dwords[16], out_dwords[16];
    int64_t a_qwords[8], b_qwords[8], out_qwords[8];
    float a_floats[16], b_floats[16], out_floats[16];
    double a_doubles[8], b_doubles[8], out_doubles[8];
    
    /* Initialize with distinct patterns */
    for (int i = 0; i < 64; i++) {
        a_bytes[i] = i;
        b_bytes[i] = 64 - i;
    }
    
    for (int i = 0; i < 32; i++) {
        a_words[i] = i * 2;
        b_words[i] = 64 - i * 2;
    }
    
    for (int i = 0; i < 16; i++) {
        a_dwords[i] = i * 100;
        b_dwords[i] = 1600 - i * 100;
        a_floats[i] = i * 1.5f;
        b_floats[i] = 24.0f - i * 1.5f;
    }
    
    for (int i = 0; i < 8; i++) {
        a_qwords[i] = i * 1000LL;
        b_qwords[i] = 8000LL - i * 1000LL;
        a_doubles[i] = i * 2.5;
        b_doubles[i] = 20.0 - i * 2.5;
    }
    
    /* Test each blend mode in a loop to prevent optimization */
    for (int iteration = 0; iteration < 10; iteration++) {
        seed += iteration;
        
#ifdef __AVX512BW__
        /* V64QImode */
        test_v64qimode(out_bytes, a_bytes, b_bytes, seed);
        for (int i = 0; i < 64; i++) total_checksum += out_bytes[i];
        
        /* V32HImode */
        test_v32himode(out_words, a_words, b_words, seed);
        for (int i = 0; i < 32; i++) total_checksum += out_words[i];
#endif
        
#ifdef __AVX512FP16__
        /* V32HFmode - using float arrays as proxy for _Float16 */
        _Float16 a_half[32], b_half[32], out_half[32];
        for (int i = 0; i < 32; i++) {
            a_half[i] = (_Float16)(i * 0.5f);
            b_half[i] = (_Float16)(16.0f - i * 0.5f);
        }
        test_v32hfmode(out_half, a_half, b_half, seed);
        for (int i = 0; i < 32; i++) total_checksum += (int)out_half[i];
#endif
        
#ifdef __AVX512BF16__
        /* V32BFmode - using uint16_t arrays as proxy for __bfloat16 */
        __bfloat16 a_bf16[32], b_bf16[32], out_bf16[32];
        for (int i = 0; i < 32; i++) {
            /* Simple bfloat16 pattern */
            uint16_t pattern = (i << 7) | 0x3F80; /* ~1.0 with varying mantissa */
            a_bf16[i] = *(__bfloat16*)&pattern;
            pattern = ((31 - i) << 7) | 0x3F80;
            b_bf16[i] = *(__bfloat16*)&pattern;
        }
        test_v32bfmode(out_bf16, a_bf16, b_bf16, seed);
        for (int i = 0; i < 32; i++) total_checksum += *(uint16_t*)&out_bf16[i];
#endif
        
#ifdef __AVX512F__
        /* V16SImode */
        test_v16simode(out_dwords, a_dwords, b_dwords, seed);
        for (int i = 0; i < 16; i++) total_checksum += out_dwords[i];
        
        /* V8DImode */
        test_v8dimode(out_qwords, a_qwords, b_qwords, seed);
        for (int i = 0; i < 8; i++) total_checksum += out_qwords[i];
        
        /* V8DFmode */
        test_v8dfmode(out_doubles, a_doubles, b_doubles, seed);
        for (int i = 0; i < 8; i++) total_checksum += (long long)out_doubles[i];
        
        /* V16SFmode */
        test_v16sfmode(out_floats, a_floats, b_floats, seed);
        for (int i = 0; i < 16; i++) total_checksum += (long long)out_floats[i];
#endif
    }
    
    printf("Total checksum: %lld\n", total_checksum);
    return 0;
}

#ifdef __cplusplus
}
#endif
