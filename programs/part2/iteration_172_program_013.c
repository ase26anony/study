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

/* Function attributes to ensure specific ISA usage */
#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64 x 8-bit integers */
__attribute__((target("avx512bw")))
static void test_v64qimode(uint8_t* src1, uint8_t* src2, uint8_t* dst, int len) {
    for (int i = 0; i < len; i += 64) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask based on element position parity */
        __mmask64 mask = 0;
        for (int j = 0; j < 64; j++) {
            if (((i + j) & 1) == 0) {
                mask |= (1ULL << j);
            }
        }
        
        __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V32HImode: 32 x 16-bit integers */
__attribute__((target("avx512bw")))
static void test_v32himode(int16_t* src1, int16_t* src2, int16_t* dst, int len) {
    for (int i = 0; i < len; i += 32) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask based on element comparison */
        __mmask32 mask = _mm512_cmpeq_epi16_mask(v1, v2);
        mask = ~mask; /* Invert to ensure some blending happens */
        
        __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

#endif /* __AVX512BW__ */

#ifdef __AVX512FP16__

/* V32HFmode: 32 x half-precision floats */
__attribute__((target("avx512fp16")))
static void test_v32hfmode(_Float16* src1, _Float16* src2, _Float16* dst, int len) {
    for (int i = 0; i < len; i += 32) {
        __m512h v1 = _mm512_loadu_ph(src1 + i);
        __m512h v2 = _mm512_loadu_ph(src2 + i);
        
        /* Dynamic mask based on element index */
        __mmask32 mask = 0;
        for (int j = 0; j < 32; j++) {
            if ((j % 3) == 0) { /* Every 3rd element */
                mask |= (1ULL << j);
            }
        }
        
        __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
        _mm512_storeu_ph(dst + i, result);
    }
}

#endif /* __AVX512FP16__ */

#ifdef __AVX512BF16__

/* V32BFmode: 32 x bfloat16 */
__attribute__((target("avx512bf16")))
static void test_v32bfmode(__bfloat16* src1, __bfloat16* src2, __bfloat16* dst, int len) {
    for (int i = 0; i < len; i += 32) {
        /* Load as __m512i and reinterpret */
        __m512i v1_data = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i v2_data = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        __m512bh v1 = _mm512_castsi512_ph(v1_data);
        __m512bh v2 = _mm512_castsi512_ph(v2_data);
        
        /* Dynamic mask: blend elements where index is even */
        __mmask32 mask = 0;
        for (int j = 0; j < 32; j++) {
            if ((j & 1) == 0) {
                mask |= (1ULL << j);
            }
        }
        
        __m512bh result = _mm512_mask_blend_ph(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), _mm512_castph_si512(result));
    }
}

#endif /* __AVX512BF16__ */

/* V16SImode: 16 x 32-bit integers */
__attribute__((target("avx512f")))
static void test_v16simode(int32_t* src1, int32_t* src2, int32_t* dst, int len) {
    for (int i = 0; i < len; i += 16) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask from comparison */
        __mmask16 mask = _mm512_cmpeq_epi32_mask(v1, v2);
        mask = mask ^ 0xAAAA; /* XOR with pattern to ensure mixing */
        
        __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((target("avx512f")))
static void test_v8dimode(int64_t* src1, int64_t* src2, int64_t* dst, int len) {
    for (int i = 0; i < len; i += 8) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask: blend based on element value range */
        __mmask8 mask = 0;
        for (int j = 0; j < 8; j++) {
            if (src1[i + j] > src2[i + j]) {
                mask |= (1ULL << j);
            }
        }
        
        __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V8DFmode: 8 x double precision floats */
__attribute__((target("avx512f")))
static void test_v8dfmode(double* src1, double* src2, double* dst, int len) {
    for (int i = 0; i < len; i += 8) {
        __m512d v1 = _mm512_loadu_pd(src1 + i);
        __m512d v2 = _mm512_loadu_pd(src2 + i);
        
        /* Dynamic mask from comparison */
        __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
        _mm512_storeu_pd(dst + i, result);
    }
}

/* V16SFmode: 16 x single precision floats */
__attribute__((target("avx512f")))
static void test_v16sfmode(float* src1, float* src2, float* dst, int len) {
    for (int i = 0; i < len; i += 16) {
        __m512 v1 = _mm512_loadu_ps(src1 + i);
        __m512 v2 = _mm512_loadu_ps(src2 + i);
        
        /* Dynamic mask from comparison */
        __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_NEQ_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
        _mm512_storeu_ps(dst + i, result);
    }
}

#endif /* __AVX512F__ */

#ifdef __cplusplus
}
#endif

int main() {
    const int ARRAY_SIZE = 1024;
    uint64_t checksum = 0;
    
    /* Initialize test data */
    uint8_t src1_u8[ARRAY_SIZE];
    uint8_t src2_u8[ARRAY_SIZE];
    uint8_t dst_u8[ARRAY_SIZE];
    
    int16_t src1_i16[ARRAY_SIZE];
    int16_t src2_i16[ARRAY_SIZE];
    int16_t dst_i16[ARRAY_SIZE];
    
    int32_t src1_i32[ARRAY_SIZE];
    int32_t src2_i32[ARRAY_SIZE];
    int32_t dst_i32[ARRAY_SIZE];
    
    int64_t src1_i64[ARRAY_SIZE];
    int64_t src2_i64[ARRAY_SIZE];
    int64_t dst_i64[ARRAY_SIZE];
    
    float src1_f32[ARRAY_SIZE];
    float src2_f32[ARRAY_SIZE];
    float dst_f32[ARRAY_SIZE];
    
    double src1_f64[ARRAY_SIZE];
    double src2_f64[ARRAY_SIZE];
    double dst_f64[ARRAY_SIZE];
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src1_u8[i] = i & 0xFF;
        src2_u8[i] = (i + 128) & 0xFF;
        
        src1_i16[i] = i * 2;
        src2_i16[i] = i * 3;
        
        src1_i32[i] = i * 4;
        src2_i32[i] = i * 5;
        
        src1_i64[i] = i * 6LL;
        src2_i64[i] = i * 7LL;
        
        src1_f32[i] = i * 0.5f;
        src2_f32[i] = i * 0.75f;
        
        src1_f64[i] = i * 0.25;
        src2_f64[i] = i * 0.125;
    }
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("Testing V64QImode (64x8-bit integers)...\n");
    test_v64qimode(src1_u8, src2_u8, dst_u8, ARRAY_SIZE);
    
    printf("Testing V32HImode (32x16-bit integers)...\n");
    test_v32himode(src1_i16, src2_i16, dst_i16, ARRAY_SIZE);
#endif /* __AVX512BW__ */

    printf("Testing V16SImode (16x32-bit integers)...\n");
    test_v16simode(src1_i32, src2_i32, dst_i32, ARRAY_SIZE);
    
    printf("Testing V8DImode (8x64-bit integers)...\n");
    test_v8dimode(src1_i64, src2_i64, dst_i64, ARRAY_SIZE);
    
    printf("Testing V8DFmode (8x double precision floats)...\n");
    test_v8dfmode(src1_f64, src2_f64, dst_f64, ARRAY_SIZE);
    
    printf("Testing V16SFmode (16x single precision floats)...\n");
    test_v16sfmode(src1_f32, src2_f32, dst_f32, ARRAY_SIZE);
    
#ifdef __AVX512FP16__
    printf("Testing V32HFmode (32x half-precision floats)...\n");
    _Float16 src1_f16[ARRAY_SIZE];
    _Float16 src2_f16[ARRAY_SIZE];
    _Float16 dst_f16[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src1_f16[i] = (_Float16)(i * 0.1f);
        src2_f16[i] = (_Float16)(i * 0.2f);
    }
    test_v32hfmode(src1_f16, src2_f16, dst_f16, ARRAY_SIZE);
#endif
    
#ifdef __AVX512BF16__
    printf("Testing V32BFmode (32x bfloat16)...\n");
    __bfloat16 src1_bf16[ARRAY_SIZE];
    __bfloat16 src2_bf16[ARRAY_SIZE];
    __bfloat16 dst_bf16[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Simple bfloat16 initialization */
        uint16_t val1 = (i & 0x7FFF) | 0x3F00; /* ~1.0 in bfloat16 */
        uint16_t val2 = (i & 0x7FFF) | 0x4000; /* ~2.0 in bfloat16 */
        memcpy(&src1_bf16[i], &val1, sizeof(__bfloat16));
        memcpy(&src2_bf16[i], &val2, sizeof(__bfloat16));
    }
    test_v32bfmode(src1_bf16, src2_bf16, dst_bf16, ARRAY_SIZE);
#endif
    
    /* Calculate checksum to prevent dead code elimination */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst_u8[i];
        checksum += dst_i16[i];
        checksum += dst_i32[i];
        checksum += dst_i64[i];
        checksum += (uint64_t)dst_f32[i];
        checksum += (uint64_t)dst_f64[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    printf("All AVX-512 blend tests completed.\n");
    
#else
    printf("AVX-512 not supported on this platform.\n");
#endif /* __AVX512F__ */
    
    return 0;
}
