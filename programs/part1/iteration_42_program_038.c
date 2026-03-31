#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
int test_v64qi_blend() {
    char src1[64] __attribute__((aligned(64)));
    char src2[64] __attribute__((aligned(64)));
    char dst[64] __attribute__((aligned(64)));
    
    // Initialize with pattern data
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 - i;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmp_epi8_mask(v1, v2, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv64qi
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    // Compute checksum to prevent optimization
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
int test_v32hi_blend() {
    short src1[32] __attribute__((aligned(64)));
    short src2[32] __attribute__((aligned(64)));
    short dst[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 2;
        src2[i] = i * 3;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Generate mask: select where src1 > src2
    __mmask32 mask = _mm512_cmp_epi16_mask(v1, v2, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
float test_v32hf_blend() {
    _Float16 src1[32] __attribute__((aligned(64)));
    _Float16 src2[32] __attribute__((aligned(64)));
    _Float16 dst[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (i % 8) * 0.5f;
        src2[i] = (i % 4) * 0.75f;
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    // Generate mask using comparison
    __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_ph(dst, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
float test_v32bf_blend() {
    __bfloat16 src1[32] __attribute__((aligned(64)));
    __bfloat16 src2[32] __attribute__((aligned(64)));
    __bfloat16 dst[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = bfloat16_from_float((i % 8) * 0.5f);
        src2[i] = bfloat16_from_float((i % 4) * 0.75f);
    }
    
    __m512bh v1 = _mm512_load_si512((__m512i*)src1);
    __m512bh v2 = _mm512_load_si512((__m512i*)src2);
    
    // For bfloat16, we need to use integer blend since there's no direct bfloat16 blend
    // This should still trigger the appropriate blend instruction
    __mmask32 mask = 0xAAAAAAAA;  // Alternating pattern
    
    // Use epi16 blend for bfloat16 data (stored as 16-bit integers)
    __m512i result = _mm512_mask_blend_epi16(mask, 
        (__m512i)v1, (__m512i)v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(dst[i]);
    }
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
int test_v16si_blend() {
    int src1[16] __attribute__((aligned(64)));
    int src2[16] __attribute__((aligned(64)));
    int dst[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 10;
        src2[i] = i * 15;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Generate mask using comparison
    __mmask16 mask = _mm512_cmp_epi32_mask(v1, v2, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
long long test_v8di_blend() {
    long long src1[8] __attribute__((aligned(64)));
    long long src2[8] __attribute__((aligned(64)));
    long long dst[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 100LL;
        src2[i] = i * 150LL;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Generate mask using comparison
    __mmask8 mask = _mm512_cmp_epi64_mask(v1, v2, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    return sum;
}

/* ==================== V8DFmode (8-double-precision floats) ==================== */
double test_v8df_blend() {
    double src1[8] __attribute__((aligned(64)));
    double src2[8] __attribute__((aligned(64)));
    double dst[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 2.0;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    // Generate mask using comparison
    __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_store_pd(dst, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    return sum;
}

/* ==================== V16SFmode (16-single-precision floats) ==================== */
float test_v16sf_blend() {
    float src1[16] __attribute__((aligned(64)));
    float src2[16] __attribute__((aligned(64)));
    float dst[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 0.75f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    // Generate mask using comparison
    __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_store_ps(dst, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
double test_mixed_blends(int iterations) {
    double total = 0.0;
    
    // Allocate aligned arrays for different data types
    char     c_data1[64] __attribute__((aligned(64)));
    char     c_data2[64] __attribute__((aligned(64)));
    short    s_data1[32] __attribute__((aligned(64)));
    short    s_data2[32] __attribute__((aligned(64)));
    int      i_data1[16] __attribute__((aligned(64)));
    int      i_data2[16] __attribute__((aligned(64)));
    float    f_data1[16] __attribute__((aligned(64)));
    float    f_data2[16] __attribute__((aligned(64)));
    double   d_data1[8]  __attribute__((aligned(64)));
    double   d_data2[8]  __attribute__((aligned(64)));
    
    for (int iter = 0; iter < iterations; iter++) {
        // Initialize with different patterns each iteration
        for (int i = 0; i < 64; i++) {
            c_data1[i] = (i + iter) % 128;
            c_data2[i] = (64 - i + iter) % 128;
        }
        for (int i = 0; i < 32; i++) {
            s_data1[i] = (i * 2 + iter) % 256;
            s_data2[i] = (i * 3 + iter) % 256;
        }
        for (int i = 0; i < 16; i++) {
            i_data1[i] = (i * 10 + iter) % 1024;
            i_data2[i] = (i * 15 + iter) % 1024;
            f_data1[i] = (i * 0.5f + iter * 0.1f);
            f_data2[i] = (i * 0.75f + iter * 0.1f);
        }
        for (int i = 0; i < 8; i++) {
            d_data1[i] = (i * 1.5 + iter * 0.1);
            d_data2[i] = (i * 2.0 + iter * 0.1);
        }
        
        // Perform all blend operations
        __m512i vc1 = _mm512_load_si512((__m512i*)c_data1);
        __m512i vc2 = _mm512_load_si512((__m512i*)c_data2);
        __mmask64 mask64 = _mm512_cmp_epi8_mask(vc1, vc2, _MM_CMPINT_GT);
        __m512i rc = _mm512_mask_blend_epi8(mask64, vc1, vc2);
        
        __m512i vs1 = _mm512_load_si512((__m512i*)s_data1);
        __m512i vs2 = _mm512_load_si512((__m512i*)s_data2);
        __mmask32 mask32 = _mm512_cmp_epi16_mask(vs1, vs2, _MM_CMPINT_GT);
        __m512i rs = _mm512_mask_blend_epi16(mask32, vs1, vs2);
        
        __m512i vi1 = _mm512_load_si512((__m512i*)i_data1);
        __m512i vi2 = _mm512_load_si512((__m512i*)i_data2);
        __mmask16 mask16i = _mm512_cmp_epi32_mask(vi1, vi2, _MM_CMPINT_GT);
        __m512i ri = _mm512_mask_blend_epi32(mask16i, vi1, vi2);
        
        __m512 vf1 = _mm512_load_ps(f_data1);
        __m512 vf2 = _mm512_load_ps(f_data2);
        __mmask16 mask16f = _mm512_cmp_ps_mask(vf1, vf2, _CMP_GT_OQ);
        __m512 rf = _mm512_mask_blend_ps(mask16f, vf1, vf2);
        
        __m512d vd1 = _mm512_load_pd(d_data1);
        __m512d vd2 = _mm512_load_pd(d_data2);
        __mmask8 mask8 = _mm512_cmp_pd_mask(vd1, vd2, _CMP_GT_OQ);
        __m512d rd = _mm512_mask_blend_pd(mask8, vd1, vd2);
        
        // Accumulate results to prevent optimization
        char c_temp[64];
        short s_temp[32];
        int i_temp[16];
        float f_temp[16];
        double d_temp[8];
        
        _mm512_store_si512((__m512i*)c_temp, rc);
        _mm512_store_si512((__m512i*)s_temp, rs);
        _mm512_store_si512((__m512i*)i_temp, ri);
        _mm512_store_ps(f_temp, rf);
        _mm512_store_pd(d_temp, rd);
        
        for (int i = 0; i < 8; i++) {
            total += d_temp[i] + f_temp[i] + i_temp[i] + s_temp[i*2] + c_temp[i*8];
        }
    }
    
    return total;
}

#endif  // __AVX512BW__
#endif  // __AVX512F__

/* ==================== Scalar Fallbacks ==================== */
int scalar_test_v64qi_blend() {
    char src1[64];
    char src2[64];
    char dst[64];
    
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 - i;
        dst[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    return sum;
}

float scalar_test_v16sf_blend() {
    float src1[16];
    float src2[16];
    float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 0.75f;
        dst[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    return sum;
}

double scalar_test_v8df_blend() {
    double src1[8];
    double src2[8];
    double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 2.0;
        dst[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    return sum;
}

/* ==================== Main Function ==================== */
int main() {
    long long total_result = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running optimized blend tests...\n");
    
    // Test all vector modes
    total_result += test_v64qi_blend();
    total_result += test_v32hi_blend();
    
#ifdef __AVX512FP16__
    total_result += (long long)test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total_result += (long long)test_v32bf_blend();
#endif
    
    total_result += test_v16si_blend();
    total_result += test_v8di_blend();
    total_result += (long long)test_v8df_blend();
    total_result += (long long)test_v16sf_blend();
    
    // Test mixed blends in loop
    double mixed_result = test_mixed_blends(10);
    total_result += (long long)mixed_result;
    
#else
    printf("AVX-512BW not available. Using scalar fallbacks...\n");
#endif
#else
    printf("AVX-512 not available. Using scalar fallbacks...\n");
#endif

#ifndef __AVX512F__
    // Scalar fallbacks
    total_result += scalar_test_v64qi_blend();
    total_result += (long long)scalar_test_v16sf_blend();
    total_result += (long long)scalar_test_v8df_blend();
#endif

    printf("Total checksum: %lld\n", total_result);
    
    // Return non-zero if we got a non-zero result (should always be true)
    return total_result != 0 ? 0 : 1;
}
