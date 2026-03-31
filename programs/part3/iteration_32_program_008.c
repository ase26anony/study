#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global volatile variables to prevent optimization
volatile __m512i g_v64qi_result;
volatile __m512i g_v32hi_result;
volatile __m512h g_v32hf_result;
volatile __m512bh g_v32bf_result;
volatile __m512i g_v16si_result;
volatile __m512i g_v8di_result;
volatile __m512d g_v8df_result;
volatile __m512 g_v16sf_result;

// Function prototypes with explicit target attributes
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask);

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask);

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask);

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask);

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask);

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask);

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask);

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask);

// Data-dependent computation to generate runtime masks
__attribute__((noinline))
__mmask64 compute_mask64(int seed) {
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i + seed) % 3 == 0) {
            mask |= (1ULL << i);
        }
    }
    return mask;
}

__attribute__((noinline))
__mmask32 compute_mask32(int seed) {
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i + seed) % 5 == 0) {
            mask |= (1U << i);
        }
    }
    return mask;
}

__attribute__((noinline))
__mmask16 compute_mask16(int seed) {
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if ((i + seed) % 7 == 0) {
            mask |= (1 << i);
        }
    }
    return mask;
}

__attribute__((noinline))
__mmask8 compute_mask8(int seed) {
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((i + seed) % 11 == 0) {
            mask |= (1 << i);
        }
    }
    return mask;
}

// V64QI: 64x 8-bit integers
__attribute__((target("avx512bw"), noinline))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Store to volatile to prevent elimination
    g_v64qi_result = result;
    
    // Additional data-dependent computation
    __m512i shifted = _mm512_slli_epi16(result, 1);
    return _mm512_xor_si512(result, shifted);
}

// V32HI: 32x 16-bit integers
__attribute__((target("avx512bw"), noinline))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Multi-stage pipeline: convert to 32-bit for next operation
    __m512i extended = _mm512_cvtepi16_epi32(_mm512_castsi512_si256(result));
    g_v32hi_result = _mm512_inserti64x4(result, _mm512_castsi512_si256(extended), 1);
    
    return result;
}

// V32HF: 32x half-precision floats (requires AVX512-FP16)
#ifdef __AVX512FP16__
__attribute__((target("avx512bw,avx512fp16"), noinline))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Mixed precision: convert to single precision and back
    __m512 singles = _mm512_cvtph_ps(result);
    __m512h converted_back = _mm512_cvtps_ph(singles, _MM_FROUND_CUR_DIRECTION);
    
    g_v32hf_result = converted_back;
    return result;
}
#endif

// V32BF: 32x bfloat16 (requires AVX512-BF16)
#ifdef __AVX512BF16__
__attribute__((target("avx512bw,avx512bf16"), noinline))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Convert to single precision for computation
    __m512 singles = _mm512_cvtneobf16_ps(result);
    __m512 scaled = _mm512_mul_ps(singles, _mm512_set1_ps(2.0f));
    
    // Store intermediate result
    _mm512_store_ps((float*)&g_v32bf_result, scaled);
    
    return result;
}
#endif

// V16SI: 16x 32-bit integers
__attribute__((target("avx512f"), noinline))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    // Conditional blending based on comparison
    __m512i cmp = _mm512_cmpgt_epi32_mask(result, _mm512_set1_epi32(0));
    __m512i alt = _mm512_abs_epi32(result);
    __m512i final = _mm512_mask_blend_epi32(cmp, result, alt);
    
    g_v16si_result = final;
    return final;
}

// V8DI: 8x 64-bit integers
__attribute__((target("avx512f"), noinline))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    // Chain with previous operation
    __m512i shifted = _mm512_slli_epi64(result, 2);
    __m512i blended = _mm512_mask_blend_epi64(mask >> 1, result, shifted);
    
    g_v8di_result = blended;
    return blended;
}

// V8DF: 8x double-precision floats
__attribute__((target("avx512f"), noinline))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    // Mixed type computation
    __m512d abs_result = _mm512_abs_pd(result);
    __m512d sign_mask = _mm512_castsi512_pd(_mm512_srai_epi64(_mm512_castpd_si512(result), 63));
    __m512d signed_abs = _mm512_xor_pd(abs_result, sign_mask);
    
    g_v8df_result = signed_abs;
    return result;
}

// V16SF: 16x single-precision floats
__attribute__((target("avx512f"), noinline))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Complex dataflow with multiple blends
    __m512 recip = _mm512_rcp14_ps(result);
    __m512 blended_recip = _mm512_mask_blend_ps(mask ^ 0xFFFF, result, recip);
    
    // Store to volatile through pointer
    volatile __m512* volatile_ptr = &g_v16sf_result;
    *volatile_ptr = blended_recip;
    
    return blended_recip;
}

// Initialize vectors with pseudo-random data based on seed
void init_vectors(int seed, 
                  __m512i* v64qi_a, __m512i* v64qi_b,
                  __m512i* v32hi_a, __m512i* v32hi_b,
                  __m512h* v32hf_a, __m512h* v32hf_b,
                  __m512bh* v32bf_a, __m512bh* v32bf_b,
                  __m512i* v16si_a, __m512i* v16si_b,
                  __m512i* v8di_a, __m512i* v8di_b,
                  __m512d* v8df_a, __m512d* v8df_b,
                  __m512* v16sf_a, __m512* v16sf_b) {
    
    // Simple LCG for reproducibility
    uint32_t lcg = seed;
    
    // Initialize 64x int8
    int8_t data64qi_a[64], data64qi_b[64];
    for (int i = 0; i < 64; i++) {
        lcg = lcg * 1103515245 + 12345;
        data64qi_a[i] = (int8_t)(lcg >> 16);
        data64qi_b[i] = (int8_t)(lcg >> 24);
    }
    *v64qi_a = _mm512_loadu_si512(data64qi_a);
    *v64qi_b = _mm512_loadu_si512(data64qi_b);
    
    // Initialize 32x int16
    int16_t data32hi_a[32], data32hi_b[32];
    for (int i = 0; i < 32; i++) {
        lcg = lcg * 1103515245 + 12345;
        data32hi_a[i] = (int16_t)(lcg >> 16);
        data32hi_b[i] = (int16_t)(lcg >> 24);
    }
    *v32hi_a = _mm512_loadu_si512(data32hi_a);
    *v32hi_b = _mm512_loadu_si512(data32hi_b);
    
#ifdef __AVX512FP16__
    // Initialize 32x half-precision
    _Float16 data32hf_a[32], data32hf_b[32];
    for (int i = 0; i < 32; i++) {
        lcg = lcg * 1103515245 + 12345;
        data32hf_a[i] = (_Float16)((lcg & 0xFFFF) / 65536.0f);
        data32hf_b[i] = (_Float16)(((lcg >> 16) & 0xFFFF) / 65536.0f);
    }
    *v32hf_a = _mm512_loadu_ph(data32hf_a);
    *v32hf_b = _mm512_loadu_ph(data32hf_b);
#endif
    
#ifdef __AVX512BF16__
    // Initialize 32x bfloat16
    __bf16 data32bf_a[32], data32bf_b[32];
    for (int i = 0; i < 32; i++) {
        lcg = lcg * 1103515245 + 12345;
        uint16_t val = lcg & 0xFFFF;
        data32bf_a[i] = *(const __bf16*)&val;
        data32bf_b[i] = *(const __bf16*)&((uint16_t)(lcg >> 16));
    }
    *v32bf_a = _mm512_loadu_si512(data32bf_a);
    *v32bf_b = _mm512_loadu_si512(data32bf_b);
#endif
    
    // Initialize 16x int32
    int32_t data16si_a[16], data16si_b[16];
    for (int i = 0; i < 16; i++) {
        lcg = lcg * 1103515245 + 12345;
        data16si_a[i] = (int32_t)lcg;
        data16si_b[i] = (int32_t)(lcg ^ 0xAAAAAAAA);
    }
    *v16si_a = _mm512_loadu_si512(data16si_a);
    *v16si_b = _mm512_loadu_si512(data16si_b);
    
    // Initialize 8x int64
    int64_t data8di_a[8], data8di_b[8];
    for (int i = 0; i < 8; i++) {
        lcg = lcg * 1103515245 + 12345;
        uint64_t val1 = ((uint64_t)lcg << 32) | (uint64_t)(lcg * 1664525 + 1013904223);
        uint64_t val2 = val1 ^ 0xAAAAAAAAAAAAAAAAULL;
        data8di_a[i] = (int64_t)val1;
        data8di_b[i] = (int64_t)val2;
    }
    *v8di_a = _mm512_loadu_si512(data8di_a);
    *v8di_b = _mm512_loadu_si512(data8di_b);
    
    // Initialize 8x double
    double data8df_a[8], data8df_b[8];
    for (int i = 0; i < 8; i++) {
        lcg = lcg * 1103515245 + 12345;
        data8df_a[i] = (double)(lcg & 0xFFFF) / 65536.0;
        data8df_b[i] = (double)((lcg >> 16) & 0xFFFF) / 65536.0;
    }
    *v8df_a = _mm512_loadu_pd(data8df_a);
    *v8df_b = _mm512_loadu_pd(data8df_b);
    
    // Initialize 16x float
    float data16sf_a[16], data16sf_b[16];
    for (int i = 0; i < 16; i++) {
        lcg = lcg * 1103515245 + 12345;
        data16sf_a[i] = (float)(lcg & 0xFFFF) / 65536.0f;
        data16sf_b[i] = (float)((lcg >> 16) & 0xFFFF) / 65536.0f;
    }
    *v16sf_a = _mm512_loadu_ps(data16sf_a);
    *v16sf_b = _mm512_loadu_ps(data16sf_b);
}

int main(int argc, char** argv) {
    // Use argc as seed for runtime variability
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    // Declare vectors
    __m512i v64qi_a, v64qi_b;
    __m512i v32hi_a, v32hi_b;
    __m512h v32hf_a, v32hf_b;
    __m512bh v32bf_a, v32bf_b;
    __m512i v16si_a, v16si_b;
    __m512i v8di_a, v8di_b;
    __m512d v8df_a, v8df_b;
    __m512 v16sf_a, v16sf_b;
    
    // Initialize with runtime-dependent data
    init_vectors(seed, &v64qi_a, &v64qi_b, &v32hi_a, &v32hi_b,
#ifdef __AVX512FP16__
                 &v32hf_a, &v32hf_b,
#else
                 NULL, NULL,
#endif
#ifdef __AVX512BF16__
                 &v32bf_a, &v32bf_b,
#else
                 NULL, NULL,
#endif
                 &v16si_a, &v16si_b, &v8di_a, &v8di_b,
                 &v8df_a, &v8df_b, &v16sf_a, &v16sf_b);
    
    // Compute runtime masks
    __mmask64 mask64 = compute_mask64(seed);
    __mmask32 mask32 = compute_mask32(seed);
    __mmask16 mask16 = compute_mask16(seed);
    __mmask8 mask8 = compute_mask8(seed);
    
    // Execute all blend operations in sequence
    __m512i r64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
    __m512i r32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
    
#ifdef __AVX512FP16__
    __m512h r32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
#endif
    
#ifdef __AVX512BF16__
    __m512bh r32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
#endif
    
    __m512i r16si = blend_v16si(v16si_a, v16si_b, mask16);
    __m512i r8di = blend_v8di(v8di_a, v8di_b, mask8);
    __m512d r8df = blend_v8df(v8df_a, v8df_b, mask8);
    __m512 r16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Compute checksum to verify execution
    uint64_t checksum = 0;
    
    // Extract values from results (forcing materialization)
    int8_t temp64qi[64];
    _mm512_storeu_si512(temp64qi, r64qi);
    for (int i = 0; i < 64; i++) {
        checksum += (uint8_t)temp64qi[i];
    }
    
    int16_t temp32hi[32];
    _mm512_storeu_si512(temp32hi, r32hi);
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)temp32hi[i];
    }
    
    int32_t temp16si[16];
    _mm512_storeu_si512(temp16si, r16si);
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)temp16si[i];
    }
    
    int64_t temp8di[8];
    _mm512_storeu_si512(temp8di, r8di);
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)temp8di[i];
    }
    
    double temp8df[8];
    _mm512_storeu_pd(temp8df, r8df);
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)(temp8df[i] * 1000);
    }
    
    float temp16sf[16];
    _mm512_storeu_ps(temp16sf, r16sf);
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)(temp16sf[i] * 1000);
    }
    
    printf("Checksum: %lu\n", checksum);
    return 0;
}
