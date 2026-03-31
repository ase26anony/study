/* avx512_blend_coverage_test.c
 * Test program to cover AVX-512 blend instruction expansion in GCC's i386-expand.cc
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_coverage_test.c -o avx512_blend_coverage_test
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes with target attributes */
#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode - 64 byte elements */
__attribute__((target("avx512bw")))
static void test_v64qimode_blend(uint8_t* out, const uint8_t* a, const uint8_t* b, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i va = _mm512_loadu_si512((const __m512i*)a);
        __m512i vb = _mm512_loadu_si512((const __m512i*)b);
        
        /* Create dynamic mask based on iteration and data */
        __mmask64 mask = 0;
        for (int j = 0; j < 64; j++) {
            if ((a[j] ^ i) > (b[j] ^ i)) {
                mask |= (1ULL << j);
            }
        }
        
        __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
        _mm512_storeu_si512((__m512i*)out, result);
        
        /* Rotate pointers to prevent optimization */
        a += 64;
        b += 64;
        out += 64;
    }
}

/* V32HImode - 32 half-word elements */
__attribute__((target("avx512bw")))
static void test_v32himode_blend(uint16_t* out, const uint16_t* a, const uint16_t* b, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i va = _mm512_loadu_si512((const __m512i*)a);
        __m512i vb = _mm512_loadu_si512((const __m512i*)b);
        
        /* Dynamic mask based on data comparison */
        __mmask32 mask = _mm512_cmpgt_epi16_mask(va, vb);
        mask ^= (i & 0xFFFFFFFF); /* Mix in iteration for dynamism */
        
        __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
        _mm512_storeu_si512((__m512i*)out, result);
        
        a += 32;
        b += 32;
        out += 32;
    }
}

#endif /* __AVX512BW__ */

#ifdef __AVX512FP16__

/* V32HFmode - 32 half-precision float elements */
__attribute__((target("avx512fp16")))
static void test_v32hfmode_blend(_Float16* out, const _Float16* a, const _Float16* b, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512h va = _mm512_loadu_ph(a);
        __m512h vb = _mm512_loadu_ph(b);
        
        /* Create mask by comparing with threshold */
        __mmask32 mask = _mm512_cmp_ph_mask(va, _mm512_set1_ph(0.5f), _CMP_GT_OQ);
        mask = mask ^ (i & 0xFFFFFFFF); /* Make mask dynamic */
        
        __m512h result = _mm512_mask_blend_ph(mask, va, vb);
        _mm512_storeu_ph(out, result);
        
        a += 32;
        b += 32;
        out += 32;
    }
}

#endif /* __AVX512FP16__ */

#ifdef __AVX512BF16__

/* V32BFmode - 32 bfloat16 elements */
__attribute__((target("avx512bf16")))
static void test_v32bfmode_blend(__bfloat16* out, const __bfloat16* a, const __bfloat16* b, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Load bfloat16 data as 32-bit integers and convert */
        __m512i va_int = _mm512_loadu_si512((const __m512i*)a);
        __m512i vb_int = _mm512_loadu_si512((const __m512i*)b);
        
        /* Convert to __m512bh for blending */
        __m512bh va = (__m512bh)va_int;
        __m512bh vb = (__m512bh)vb_int;
        
        /* Dynamic mask based on iteration */
        __mmask32 mask = 0;
        for (int j = 0; j < 32; j++) {
            if ((i + j) % 3 == 0) {
                mask |= (1U << j);
            }
        }
        
        /* Use the same intrinsic as half-precision */
        __m512bh result = _mm512_mask_blend_ph(mask, va, vb);
        _mm512_storeu_si512((__m512i*)out, (__m512i)result);
        
        a += 32;
        b += 32;
        out += 32;
    }
}

#endif /* __AVX512BF16__ */

/* V16SImode - 32-bit integer elements */
__attribute__((target("avx512f")))
static void test_v16simode_blend(int32_t* out, const int32_t* a, const int32_t* b, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i va = _mm512_loadu_si512((const __m512i*)a);
        __m512i vb = _mm512_loadu_si512((const __m512i*)b);
        
        /* Create mask by comparing elements */
        __mmask16 mask = _mm512_cmpeq_epi32_mask(va, _mm512_set1_epi32(i));
        mask = ~mask; /* Invert for variety */
        
        __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
        _mm512_storeu_si512((__m512i*)out, result);
        
        a += 16;
        b += 16;
        out += 16;
    }
}

/* V8DImode - 64-bit integer elements */
__attribute__((target("avx512f")))
static void test_v8dimode_blend(int64_t* out, const int64_t* a, const int64_t* b, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i va = _mm512_loadu_si512((const __m512i*)a);
        __m512i vb = _mm512_loadu_si512((const __m512i*)b);
        
        /* Dynamic mask based on data parity */
        __mmask8 mask = 0;
        for (int j = 0; j < 8; j++) {
            if ((a[j] & 1) != (i & 1)) {
                mask |= (1 << j);
            }
        }
        
        __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
        _mm512_storeu_si512((__m512i*)out, result);
        
        a += 8;
        b += 8;
        out += 8;
    }
}

/* V8DFmode - double precision elements */
__attribute__((target("avx512f")))
static void test_v8dfmode_blend(double* out, const double* a, const double* b, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512d va = _mm512_loadu_pd(a);
        __m512d vb = _mm512_loadu_pd(b);
        
        /* Create mask by comparing with threshold */
        __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(0.0), _CMP_GT_OQ);
        mask ^= (i & 0xFF); /* Mix iteration for dynamism */
        
        __m512d result = _mm512_mask_blend_pd(mask, va, vb);
        _mm512_storeu_pd(out, result);
        
        a += 8;
        b += 8;
        out += 8;
    }
}

/* V16SFmode - single precision elements */
__attribute__((target("avx512f")))
static void test_v16sfmode_blend(float* out, const float* a, const float* b, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512 va = _mm512_loadu_ps(a);
        __m512 vb = _mm512_loadu_ps(b);
        
        /* Dynamic mask based on element index */
        __mmask16 mask = 0;
        for (int j = 0; j < 16; j++) {
            if ((i + j) % 2 == 0) {
                mask |= (1 << j);
            }
        }
        
        __m512 result = _mm512_mask_blend_ps(mask, va, vb);
        _mm512_storeu_ps(out, result);
        
        a += 16;
        b += 16;
        out += 16;
    }
}

#endif /* __AVX512F__ */

/* Main test driver */
int main() {
    const int ITERATIONS = 4;
    const int BUFFER_SIZE = 1024;
    
    /* Initialize test data */
    uint8_t a_u8[BUFFER_SIZE];
    uint8_t b_u8[BUFFER_SIZE];
    uint8_t out_u8[BUFFER_SIZE];
    
    uint16_t a_u16[BUFFER_SIZE];
    uint16_t b_u16[BUFFER_SIZE];
    uint16_t out_u16[BUFFER_SIZE];
    
    int32_t a_s32[BUFFER_SIZE];
    int32_t b_s32[BUFFER_SIZE];
    int32_t out_s32[BUFFER_SIZE];
    
    int64_t a_s64[BUFFER_SIZE];
    int64_t b_s64[BUFFER_SIZE];
    int64_t out_s64[BUFFER_SIZE];
    
    float a_f32[BUFFER_SIZE];
    float b_f32[BUFFER_SIZE];
    float out_f32[BUFFER_SIZE];
    
    double a_f64[BUFFER_SIZE];
    double b_f64[BUFFER_SIZE];
    double out_f64[BUFFER_SIZE];
    
    /* Initialize with pattern data */
    for (int i = 0; i < BUFFER_SIZE; i++) {
        a_u8[i] = i & 0xFF;
        b_u8[i] = (i + 128) & 0xFF;
        
        a_u16[i] = i * 3;
        b_u16[i] = i * 5;
        
        a_s32[i] = i * 7;
        b_s32[i] = i * 11;
        
        a_s64[i] = i * 13LL;
        b_s64[i] = i * 17LL;
        
        a_f32[i] = i * 0.1f;
        b_f32[i] = i * 0.2f;
        
        a_f64[i] = i * 0.01;
        b_f64[i] = i * 0.02;
    }
    
    long long checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("Testing V64QImode blend...\n");
    test_v64qimode_blend(out_u8, a_u8, b_u8, ITERATIONS);
    for (int i = 0; i < 64 * ITERATIONS; i++) checksum += out_u8[i];
    
    printf("Testing V32HImode blend...\n");
    test_v32himode_blend(out_u16, a_u16, b_u16, ITERATIONS);
    for (int i = 0; i < 32 * ITERATIONS; i++) checksum += out_u16[i];
#endif /* __AVX512BW__ */
    
#ifdef __AVX512FP16__
    printf("Testing V32HFmode blend...\n");
    _Float16 a_half[BUFFER_SIZE];
    _Float16 b_half[BUFFER_SIZE];
    _Float16 out_half[BUFFER_SIZE];
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        a_half[i] = (_Float16)(i * 0.05f);
        b_half[i] = (_Float16)(i * 0.1f);
    }
    
    test_v32hfmode_blend(out_half, a_half, b_half, ITERATIONS);
    for (int i = 0; i < 32 * ITERATIONS; i++) checksum += (int)out_half[i];
#endif /* __AVX512FP16__ */
    
#ifdef __AVX512BF16__
    printf("Testing V32BFmode blend...\n");
    __bfloat16 a_bf16[BUFFER_SIZE];
    __bfloat16 b_bf16[BUFFER_SIZE];
    __bfloat16 out_bf16[BUFFER_SIZE];
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        /* Simple bfloat16 initialization */
        uint16_t val_a = (i * 2) & 0x7FFF;
        uint16_t val_b = (i * 3) & 0x7FFF;
        memcpy(&a_bf16[i], &val_a, sizeof(__bfloat16));
        memcpy(&b_bf16[i], &val_b, sizeof(__bfloat16));
    }
    
    test_v32bfmode_blend(out_bf16, a_bf16, b_bf16, ITERATIONS);
    for (int i = 0; i < 32 * ITERATIONS; i++) {
        uint16_t val;
        memcpy(&val, &out_bf16[i], sizeof(__bfloat16));
        checksum += val;
    }
#endif /* __AVX512BF16__ */
    
    printf("Testing V16SImode blend...\n");
    test_v16simode_blend(out_s32, a_s32, b_s32, ITERATIONS);
    for (int i = 0; i < 16 * ITERATIONS; i++) checksum += out_s32[i];
    
    printf("Testing V8DImode blend...\n");
    test_v8dimode_blend(out_s64, a_s64, b_s64, ITERATIONS);
    for (int i = 0; i < 8 * ITERATIONS; i++) checksum += out_s64[i];
    
    printf("Testing V8DFmode blend...\n");
    test_v8dfmode_blend(out_f64, a_f64, b_f64, ITERATIONS);
    for (int i = 0; i < 8 * ITERATIONS; i++) checksum += (long long)out_f64[i];
    
    printf("Testing V16SFmode blend...\n");
    test_v16sfmode_blend(out_f32, a_f32, b_f32, ITERATIONS);
    for (int i = 0; i < 16 * ITERATIONS; i++) checksum += (long long)out_f32[i];
    
#endif /* __AVX512F__ */
    
    printf("Final checksum: %lld\n", checksum);
    printf("Test completed successfully!\n");
    
    return 0;
}

#ifdef __cplusplus
}
#endif
