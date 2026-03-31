/* AVX-512 blend coverage test for i386-expand.cc lines 4303-4326
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_test.c -o avx512_blend_test
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Helper to generate dynamic masks based on input data */
static inline __mmask64 generate_mask64(int seed) {
    return (__mmask64)((0xAAAAAAAAAAAAAAAAULL ^ seed) | 0x5555555555555555ULL);
}

static inline __mmask32 generate_mask32(int seed) {
    return (__mmask32)((0xAAAAAAAA ^ seed) | 0x55555555);
}

static inline __mmask16 generate_mask16(int seed) {
    return (__mmask16)((0xAAAA ^ seed) | 0x5555);
}

static inline __mmask8 generate_mask8(int seed) {
    return (__mmask8)((0xAA ^ seed) | 0x55);
}

/* V64QImode - 64-byte integers */
#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
static void test_v64qimode(uint8_t* src1, uint8_t* src2, uint8_t* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        /* Dynamic mask based on iteration count */
        __mmask64 mask = generate_mask64(i);
        
        /* This should trigger gen_avx512bw_blendmv64qi */
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        _mm512_storeu_si512((__m512i*)dst, result);
        
        /* Rotate data to prevent optimization */
        src1[0] ^= (uint8_t)i;
        src2[0] ^= (uint8_t)(i >> 8);
    }
}
#endif

/* V32HImode - 32 half-word integers */
#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
static void test_v32himode(int16_t* src1, int16_t* src2, int16_t* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        /* Dynamic mask */
        __mmask32 mask = generate_mask32(i);
        
        /* This should trigger gen_avx512bw_blendmv32hi */
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        _mm512_storeu_si512((__m512i*)dst, result);
        
        src1[0] ^= (int16_t)i;
        src2[0] ^= (int16_t)(i >> 4);
    }
}
#endif

/* V32HFmode - 32 half-precision floats (requires AVX512-FP16) */
#ifdef __AVX512FP16__
__attribute__((target("avx512fp16")))
static void test_v32hfmode(_Float16* src1, _Float16* src2, _Float16* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512h a = _mm512_loadu_ph(src1);
        __m512h b = _mm512_loadu_ph(src2);
        
        /* Dynamic mask */
        __mmask32 mask = generate_mask32(i);
        
        /* This should trigger gen_avx512bw_blendmv32hf */
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        _mm512_storeu_ph(dst, result);
        
        src1[0] = (_Float16)(src1[0] + (_Float16)i);
        src2[0] = (_Float16)(src2[0] + (_Float16)(i * 2));
    }
}
#endif

/* V32BFmode - 32 bfloat16 floats (requires AVX512-BF16) */
#ifdef __AVX512BF16__
__attribute__((target("avx512bf16")))
static void test_v32bfmode(__bfloat16* src1, __bfloat16* src2, __bfloat16* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Load as epi32 and reinterpret */
        __m512i a_int = _mm512_loadu_si512((__m512i*)src1);
        __m512i b_int = _mm512_loadu_si512((__m512i*)src2);
        
        /* Dynamic mask */
        __mmask32 mask = generate_mask32(i);
        
        /* Blend at 32-bit level since bfloat16 is stored in 32-bit containers */
        __m512i result_int = _mm512_mask_blend_epi32(mask, a_int, b_int);
        
        _mm512_storeu_si512((__m512i*)dst, result_int);
        
        /* Modify data to prevent optimization */
        src1[0] = (__bfloat16)((float)src1[0] + (float)i);
        src2[0] = (__bfloat16)((float)src2[0] + (float)(i * 2));
    }
}
#endif

/* V16SImode - 16 single-word integers */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v16simode(int32_t* src1, int32_t* src2, int32_t* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        /* Dynamic mask */
        __mmask16 mask = generate_mask16(i);
        
        /* This should trigger gen_avx512f_blendmv16si */
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        _mm512_storeu_si512((__m512i*)dst, result);
        
        src1[0] ^= i;
        src2[0] ^= (i << 1);
    }
}
#endif

/* V8DImode - 8 double-word integers */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v8dimode(int64_t* src1, int64_t* src2, int64_t* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        /* Dynamic mask */
        __mmask8 mask = generate_mask8(i);
        
        /* This should trigger gen_avx512f_blendmv8di */
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        _mm512_storeu_si512((__m512i*)dst, result);
        
        src1[0] ^= (int64_t)i;
        src2[0] ^= (int64_t)(i << 2);
    }
}
#endif

/* V8DFmode - 8 double-precision floats */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v8dfmode(double* src1, double* src2, double* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512d a = _mm512_loadu_pd(src1);
        __m512d b = _mm512_loadu_pd(src2);
        
        /* Dynamic mask */
        __mmask8 mask = generate_mask8(i);
        
        /* This should trigger gen_avx512f_blendmv8df */
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        _mm512_storeu_pd(dst, result);
        
        src1[0] += (double)i * 0.1;
        src2[0] += (double)i * 0.2;
    }
}
#endif

/* V16SFmode - 16 single-precision floats */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v16sfmode(float* src1, float* src2, float* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512 a = _mm512_loadu_ps(src1);
        __m512 b = _mm512_loadu_ps(src2);
        
        /* Dynamic mask */
        __mmask16 mask = generate_mask16(i);
        
        /* This should trigger gen_avx512f_blendmv16sf */
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        _mm512_storeu_ps(dst, result);
        
        src1[0] += (float)i * 0.1f;
        src2[0] += (float)i * 0.2f;
    }
}
#endif

/* Main test driver */
int main() {
    const int ITERATIONS = 100;
    const int ARRAY_SIZE = 64; /* Enough for all vector types */
    
    /* Initialize test data */
    uint8_t src1_u8[ARRAY_SIZE];
    uint8_t src2_u8[ARRAY_SIZE];
    uint8_t dst_u8[ARRAY_SIZE];
    
    int16_t src1_i16[ARRAY_SIZE];
    int16_t src2_i16[ARRAY_SIZE];
    int16_t dst_i16[ARRAY_SIZE];
    
    _Float16 src1_f16[ARRAY_SIZE];
    _Float16 src2_f16[ARRAY_SIZE];
    _Float16 dst_f16[ARRAY_SIZE];
    
    __bfloat16 src1_bf16[ARRAY_SIZE];
    __bfloat16 src2_bf16[ARRAY_SIZE];
    __bfloat16 dst_bf16[ARRAY_SIZE];
    
    int32_t src1_i32[ARRAY_SIZE];
    int32_t src2_i32[ARRAY_SIZE];
    int32_t dst_i32[ARRAY_SIZE];
    
    int64_t src1_i64[ARRAY_SIZE];
    int64_t src2_i64[ARRAY_SIZE];
    int64_t dst_i64[ARRAY_SIZE];
    
    double src1_f64[ARRAY_SIZE];
    double src2_f64[ARRAY_SIZE];
    double dst_f64[ARRAY_SIZE];
    
    float src1_f32[ARRAY_SIZE];
    float src2_f32[ARRAY_SIZE];
    float dst_f32[ARRAY_SIZE];
    
    /* Fill with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src1_u8[i] = (uint8_t)(i * 3);
        src2_u8[i] = (uint8_t)(i * 7);
        
        src1_i16[i] = (int16_t)(i * 5);
        src2_i16[i] = (int16_t)(i * 11);
        
        src1_f16[i] = (_Float16)(i * 0.5f);
        src2_f16[i] = (_Float16)(i * 1.5f);
        
        src1_bf16[i] = (__bfloat16)(i * 0.25f);
        src2_bf16[i] = (__bfloat16)(i * 0.75f);
        
        src1_i32[i] = i * 13;
        src2_i32[i] = i * 17;
        
        src1_i64[i] = (int64_t)i * 19;
        src2_i64[i] = (int64_t)i * 23;
        
        src1_f64[i] = i * 0.3;
        src2_f64[i] = i * 0.7;
        
        src1_f32[i] = i * 0.2f;
        src2_f32[i] = i * 0.8f;
    }
    
    /* Run all tests */
    printf("Running AVX-512 blend coverage tests...\n");
    
#ifdef __AVX512BW__
    test_v64qimode(src1_u8, src2_u8, dst_u8, ITERATIONS);
    printf("  V64QImode test completed\n");
    
    test_v32himode(src1_i16, src2_i16, dst_i16, ITERATIONS);
    printf("  V32HImode test completed\n");
#endif
    
#ifdef __AVX512FP16__
    test_v32hfmode(src1_f16, src2_f16, dst_f16, ITERATIONS);
    printf("  V32HFmode test completed\n");
#endif
    
#ifdef __AVX512BF16__
    test_v32bfmode(src1_bf16, src2_bf16, dst_bf16, ITERATIONS);
    printf("  V32BFmode test completed\n");
#endif
    
#ifdef __AVX512F__
    test_v16simode(src1_i32, src2_i32, dst_i32, ITERATIONS);
    printf("  V16SImode test completed\n");
    
    test_v8dimode(src1_i64, src2_i64, dst_i64, ITERATIONS);
    printf("  V8DImode test completed\n");
    
    test_v8dfmode(src1_f64, src2_f64, dst_f64, ITERATIONS);
    printf("  V8DFmode test completed\n");
    
    test_v16sfmode(src1_f32, src2_f32, dst_f32, ITERATIONS);
    printf("  V16SFmode test completed\n");
#endif
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst_u8[i] + dst_i16[i] + dst_i32[i] + dst_i64[i];
        checksum += (uint64_t)dst_f32[i] + (uint64_t)dst_f64[i];
    }
    
    printf("All tests completed. Checksum: %lu\n", checksum);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
