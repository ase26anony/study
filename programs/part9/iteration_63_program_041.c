#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa)) 
void use_result(void* ptr) {
    /* Use inline assembly as a memory clobber to prevent optimization */
    asm volatile ("" : : "r"(ptr) : "memory");
}

/* Simple LCG for pseudo-random values */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand() {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize arrays with pseudo-random data */
static void init_array(void* arr, size_t size) {
    uint8_t* ptr = (uint8_t*)arr;
    for (size_t i = 0; i < size; i++) {
        ptr[i] = lcg_rand() & 0xFF;
    }
}

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QI: 64-byte integer vectors */
static void test_v64qi_blend() {
    uint8_t src1[64 * 4] __attribute__((aligned(64)));
    uint8_t src2[64 * 4] __attribute__((aligned(64)));
    uint8_t dst[64 * 4] __attribute__((aligned(64)));
    
    init_array(src1, sizeof(src1));
    init_array(src2, sizeof(src2));
    
    for (int i = 0; i < 4; i++) {
        __m512i v1 = _mm512_load_si512((__m512i*)(src1 + i * 64));
        __m512i v2 = _mm512_load_si512((__m512i*)(src2 + i * 64));
        
        /* Data-dependent mask: compare elements for inequality */
        __mmask64 mask = _mm512_cmpeq_epi8_mask(v1, v2);
        /* Invert mask to ensure it's not all zeros or all ones */
        mask = ~mask;
        
        /* Blend based on the computed mask */
        __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
        _mm512_store_si512((__m512i*)(dst + i * 64), result);
    }
    
    use_result(dst);
}

/* V32HI: 32-word integer vectors */
static void test_v32hi_blend() {
    uint16_t src1[32 * 8] __attribute__((aligned(64)));
    uint16_t src2[32 * 8] __attribute__((aligned(64)));
    uint16_t dst[32 * 8] __attribute__((aligned(64)));
    
    init_array(src1, sizeof(src1));
    init_array(src2, sizeof(src2));
    
    for (int i = 0; i < 8; i++) {
        __m512i v1 = _mm512_load_si512((__m512i*)(src1 + i * 32));
        __m512i v2 = _mm512_load_si512((__m512i*)(src2 + i * 32));
        
        /* Data-dependent mask: compare for less-than */
        __mmask32 mask = _mm512_cmplt_epi16_mask(v1, v2);
        /* Ensure mask is not trivial */
        mask = mask ^ 0xAAAAAAAA;  /* XOR with pattern */
        
        __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
        _mm512_store_si512((__m512i*)(dst + i * 32), result);
    }
    
    use_result(dst);
}

/* V32HF: 32 half-precision float vectors */
static void test_v32hf_blend() {
    _Float16 src1[32 * 8] __attribute__((aligned(64)));
    _Float16 src2[32 * 8] __attribute__((aligned(64)));
    _Float16 dst[32 * 8] __attribute__((aligned(64)));
    
    /* Initialize with float values converted to half */
    for (size_t i = 0; i < 32 * 8; i++) {
        float val = (lcg_rand() % 1000) / 100.0f;
        src1[i] = (_Float16)val;
        src2[i] = (_Float16)(val * 1.5f);
    }
    
    for (int i = 0; i < 8; i++) {
        __m512h v1 = _mm512_load_ph(src1 + i * 32);
        __m512h v2 = _mm512_load_ph(src2 + i * 32);
        
        /* Data-dependent mask: compare for equality with tolerance */
        __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_EQ_OQ);
        /* Modify mask to avoid trivial cases */
        mask = mask ^ (__mmask32)(lcg_rand() & 0xFFFFFFFF);
        
        __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
        _mm512_store_ph(dst + i * 32, result);
    }
    
    use_result(dst);
}

/* V32BF: 32 bfloat16 vectors */
static void test_v32bf_blend() {
    __bf16 src1[32 * 8] __attribute__((aligned(64)));
    __bf16 src2[32 * 8] __attribute__((aligned(64)));
    __bf16 dst[32 * 8] __attribute__((aligned(64)));
    
    /* Initialize bfloat16 values */
    for (size_t i = 0; i < 32 * 8; i++) {
        float val = (lcg_rand() % 1000) / 100.0f;
        uint32_t int_val = *(uint32_t*)&val;
        src1[i] = (__bf16)(int_val >> 16);
        src2[i] = (__bf16)((int_val + 0x1000) >> 16);
    }
    
    for (int i = 0; i < 8; i++) {
        /* Load as epi16 and cast to bfloat16 vector */
        __m512i v1_i = _mm512_load_si512((__m512i*)(src1 + i * 32));
        __m512i v2_i = _mm512_load_si512((__m512i*)(src2 + i * 32));
        
        __m512bh v1 = (__m512bh)v1_i;
        __m512bh v2 = (__m512bh)v2_i;
        
        /* Compare using integer comparison since bfloat16 lacks direct comparison */
        __mmask32 mask = _mm512_cmpeq_epi16_mask(v1_i, v2_i);
        mask = ~mask;  /* Invert to get non-trivial mask */
        
        /* Use the same blend intrinsic as for half-precision */
        __m512bh result = _mm512_mask_blend_ph(mask, v1, v2);
        _mm512_store_si512((__m512i*)(dst + i * 32), (__m512i)result);
    }
    
    use_result(dst);
}

/* V16SI: 16 dword integer vectors */
static void test_v16si_blend() {
    int32_t src1[16 * 16] __attribute__((aligned(64)));
    int32_t src2[16 * 16] __attribute__((aligned(64)));
    int32_t dst[16 * 16] __attribute__((aligned(64)));
    
    init_array(src1, sizeof(src1));
    init_array(src2, sizeof(src2));
    
    for (int i = 0; i < 16; i++) {
        __m512i v1 = _mm512_load_si512((__m512i*)(src1 + i * 16));
        __m512i v2 = _mm512_load_si512((__m512i*)(src2 + i * 16));
        
        /* Data-dependent mask: compare for greater-than */
        __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, v2);
        /* Mix with pattern */
        mask = mask ^ 0x5555;
        
        __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
        _mm512_store_si512((__m512i*)(dst + i * 16), result);
    }
    
    use_result(dst);
}

/* V8DI: 8 qword integer vectors */
static void test_v8di_blend() {
    int64_t src1[8 * 32] __attribute__((aligned(64)));
    int64_t src2[8 * 32] __attribute__((aligned(64)));
    int64_t dst[8 * 32] __attribute__((aligned(64)));
    
    init_array(src1, sizeof(src1));
    init_array(src2, sizeof(src2));
    
    for (int i = 0; i < 32; i++) {
        __m512i v1 = _mm512_load_si512((__m512i*)(src1 + i * 8));
        __m512i v2 = _mm512_load_si512((__m512i*)(src2 + i * 8));
        
        /* Data-dependent mask: compare for inequality */
        __mmask8 mask = _mm512_cmpneq_epi64_mask(v1, v2);
        /* Ensure mask is non-trivial */
        mask = mask ^ 0xAA;
        
        __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
        _mm512_store_si512((__m512i*)(dst + i * 8), result);
    }
    
    use_result(dst);
}

/* V8DF: 8 double-precision float vectors */
static void test_v8df_blend() {
    double src1[8 * 32] __attribute__((aligned(64)));
    double src2[8 * 32] __attribute__((aligned(64)));
    double dst[8 * 32] __attribute__((aligned(64)));
    
    /* Initialize with double values */
    for (size_t i = 0; i < 8 * 32; i++) {
        src1[i] = (lcg_rand() % 10000) / 100.0;
        src2[i] = src1[i] * 1.1;
    }
    
    for (int i = 0; i < 32; i++) {
        __m512d v1 = _mm512_load_pd(src1 + i * 8);
        __m512d v2 = _mm512_load_pd(src2 + i * 8);
        
        /* Data-dependent mask: compare for less-than */
        __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
        /* Modify mask */
        mask = mask ^ 0x55;
        
        __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
        _mm512_store_pd(dst + i * 8, result);
    }
    
    use_result(dst);
}

/* V16SF: 16 single-precision float vectors */
static void test_v16sf_blend() {
    float src1[16 * 16] __attribute__((aligned(64)));
    float src2[16 * 16] __attribute__((aligned(64)));
    float dst[16 * 16] __attribute__((aligned(64)));
    
    /* Initialize with float values */
    for (size_t i = 0; i < 16 * 16; i++) {
        src1[i] = (lcg_rand() % 10000) / 100.0f;
        src2[i] = src1[i] * 0.9f;
    }
    
    for (int i = 0; i < 16; i++) {
        __m512 v1 = _mm512_load_ps(src1 + i * 16);
        __m512 v2 = _mm512_load_ps(src2 + i * 16);
        
        /* Data-dependent mask: compare for greater-than-or-equal */
        __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_GE_OQ);
        /* Modify mask */
        mask = mask ^ 0xAAAA;
        
        __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
        _mm512_store_ps(dst + i * 16, result);
    }
    
    use_result(dst);
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

int main() {
    /* Runtime CPU feature detection */
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (!has_avx512f || !has_avx512bw) {
        printf("AVX-512F and/or AVX-512BW not supported on this CPU\n");
        printf("AVX-512F: %s\n", has_avx512f ? "yes" : "no");
        printf("AVX-512BW: %s\n", has_avx512bw ? "yes" : "no");
        return 0;
    }
    
    printf("Running AVX-512 blend tests...\n");
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    /* Execute all blend tests */
    test_v64qi_blend();
    test_v32hi_blend();
    test_v32hf_blend();
    test_v32bf_blend();
    test_v16si_blend();
    test_v8di_blend();
    test_v8df_blend();
    test_v16sf_blend();
    
    printf("All AVX-512 blend tests completed\n");
#else
    printf("Compiler does not support AVX-512BW intrinsics\n");
#endif
#else
    printf("Compiler does not support AVX-512F intrinsics\n");
#endif
    
    return 0;
}
