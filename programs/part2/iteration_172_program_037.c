/* avx512_blend_coverage.c
 * Test program to cover AVX-512 blend instruction expansion in GCC i386-expand.cc
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_coverage.c -o avx512_blend_coverage
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Prevent aggressive optimization */
static volatile int use_result = 1;

/* Function to generate non-constant masks */
static __mmask64 generate_mask64(int seed) {
    return (__mmask64)((seed * 0x5DEECE66DLL + 0xB) & 0xFFFFFFFFFFFFFFFFULL);
}

static __mmask32 generate_mask32(int seed) {
    return (__mmask32)((seed * 0x5DEECE66DLL + 0xB) & 0xFFFFFFFF);
}

static __mmask16 generate_mask16(int seed) {
    return (__mmask16)((seed * 0x5DEECE66DLL + 0xB) & 0xFFFF);
}

static __mmask8 generate_mask8(int seed) {
    return (__mmask8)((seed * 0x5DEECE66DLL + 0xB) & 0xFF);
}

/* V64QImode - 64-byte integers */
#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
static void test_v64qimode(uint8_t* out, const uint8_t* a, const uint8_t* b, int seed) {
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Generate dynamic mask */
    __mmask64 mask = generate_mask64(seed);
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
}
#endif

/* V32HImode - 32 half-word integers */
#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
static void test_v32himode(int16_t* out, const int16_t* a, const int16_t* b, int seed) {
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Generate dynamic mask */
    __mmask32 mask = generate_mask32(seed);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
}
#endif

/* V32HFmode - 32 half-precision floats (AVX512-FP16) */
#ifdef __AVX512FP16__
__attribute__((target("avx512fp16")))
static void test_v32hfmode(_Float16* out, const _Float16* a, const _Float16* b, int seed) {
    __m512h va = _mm512_loadu_ph(a);
    __m512h vb = _mm512_loadu_ph(b);
    
    /* Generate dynamic mask */
    __mmask32 mask = generate_mask32(seed);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_storeu_ph(out, result);
}
#endif

/* V32BFmode - 32 bfloat16 (AVX512-BF16) */
#ifdef __AVX512BF16__
__attribute__((target("avx512bf16")))
static void test_v32bfmode(__bfloat16* out, const __bfloat16* a, const __bfloat16* b, int seed) {
    /* Load bfloat16 data - using integer load since there's no direct bfloat16 load intrinsic */
    __m512i va_int = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb_int = _mm512_loadu_si512((const __m512i*)b);
    
    /* Convert to __m512bh for blend operation */
    __m512bh va = _mm512_castsi512_bh(va_int);
    __m512bh vb = _mm512_castsi512_bh(vb_int);
    
    /* Generate dynamic mask */
    __mmask32 mask = generate_mask32(seed);
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, _mm512_castbh_si512(result));
}
#endif

/* V16SImode - 32-bit integers */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v16simode(int32_t* out, const int32_t* a, const int32_t* b, int seed) {
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Generate dynamic mask using comparison */
    __mmask16 mask = generate_mask16(seed);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
}
#endif

/* V8DImode - 64-bit integers */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v8dimode(int64_t* out, const int64_t* a, const int64_t* b, int seed) {
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Generate dynamic mask */
    __mmask8 mask = generate_mask8(seed);
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
}
#endif

/* V8DFmode - double precision floats */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v8dfmode(double* out, const double* a, const double* b, int seed) {
    __m512d va = _mm512_loadu_pd(a);
    __m512d vb = _mm512_loadu_pd(b);
    
    /* Generate dynamic mask */
    __mmask8 mask = generate_mask8(seed);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_storeu_pd(out, result);
}
#endif

/* V16SFmode - single precision floats */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v16sfmode(float* out, const float* a, const float* b, int seed) {
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    
    /* Generate dynamic mask */
    __mmask16 mask = generate_mask16(seed);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_storeu_ps(out, result);
}
#endif

int main() {
    int i;
    uint64_t checksum = 0;
    
    /* Initialize test data */
    uint8_t a_bytes[64], b_bytes[64], out_bytes[64];
    int16_t a_short[32], b_short[32], out_short[32];
    int32_t a_int[16], b_int[16], out_int[16];
    int64_t a_long[8], b_long[8], out_long[8];
    float a_float[16], b_float[16], out_float[16];
    double a_double[8], b_double[8], out_double[8];
    
    /* Initialize with distinct patterns */
    for (i = 0; i < 64; i++) {
        a_bytes[i] = i;
        b_bytes[i] = i + 64;
    }
    
    for (i = 0; i < 32; i++) {
        a_short[i] = i * 2;
        b_short[i] = i * 2 + 1;
    }
    
    for (i = 0; i < 16; i++) {
        a_int[i] = i * 100;
        b_int[i] = i * 100 + 50;
        a_float[i] = i * 1.5f;
        b_float[i] = i * 2.5f;
    }
    
    for (i = 0; i < 8; i++) {
        a_long[i] = i * 1000LL;
        b_long[i] = i * 1000LL + 500;
        a_double[i] = i * 3.14159;
        b_double[i] = i * 2.71828;
    }
    
    /* Test each blend operation in a loop to prevent optimization */
    for (int iter = 0; iter < 10; iter++) {
#ifdef __AVX512BW__
        test_v64qimode(out_bytes, a_bytes, b_bytes, iter);
        test_v32himode(out_short, a_short, b_short, iter);
#endif
        
#ifdef __AVX512F__
        test_v16simode(out_int, a_int, b_int, iter);
        test_v8dimode(out_long, a_long, b_long, iter);
        test_v8dfmode(out_double, a_double, b_double, iter);
        test_v16sfmode(out_float, a_float, b_float, iter);
#endif
        
#ifdef __AVX512FP16__
        /* Half-precision float test */
        _Float16 a_half[32], b_half[32], out_half[32];
        for (i = 0; i < 32; i++) {
            a_half[i] = (_Float16)(i * 0.5f);
            b_half[i] = (_Float16)(i * 1.5f);
        }
        test_v32hfmode(out_half, a_half, b_half, iter);
#endif
        
#ifdef __AVX512BF16__
        /* Bfloat16 test */
        __bfloat16 a_bf16[32], b_bf16[32], out_bf16[32];
        for (i = 0; i < 32; i++) {
            /* Simple bfloat16 pattern */
            uint16_t val = (i << 8) | (i & 0xFF);
            a_bf16[i] = *(__bfloat16*)&val;
            b_bf16[i] = *(__bfloat16*)&(uint16_t){val ^ 0x8000}; /* Flip sign bit */
        }
        test_v32bfmode(out_bf16, a_bf16, b_bf16, iter);
#endif
    }
    
    /* Compute checksum to ensure all operations are used */
    for (i = 0; i < 64; i++) checksum += out_bytes[i];
    for (i = 0; i < 32; i++) checksum += out_short[i];
    for (i = 0; i < 16; i++) checksum += out_int[i] + (uint64_t)out_float[i];
    for (i = 0; i < 8; i++) checksum += out_long[i] + (uint64_t)out_double[i];
    
    printf("Checksum: %lu\n", checksum);
    
    /* Use the result to prevent dead code elimination */
    if (use_result) {
        return (int)(checksum & 0x7FFFFFFF);
    }
    
    return 0;
}

#ifdef __cplusplus
}
#endif
