#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
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

// Function prototypes with target attributes
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

// Data-dependent computation functions
__attribute__((target("avx512bw")))
__m512i compute_v64qi_blend(__m512i data1, __m512i data2, int seed) {
    // Create runtime-dependent mask using arithmetic
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i + seed) % 3 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    // Force mask to be non-constant
    volatile int vseed = seed;
    if (vseed & 1) {
        mask ^= 0xAAAAAAAAAAAAAAAAULL;
    }
    
    return blend_v64qi(data1, data2, mask);
}

__attribute__((target("avx512bw")))
__m512i compute_v32hi_blend(__m512i data1, __m512i data2, int seed) {
    // Compare vectors to generate predicate
    __m512i cmp = _mm512_cmpgt_epi16_mask(data1, data2);
    
    // Modify mask based on runtime value
    __mmask32 mask = cmp;
    for (int i = 0; i < 32; i++) {
        if ((i * seed) % 5 == 0) {
            mask ^= (1U << i);
        }
    }
    
    return blend_v32hi(data1, data2, mask);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h compute_v32hf_blend(__m512h data1, __m512h data2, int seed) {
    // Generate mask from comparison
    __mmask32 mask = _mm512_cmp_ph_mask(data1, data2, _CMP_GT_OQ);
    
    // Make mask data-dependent
    volatile int vseed = seed;
    __mmask32 dynamic_mask = mask;
    for (int i = 0; i < 32; i++) {
        if ((vseed >> (i & 7)) & 1) {
            dynamic_mask ^= (1U << i);
        }
    }
    
    return blend_v32hf(data1, data2, dynamic_mask);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh compute_v32bf_blend(__m512bh data1, __m512bh data2, int seed) {
    // Use arithmetic to create non-constant mask
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (((i * seed + 12345) % 7) < 3) {
            mask |= (1U << i);
        }
    }
    
    // Force mask computation
    volatile int temp = seed;
    if (temp & 2) {
        mask = ~mask;
    }
    
    return blend_v32bf(data1, data2, mask);
}

__attribute__((target("avx512f")))
__m512i compute_v16si_blend(__m512i data1, __m512i data2, int seed) {
    // Create mask from vector comparison
    __mmask16 mask = _mm512_cmpgt_epi32_mask(data1, data2);
    
    // Modify with runtime-dependent pattern
    for (int i = 0; i < 16; i++) {
        if ((seed + i) % 4 == 0) {
            mask ^= (1U << i);
        }
    }
    
    return blend_v16si(data1, data2, mask);
}

__attribute__((target("avx512f")))
__m512i compute_v8di_blend(__m512i data1, __m512i data2, int seed) {
    // Generate predicate mask
    __mmask8 mask = _mm512_cmpgt_epi64_mask(data1, data2);
    
    // Make it data-dependent
    volatile int vseed = seed;
    __mmask8 dynamic_mask = mask;
    for (int i = 0; i < 8; i++) {
        if ((vseed >> i) & 1) {
            dynamic_mask ^= (1U << i);
        }
    }
    
    return blend_v8di(data1, data2, dynamic_mask);
}

__attribute__((target("avx512f")))
__m512d compute_v8df_blend(__m512d data1, __m512d data2, int seed) {
    // Compare and create mask
    __mmask8 mask = _mm512_cmp_pd_mask(data1, data2, _CMP_GT_OQ);
    
    // Apply runtime modification
    for (int i = 0; i < 8; i++) {
        if (((seed * i) % 6) == 0) {
            mask ^= (1U << i);
        }
    }
    
    return blend_v8df(data1, data2, mask);
}

__attribute__((target("avx512f")))
__m512 compute_v16sf_blend(__m512 data1, __m512 data2, int seed) {
    // Generate comparison mask
    __mmask16 mask = _mm512_cmp_ps_mask(data1, data2, _CMP_GT_OQ);
    
    // Create data-dependent pattern
    __mmask16 dynamic_mask = mask;
    volatile int vseed = seed;
    for (int i = 0; i < 16; i++) {
        if ((vseed + i * 7) % 11 < 5) {
            dynamic_mask ^= (1U << i);
        }
    }
    
    return blend_v16sf(data1, data2, dynamic_mask);
}

// Multi-stage pipeline: V64QI -> V16SI conversion and blending
__attribute__((target("avx512bw,avx512f")))
int pipeline_v64qi_to_v16si(int seed) {
    // Stage 1: V64QI blending
    char data1[64], data2[64];
    for (int i = 0; i < 64; i++) {
        data1[i] = (char)((i * seed) & 0xFF);
        data2[i] = (char)((i * seed + 123) & 0xFF);
    }
    
    __m512i v1 = _mm512_loadu_si512(data1);
    __m512i v2 = _mm512_loadu_si512(data2);
    
    __m512i blended_qi = compute_v64qi_blend(v1, v2, seed);
    
    // Store to volatile to force materialization
    g_v64qi_result = blended_qi;
    
    // Stage 2: Convert to V16SI and blend
    __m512i v1_si = _mm512_cvtepi8_epi32(_mm512_extracti64x4_epi64(blended_qi, 0));
    __m512i v2_si = _mm512_cvtepi8_epi32(_mm512_extracti64x4_epi64(blended_qi, 1));
    
    // Create new data for second blend
    int data3[16], data4[16];
    for (int i = 0; i < 16; i++) {
        data3[i] = i * seed * 7;
        data4[i] = i * seed * 11;
    }
    
    __m512i v3 = _mm512_loadu_si512(data3);
    __m512i v4 = _mm512_loadu_si512(data4);
    
    __m512i blended_si = compute_v16si_blend(v3, v4, seed);
    g_v16si_result = blended_si;
    
    // Horizontal sum for checksum
    return _mm512_reduce_add_epi32(blended_si);
}

// Mixed precision pipeline: integer -> float conversion
__attribute__((target("avx512f")))
double pipeline_int_to_float(int seed) {
    // Start with integer data
    int data1[16], data2[16];
    for (int i = 0; i < 16; i++) {
        data1[i] = (i + seed) * 3;
        data2[i] = (i + seed) * 5;
    }
    
    __m512i v1_int = _mm512_loadu_si512(data1);
    __m512i v2_int = _mm512_loadu_si512(data2);
    
    // Blend integers
    __m512i blended_int = compute_v16si_blend(v1_int, v2_int, seed);
    
    // Convert to float and blend
    __m512 v1_float = _mm512_cvtepi32_ps(blended_int);
    
    float data3[16], data4[16];
    for (int i = 0; i < 16; i++) {
        data3[i] = (float)(i * seed) / 7.0f;
        data4[i] = (float)(i * seed) / 11.0f;
    }
    
    __m512 v2_float = _mm512_loadu_ps(data3);
    __m512 v3_float = _mm512_loadu_ps(data4);
    
    __m512 blended_float = compute_v16sf_blend(v2_float, v3_float, seed);
    g_v16sf_result = blended_float;
    
    // Convert to double and blend
    __m512d v1_double = _mm512_cvtps_pd(_mm512_extractf32x8_ps(blended_float, 0));
    
    double data5[8], data6[8];
    for (int i = 0; i < 8; i++) {
        data5[i] = (double)(i + seed) / 13.0;
        data6[i] = (double)(i + seed) / 17.0;
    }
    
    __m512d v2_double = _mm512_loadu_pd(data5);
    __m512d v3_double = _mm512_loadu_pd(data6);
    
    __m512d blended_double = compute_v8df_blend(v2_double, v3_double, seed);
    g_v8df_result = blended_double;
    
    return _mm512_reduce_add_pd(blended_double);
}

// Blend function implementations (force RTL expansion)
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Direct intrinsic usage for V64QI
    return _mm512_mask_blend_epi8(mask, a, b);
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Direct intrinsic usage for V32HI
    return _mm512_mask_blend_epi16(mask, a, b);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Direct intrinsic usage for V32HF
    return _mm512_mask_blend_ph(mask, a, b);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Direct intrinsic usage for V32BF
    return _mm512_mask_blend_ph(mask, a, b);
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Direct intrinsic usage for V16SI
    return _mm512_mask_blend_epi32(mask, a, b);
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Direct intrinsic usage for V8DI
    return _mm512_mask_blend_epi64(mask, a, b);
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Direct intrinsic usage for V8DF
    return _mm512_mask_blend_pd(mask, a, b);
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Direct intrinsic usage for V16SF
    return _mm512_mask_blend_ps(mask, a, b);
}

int main(int argc, char** argv) {
    // Use argc as seed for runtime variability
    int seed = argc;
    
    // Initialize test data arrays
    char qi_data1[64], qi_data2[64];
    short hi_data1[32], hi_data2[32];
    int si_data1[16], si_data2[16];
    long long di_data1[8], di_data2[8];
    float sf_data1[16], sf_data2[16];
    double df_data1[8], df_data2[8];
    
    // Initialize with pseudo-random values based on seed
    for (int i = 0; i < 64; i++) {
        qi_data1[i] = (char)((i * seed + 1) & 0xFF);
        qi_data2[i] = (char)((i * seed * 3 + 7) & 0xFF);
    }
    
    for (int i = 0; i < 32; i++) {
        hi_data1[i] = (short)((i * seed * 5) & 0xFFFF);
        hi_data2[i] = (short)((i * seed * 7 + 11) & 0xFFFF);
    }
    
    for (int i = 0; i < 16; i++) {
        si_data1[i] = i * seed * 13;
        si_data2[i] = i * seed * 17 + 19;
        sf_data1[i] = (float)(i * seed) / 23.0f;
        sf_data2[i] = (float)(i * seed + 29) / 31.0f;
    }
    
    for (int i = 0; i < 8; i++) {
        di_data1[i] = (long long)i * seed * 37;
        di_data2[i] = (long long)i * seed * 41 + 43;
        df_data1[i] = (double)(i * seed) / 47.0;
        df_data2[i] = (double)(i * seed + 53) / 59.0;
    }
    
    // Execute all blend operations
    __m512i v64qi_1 = _mm512_loadu_si512(qi_data1);
    __m512i v64qi_2 = _mm512_loadu_si512(qi_data2);
    __m512i blended_qi = compute_v64qi_blend(v64qi_1, v64qi_2, seed);
    g_v64qi_result = blended_qi;
    
    __m512i v32hi_1 = _mm512_loadu_si512(hi_data1);
    __m512i v32hi_2 = _mm512_loadu_si512(hi_data2);
    __m512i blended_hi = compute_v32hi_blend(v32hi_1, v32hi_2, seed);
    g_v32hi_result = blended_hi;
    
    // For half-precision, we need to convert from float
    __m512h v32hf_1 = _mm512_setzero_ph();
    __m512h v32hf_2 = _mm512_setzero_ph();
    // Initialize with simple pattern since conversion is complex
    for (int i = 0; i < 32; i++) {
        ((short*)&v32hf_1)[i] = (short)((i + seed) & 0xFFFF);
        ((short*)&v32hf_2)[i] = (short)((i * seed) & 0xFFFF);
    }
    __m512h blended_hf = compute_v32hf_blend(v32hf_1, v32hf_2, seed);
    g_v32hf_result = blended_hf;
    
    // Similarly for bfloat16
    __m512bh v32bf_1 = _mm512_setzero_bh();
    __m512bh v32bf_2 = _mm512_setzero_bh();
    for (int i = 0; i < 32; i++) {
        ((short*)&v32bf_1)[i] = (short)((i + seed * 2) & 0xFFFF);
        ((short*)&v32bf_2)[i] = (short)((i * seed * 3) & 0xFFFF);
    }
    __m512bh blended_bf = compute_v32bf_blend(v32bf_1, v32bf_2, seed);
    g_v32bf_result = blended_bf;
    
    __m512i v16si_1 = _mm512_loadu_si512(si_data1);
    __m512i v16si_2 = _mm512_loadu_si512(si_data2);
    __m512i blended_si = compute_v16si_blend(v16si_1, v16si_2, seed);
    g_v16si_result = blended_si;
    
    __m512i v8di_1 = _mm512_loadu_si512(di_data1);
    __m512i v8di_2 = _mm512_loadu_si512(di_data2);
    __m512i blended_di = compute_v8di_blend(v8di_1, v8di_2, seed);
    g_v8di_result = blended_di;
    
    __m512d v8df_1 = _mm512_loadu_pd(df_data1);
    __m512d v8df_2 = _mm512_loadu_pd(df_data2);
    __m512d blended_df = compute_v8df_blend(v8df_1, v8df_2, seed);
    g_v8df_result = blended_df;
    
    __m512 v16sf_1 = _mm512_loadu_ps(sf_data1);
    __m512 v16sf_2 = _mm512_loadu_ps(sf_data2);
    __m512 blended_sf = compute_v16sf_blend(v16sf_1, v16sf_2, seed);
    g_v16sf_result = blended_sf;
    
    // Execute multi-stage pipelines
    int pipeline1_result = pipeline_v64qi_to_v16si(seed);
    double pipeline2_result = pipeline_int_to_float(seed);
    
    // Compute checksum from all results
    long long checksum = 0;
    
    // Add integer results
    checksum += _mm512_reduce_add_epi64(blended_qi);
    checksum += _mm512_reduce_add_epi64(blended_hi);
    checksum += _mm512_reduce_add_epi64(blended_si);
    checksum += _mm512_reduce_add_epi64(blended_di);
    
    // Add pipeline results
    checksum += pipeline1_result;
    checksum += (long long)pipeline2_result;
    
    printf("Checksum: %lld\n", checksum);
    printf("All AVX-512 blend operations executed.\n");
    
    return 0;
}
