/* AVX-512 Blend Coverage Test for i386-expand.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Compile-time feature checks */
#ifdef __AVX512F__
#define HAS_AVX512F 1
#else
#define HAS_AVX512F 0
#endif

#ifdef __AVX512BW__
#define HAS_AVX512BW 1
#else
#define HAS_AVX512BW 0
#endif

#ifdef __AVX512FP16__
#define HAS_AVX512FP16 1
#else
#define HAS_AVX512FP16 0
#endif

#ifdef __AVX512BF16__
#define HAS_AVX512BF16 1
#else
#define HAS_AVX512BF16 0
#endif

/* Function declarations with target attributes */
#ifdef __cplusplus
extern "C" {
#endif

/* V64QImode - 64 bytes */
__attribute__((target("avx512bw")))
void test_v64qimode(uint8_t* src1, uint8_t* src2, uint8_t* dst, int mask_seed) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    /* Create dynamic mask based on seed - prevents constant folding */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((mask_seed >> (i & 7)) & 1) {
            mask |= (1ULL << i);
        }
    }
    
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
}

/* V32HImode - 32 half-words */
__attribute__((target("avx512bw")))
void test_v32himode(int16_t* src1, int16_t* src2, int16_t* dst, int mask_seed) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    /* Dynamic mask generation */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        mask |= ((mask_seed >> (i & 3)) & 1) << i;
    }
    
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
}

/* V32HFmode - 32 half-precision floats (requires AVX512-FP16) */
__attribute__((target("avx512fp16,avx512bw")))
#if HAS_AVX512FP16
void test_v32hfmode(_Float16* src1, _Float16* src2, _Float16* dst, int mask_seed) {
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        mask |= ((mask_seed >> (i & 3)) & 1) << i;
    }
    
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    _mm512_storeu_ph(dst, result);
}
#else
void test_v32hfmode(_Float16* src1, _Float16* src2, _Float16* dst, int mask_seed) {
    /* Stub for compilation without AVX512-FP16 */
    (void)src1; (void)src2; (void)dst; (void)mask_seed;
}
#endif

/* V32BFmode - 32 bfloat16 (requires AVX512-BF16) */
__attribute__((target("avx512bf16,avx512bw")))
#if HAS_AVX512BF16
void test_v32bfmode(__bf16* src1, __bf16* src2, __bf16* dst, int mask_seed) {
    /* Use same intrinsic as FP16 but with bfloat16 data */
    __m512bh v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512bh v2 = _mm512_loadu_si512((__m512i*)src2);
    
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        mask |= ((mask_seed >> (i & 3)) & 1) << i;
    }
    
    __m512bh result = _mm512_mask_blend_ph(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
}
#else
void test_v32bfmode(__bf16* src1, __bf16* src2, __bf16* dst, int mask_seed) {
    /* Stub for compilation without AVX512-BF16 */
    (void)src1; (void)src2; (void)dst; (void)mask_seed;
}
#endif

/* V16SImode - 16 integers */
__attribute__((target("avx512f")))
void test_v16simode(int32_t* src1, int32_t* src2, int32_t* dst, int mask_seed) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        mask |= ((mask_seed >> (i & 3)) & 1) << i;
    }
    
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
}

/* V8DImode - 8 double integers */
__attribute__((target("avx512f")))
void test_v8dimode(int64_t* src1, int64_t* src2, int64_t* dst, int mask_seed) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        mask |= ((mask_seed >> (i & 3)) & 1) << i;
    }
    
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
}

/* V8DFmode - 8 doubles */
__attribute__((target("avx512f")))
void test_v8dfmode(double* src1, double* src2, double* dst, int mask_seed) {
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        mask |= ((mask_seed >> (i & 3)) & 1) << i;
    }
    
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    _mm512_storeu_pd(dst, result);
}

/* V16SFmode - 16 floats */
__attribute__((target("avx512f")))
void test_v16sfmode(float* src1, float* src2, float* dst, int mask_seed) {
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        mask |= ((mask_seed >> (i & 3)) & 1) << i;
    }
    
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    _mm512_storeu_ps(dst, result);
}

#ifdef __cplusplus
}
#endif

/* Helper function to initialize test data */
void init_test_data(void* data, size_t size, int seed) {
    uint8_t* ptr = (uint8_t*)data;
    for (size_t i = 0; i < size; i++) {
        ptr[i] = (uint8_t)((i + seed) * 3 + 1);
    }
}

/* Checksum function to prevent dead code elimination */
uint64_t compute_checksum(void* data, size_t size) {
    uint64_t sum = 0;
    uint8_t* ptr = (uint8_t*)data;
    for (size_t i = 0; i < size; i++) {
        sum += ptr[i];
    }
    return sum;
}

int main() {
    /* Test data buffers - aligned for better performance */
    uint8_t src1_bytes[64] __attribute__((aligned(64)));
    uint8_t src2_bytes[64] __attribute__((aligned(64)));
    uint8_t dst_bytes[64] __attribute__((aligned(64)));
    
    int16_t src1_words[32] __attribute__((aligned(64)));
    int16_t src2_words[32] __attribute__((aligned(64)));
    int16_t dst_words[32] __attribute__((aligned(64)));
    
    _Float16 src1_half[32] __attribute__((aligned(64)));
    _Float16 src2_half[32] __attribute__((aligned(64)));
    _Float16 dst_half[32] __attribute__((aligned(64)));
    
    __bf16 src1_bf16[32] __attribute__((aligned(64)));
    __bf16 src2_bf16[32] __attribute__((aligned(64)));
    __bf16 dst_bf16[32] __attribute__((aligned(64)));
    
    int32_t src1_dwords[16] __attribute__((aligned(64)));
    int32_t src2_dwords[16] __attribute__((aligned(64)));
    int32_t dst_dwords[16] __attribute__((aligned(64)));
    
    int64_t src1_qwords[8] __attribute__((aligned(64)));
    int64_t src2_qwords[8] __attribute__((aligned(64)));
    int64_t dst_qwords[8] __attribute__((aligned(64)));
    
    double src1_double[8] __attribute__((aligned(64)));
    double src2_double[8] __attribute__((aligned(64)));
    double dst_double[8] __attribute__((aligned(64)));
    
    float src1_float[16] __attribute__((aligned(64)));
    float src2_float[16] __attribute__((aligned(64)));
    float dst_float[16] __attribute__((aligned(64)));
    
    /* Initialize all test data with different seeds */
    init_test_data(src1_bytes, sizeof(src1_bytes), 1);
    init_test_data(src2_bytes, sizeof(src2_bytes), 2);
    
    init_test_data(src1_words, sizeof(src1_words), 3);
    init_test_data(src2_words, sizeof(src2_words), 4);
    
    init_test_data(src1_half, sizeof(src1_half), 5);
    init_test_data(src2_half, sizeof(src2_half), 6);
    
    init_test_data(src1_bf16, sizeof(src1_bf16), 7);
    init_test_data(src2_bf16, sizeof(src2_bf16), 8);
    
    init_test_data(src1_dwords, sizeof(src1_dwords), 9);
    init_test_data(src2_dwords, sizeof(src2_dwords), 10);
    
    init_test_data(src1_qwords, sizeof(src1_qwords), 11);
    init_test_data(src2_qwords, sizeof(src2_qwords), 12);
    
    init_test_data(src1_double, sizeof(src1_double), 13);
    init_test_data(src2_double, sizeof(src2_double), 14);
    
    init_test_data(src1_float, sizeof(src1_float), 15);
    init_test_data(src2_float, sizeof(src2_float), 16);
    
    /* Run tests with different mask seeds in a loop */
    uint64_t total_checksum = 0;
    
    for (int iter = 0; iter < 10; iter++) {
        int mask_seed = iter * 7 + 123;
        
        /* Test each vector mode */
        test_v64qimode(src1_bytes, src2_bytes, dst_bytes, mask_seed);
        total_checksum += compute_checksum(dst_bytes, sizeof(dst_bytes));
        
        test_v32himode(src1_words, src2_words, dst_words, mask_seed);
        total_checksum += compute_checksum(dst_words, sizeof(dst_words));
        
        #if HAS_AVX512FP16
        test_v32hfmode(src1_half, src2_half, dst_half, mask_seed);
        total_checksum += compute_checksum(dst_half, sizeof(dst_half));
        #endif
        
        #if HAS_AVX512BF16
        test_v32bfmode(src1_bf16, src2_bf16, dst_bf16, mask_seed);
        total_checksum += compute_checksum(dst_bf16, sizeof(dst_bf16));
        #endif
        
        test_v16simode(src1_dwords, src2_dwords, dst_dwords, mask_seed);
        total_checksum += compute_checksum(dst_dwords, sizeof(dst_dwords));
        
        test_v8dimode(src1_qwords, src2_qwords, dst_qwords, mask_seed);
        total_checksum += compute_checksum(dst_qwords, sizeof(dst_qwords));
        
        test_v8dfmode(src1_double, src2_double, dst_double, mask_seed);
        total_checksum += compute_checksum(dst_double, sizeof(dst_double));
        
        test_v16sfmode(src1_float, src2_float, dst_float, mask_seed);
        total_checksum += compute_checksum(dst_float, sizeof(dst_float));
    }
    
    printf("Total checksum: %lu\n", total_checksum);
    printf("All AVX-512 blend tests completed.\n");
    
    return 0;
}
