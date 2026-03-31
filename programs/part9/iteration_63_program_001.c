/* avx512_blend_coverage.c
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512vl -fdump-rtl-expand -o avx512_test avx512_blend_coverage.c
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent dead code elimination */
__attribute__((noinline, noipa)) 
void use_result(void* ptr) {
    /* Use inline assembly as a sink to prevent optimization */
    __asm__ volatile ("" : : "r"(ptr) : "memory");
}

/* Simple LCG for pseudo-random values */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize arrays with pseudo-random data */
static void init_array(void* arr, size_t size) {
    uint32_t* ptr = (uint32_t*)arr;
    size_t words = (size + 3) / 4;
    for (size_t i = 0; i < words; i++) {
        ptr[i] = lcg_rand();
    }
}

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QI: 64-byte integer blend */
static void test_v64qi_blend(void) {
    const size_t N = 1024;
    uint8_t src1[N], src2[N], dst[N];
    
    init_array(src1, N);
    init_array(src2, N);
    
    for (size_t i = 0; i < N; i += 64) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)&src1[i]);
        __m512i v2 = _mm512_loadu_si512((__m512i*)&src2[i]);
        
        /* Create data-dependent mask: blend where src1[i] < 128 */
        __mmask64 mask = _mm512_cmplt_epu8_mask(v1, _mm512_set1_epi8(128));
        
        __m512i result = _mm512_mask_blend_epi8(mask, v2, v1);
        _mm512_storeu_si512((__m512i*)&dst[i], result);
    }
    
    use_result(dst);
}

/* V32HI: 32-word integer blend */
static void test_v32hi_blend(void) {
    const size_t N = 1024;
    uint16_t src1[N], src2[N], dst[N];
    
    init_array(src1, N * sizeof(uint16_t));
    init_array(src2, N * sizeof(uint16_t));
    
    for (size_t i = 0; i < N; i += 32) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)&src1[i]);
        __m512i v2 = _mm512_loadu_si512((__m512i*)&src2[i]);
        
        /* Data-dependent mask: blend where src1[i] < 32768 */
        __mmask32 mask = _mm512_cmplt_epu16_mask(v1, _mm512_set1_epi16(32768));
        
        __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
        _mm512_storeu_si512((__m512i*)&dst[i], result);
    }
    
    use_result(dst);
}

/* V32HF: 32 half-float blend */
static void test_v32hf_blend(void) {
    const size_t N = 1024;
    _Float16 src1[N], src2[N], dst[N];
    
    /* Initialize with random floats converted to half */
    for (size_t i = 0; i < N; i++) {
        float f1 = (float)(lcg_rand() % 1000) / 100.0f;
        float f2 = (float)(lcg_rand() % 1000) / 100.0f;
        src1[i] = (_Float16)f1;
        src2[i] = (_Float16)f2;
    }
    
    for (size_t i = 0; i < N; i += 32) {
        __m512h v1 = _mm512_loadu_ph(&src1[i]);
        __m512h v2 = _mm512_loadu_ph(&src2[i]);
        
        /* Data-dependent mask: blend where v1 < 5.0 */
        __mmask32 mask = _mm512_cmplt_ph_mask(v1, _mm512_set1_ph((_Float16)5.0f));
        
        __m512h result = _mm512_mask_blend_ph(mask, v2, v1);
        _mm512_storeu_ph(&dst[i], result);
    }
    
    use_result(dst);
}

/* V32BF: 32 bfloat16 blend */
static void test_v32bf_blend(void) {
    const size_t N = 1024;
    __bf16 src1[N], src2[N], dst[N];
    
    /* Initialize with random floats converted to bfloat16 */
    for (size_t i = 0; i < N; i++) {
        float f1 = (float)(lcg_rand() % 1000) / 100.0f;
        float f2 = (float)(lcg_rand() % 1000) / 100.0f;
        src1[i] = (__bf16)f1;
        src2[i] = (__bf16)f2;
    }
    
    for (size_t i = 0; i < N; i += 32) {
        /* Load as epi16 and cast to bfloat16 vector */
        __m512i v1_i = _mm512_loadu_si512((__m512i*)&src1[i]);
        __m512i v2_i = _mm512_loadu_si512((__m512i*)&src2[i]);
        __m512bh v1 = (__m512bh)v1_i;
        __m512bh v2 = (__m512bh)v2_i;
        
        /* Create comparison mask using float conversion */
        __m512 v1_f = _mm512_cvtpbh_ps(v1);
        __m512 v2_f = _mm512_cvtpbh_ps(v2);
        __mmask16 mask_lo = _mm512_cmp_ps_mask(v1_f, _mm512_set1_ps(5.0f), _CMP_LT_OQ);
        
        /* Expand to 32-bit mask for bfloat16 blend */
        __mmask32 mask = _mm512_kunpackd(mask_lo, mask_lo);
        
        /* Blend using the same intrinsic as half-float */
        __m512bh result = _mm512_mask_blend_ph(mask, v2, v1);
        _mm512_storeu_si512((__m512i*)&dst[i], (__m512i)result);
    }
    
    use_result(dst);
}

/* V16SI: 16 dword integer blend */
static void test_v16si_blend(void) {
    const size_t N = 1024;
    int32_t src1[N], src2[N], dst[N];
    
    init_array(src1, N * sizeof(int32_t));
    init_array(src2, N * sizeof(int32_t));
    
    for (size_t i = 0; i < N; i += 16) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)&src1[i]);
        __m512i v2 = _mm512_loadu_si512((__m512i*)&src2[i]);
        
        /* Data-dependent mask: blend where src1[i] < 0 */
        __mmask16 mask = _mm512_cmplt_epi32_mask(v1, _mm512_setzero_si512());
        
        __m512i result = _mm512_mask_blend_epi32(mask, v2, v1);
        _mm512_storeu_si512((__m512i*)&dst[i], result);
    }
    
    use_result(dst);
}

/* V8DI: 8 qword integer blend */
static void test_v8di_blend(void) {
    const size_t N = 1024;
    int64_t src1[N], src2[N], dst[N];
    
    init_array(src1, N * sizeof(int64_t));
    init_array(src2, N * sizeof(int64_t));
    
    for (size_t i = 0; i < N; i += 8) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)&src1[i]);
        __m512i v2 = _mm512_loadu_si512((__m512i*)&src2[i]);
        
        /* Data-dependent mask: blend where src1[i] < 0 */
        __mmask8 mask = _mm512_cmplt_epi64_mask(v1, _mm512_setzero_si512());
        
        __m512i result = _mm512_mask_blend_epi64(mask, v2, v1);
        _mm512_storeu_si512((__m512i*)&dst[i], result);
    }
    
    use_result(dst);
}

/* V8DF: 8 double-float blend */
static void test_v8df_blend(void) {
    const size_t N = 1024;
    double src1[N], src2[N], dst[N];
    
    /* Initialize with random doubles */
    for (size_t i = 0; i < N; i++) {
        src1[i] = (double)(lcg_rand() % 1000) / 100.0;
        src2[i] = (double)(lcg_rand() % 1000) / 100.0;
    }
    
    for (size_t i = 0; i < N; i += 8) {
        __m512d v1 = _mm512_loadu_pd(&src1[i]);
        __m512d v2 = _mm512_loadu_pd(&src2[i]);
        
        /* Data-dependent mask: blend where v1 < 5.0 */
        __mmask8 mask = _mm512_cmp_pd_mask(v1, _mm512_set1_pd(5.0), _CMP_LT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, v2, v1);
        _mm512_storeu_pd(&dst[i], result);
    }
    
    use_result(dst);
}

/* V16SF: 16 single-float blend */
static void test_v16sf_blend(void) {
    const size_t N = 1024;
    float src1[N], src2[N], dst[N];
    
    /* Initialize with random floats */
    for (size_t i = 0; i < N; i++) {
        src1[i] = (float)(lcg_rand() % 1000) / 100.0f;
        src2[i] = (float)(lcg_rand() % 1000) / 100.0f;
    }
    
    for (size_t i = 0; i < N; i += 16) {
        __m512 v1 = _mm512_loadu_ps(&src1[i]);
        __m512 v2 = _mm512_loadu_ps(&src2[i]);
        
        /* Data-dependent mask: blend where v1 < 5.0 */
        __mmask16 mask = _mm512_cmp_ps_mask(v1, _mm512_set1_ps(5.0f), _CMP_LT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, v2, v1);
        _mm512_storeu_ps(&dst[i], result);
    }
    
    use_result(dst);
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

int main(void) {
    /* Runtime CPU feature detection */
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (!has_avx512f || !has_avx512bw) {
        printf("AVX-512F and AVX-512BW not supported on this CPU\n");
        printf("AVX-512F: %s, AVX-512BW: %s\n", 
               has_avx512f ? "yes" : "no",
               has_avx512bw ? "yes" : "no");
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
    
    printf("All AVX-512 blend tests completed.\n");
#endif
#endif
    
    return 0;
}
