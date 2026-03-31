#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Volatile globals to prevent optimization
volatile __m512i v64qi_result;
volatile __m512i v32hi_result;
volatile __m512h v32hf_result;
volatile __m512bh v32bf_result;
volatile __m512i v16si_result;
volatile __m512i v8di_result;
volatile __m512d v8df_result;
volatile __m512 v16sf_result;

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

// V64QI: 64 x 8-bit integers
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Store to volatile to prevent elimination
    v64qi_result = result;
    
    // Additional computation to create data dependency
    __m512i shifted = _mm512_slli_epi16(result, 1);
    return _mm512_xor_si512(result, shifted);
}

// V32HI: 32 x 16-bit integers
__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Create conditional mask based on data comparison
    __mmask32 cmp_mask = _mm512_cmpgt_epi16_mask(a, b);
    __mmask32 blend_mask = mask & cmp_mask;
    
    __m512i result = _mm512_mask_blend_epi16(blend_mask, a, b);
    v32hi_result = result;
    
    // Convert to 32-bit for next stage
    __m512i extended = _mm512_srai_epi32(_mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(result, 0)), 2);
    return _mm512_add_epi32(result, extended);
}

// V32HF: 32 x half-precision floats (requires AVX512-FP16)
#ifdef __AVX512FP16__
__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Generate mask from comparison
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
    __mmask32 blend_mask = mask | cmp_mask;
    
    __m512h result = _mm512_mask_blend_ph(blend_mask, a, b);
    v32hf_result = result;
    
    // Convert to single precision for mixing
    __m512 singles = _mm512_cvtph_ps(result);
    return _mm512_cvtps_ph(singles, _MM_FROUND_TO_NEAREST_INT);
}
#endif

// V32BF: 32 x bfloat16 (requires AVX512-BF16)
#ifdef __AVX512BF16__
__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Convert to float, blend, convert back
    __m512 fa = _mm512_cvtneobf16_ps(a);
    __m512 fb = _mm512_cvtneobf16_ps(b);
    
    // Create mask from float comparison
    __mmask16 float_mask = _mm512_cmp_ps_mask(fa, fb, _CMP_GT_OQ);
    __mmask32 blend_mask = mask | (float_mask << 16) | float_mask;
    
    __m512bh result = _mm512_mask_blend_ph(blend_mask, a, b);
    v32bf_result = result;
    
    return result;
}
#endif

// V16SI: 16 x 32-bit integers
__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Complex mask generation from data
    __m512i diff = _mm512_sub_epi32(a, b);
    __mmask16 sign_mask = _mm512_cmplt_epi32_mask(diff, _mm512_setzero_si512());
    __mmask16 blend_mask = mask ^ sign_mask;
    
    __m512i result = _mm512_mask_blend_epi32(blend_mask, a, b);
    v16si_result = result;
    
    // Convert to double for next stage
    __m512d doubles_even = _mm512_cvtepi32_pd(_mm512_extracti32x8_epi32(result, 0));
    __m512d doubles_odd = _mm512_cvtepi32_pd(_mm512_extracti32x8_epi32(result, 1));
    
    // Force spill/reload
    volatile __m512d temp = doubles_even;
    return _mm512_add_epi32(result, _mm512_castpd_si512(temp));
}

// V8DI: 8 x 64-bit integers
__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Multi-stage blending
    __m512i xor_result = _mm512_xor_si512(a, b);
    __mmask8 nonzero_mask = _mm512_cmpneq_epi64_mask(xor_result, _mm512_setzero_si512());
    __mmask8 blend_mask = mask & nonzero_mask;
    
    __m512i result = _mm512_mask_blend_epi64(blend_mask, a, b);
    v8di_result = result;
    
    // Convert to float
    __m512 floats = _mm512_cvtepi64_ps(result);
    return _mm512_castps_si512(floats);
}

// V8DF: 8 x double-precision floats
__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Data-dependent mask
    __mmask8 nan_mask = _mm512_cmp_pd_mask(a, a, _CMP_UNORD_Q);
    __mmask8 finite_mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(1e308), _CMP_LT_OQ);
    __mmask8 blend_mask = mask & (~nan_mask) & finite_mask;
    
    __m512d result = _mm512_mask_blend_pd(blend_mask, a, b);
    v8df_result = result;
    
    // Convert to integer and back
    __m512i ints = _mm512_castpd_si512(result);
    return _mm512_castsi512_pd(_mm512_add_epi64(ints, _mm512_set1_epi64(1)));
}

// V16SF: 16 x single-precision floats
__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Complex conditional blending
    __m512 abs_a = _mm512_abs_ps(a);
    __m512 abs_b = _mm512_abs_ps(b);
    __mmask16 gt_mask = _mm512_cmp_ps_mask(abs_a, abs_b, _CMP_GT_OQ);
    __mmask16 lt_mask = _mm512_cmp_ps_mask(abs_a, abs_b, _CMP_LT_OQ);
    __mmask16 blend_mask = (mask & gt_mask) | (~mask & lt_mask);
    
    __m512 result = _mm512_mask_blend_ps(blend_mask, a, b);
    v16sf_result = result;
    
    // Horizontal reduction
    __m256 hi = _mm512_extractf32x8_ps(result, 1);
    __m256 lo = _mm512_extractf32x8_ps(result, 0);
    __m256 sum = _mm256_add_ps(hi, lo);
    return _mm512_insertf32x8(result, sum, 0);
}

// Multi-stage pipeline that uses outputs from one blend as inputs to another
__attribute__((target("avx512f,avx512bw")))
double pipeline_test(unsigned seed) {
    // Initialize with pseudo-random but deterministic values
    char char_data[64];
    short short_data[32];
    int int_data[16];
    long long long_data[8];
    float float_data[16];
    double double_data[8];
    
    // Fill arrays with seed-dependent values
    for (int i = 0; i < 64; i++) {
        char_data[i] = (char)((seed + i * 13) % 256 - 128);
        if (i < 32) short_data[i] = (short)((seed + i * 17) % 65536 - 32768);
        if (i < 16) int_data[i] = (seed + i * 19) * 1103515245;
        if (i < 8) {
            long_data[i] = (long long)(seed + i * 23) * 6364136223846793005LL;
            double_data[i] = (double)(seed + i * 29) / 1073741824.0;
        }
        if (i < 16) float_data[i] = (float)(seed + i * 31) / 65536.0f;
    }
    
    // Load into vectors
    __m512i v64qi_a = _mm512_loadu_si512(char_data);
    __m512i v64qi_b = _mm512_loadu_si512(char_data + 32);
    
    __m512i v32hi_a = _mm512_loadu_si512(short_data);
    __m512i v32hi_b = _mm512_loadu_si512(short_data + 16);
    
    __m512i v16si_a = _mm512_loadu_si512(int_data);
    __m512i v16si_b = _mm512_loadu_si512(int_data + 8);
    
    __m512i v8di_a = _mm512_loadu_si512(long_data);
    __m512i v8di_b = _mm512_loadu_si512(long_data + 4);
    
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_loadu_ps(float_data + 8);
    
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_loadu_pd(double_data + 4);
    
    // Generate runtime masks (prevents constant folding)
    __mmask64 mask64 = 0;
    __mmask32 mask32 = 0;
    __mmask16 mask16 = 0;
    __mmask8 mask8 = 0;
    
    for (int i = 0; i < 64; i++) {
        if ((seed + i) % 3 == 0) mask64 |= (1ULL << i);
        if (i < 32 && (seed + i) % 5 == 0) mask32 |= (1U << i);
        if (i < 16 && (seed + i) % 7 == 0) mask16 |= (1U << i);
        if (i < 8 && (seed + i) % 11 == 0) mask8 |= (1U << i);
    }
    
    // Execute all blend operations in sequence
    __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
    __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    // Prepare half-precision data if available
#ifdef __AVX512FP16__
    _Float16 half_data_a[32], half_data_b[32];
    for (int i = 0; i < 32; i++) {
        half_data_a[i] = (_Float16)((seed + i * 37) % 100) / 10.0f;
        half_data_b[i] = (_Float16)((seed + i * 41) % 100) / 10.0f;
    }
    __m512h v32hf_a = _mm512_loadu_ph(half_data_a);
    __m512h v32hf_b = _mm512_loadu_ph(half_data_b);
    __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
#endif
    
#ifdef __AVX512BF16__
    __bfloat16 bf16_data_a[32], bf16_data_b[32];
    for (int i = 0; i < 32; i++) {
        uint16_t val = (seed + i * 43) % 65536;
        bf16_data_a[i] = *(reinterpret_cast<__bfloat16*>(&val));
        val = (seed + i * 47) % 65536;
        bf16_data_b[i] = *(reinterpret_cast<__bfloat16*>(&val));
    }
    __m512bh v32bf_a = _mm512_loadu_si512(bf16_data_a);
    __m512bh v32bf_b = _mm512_loadu_si512(bf16_data_b);
    __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
#endif
    
    __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16);
    __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8);
    __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8);
    __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Compute checksum from all results
    double checksum = 0.0;
    
    // Reduce 64qi
    __m256i v64qi_low = _mm512_extracti64x4_epi64(result64qi, 0);
    __m256i v64qi_high = _mm512_extracti64x4_epi64(result64qi, 1);
    __m256i sum64qi = _mm256_add_epi8(v64qi_low, v64qi_high);
    char sum_chars[32];
    _mm256_storeu_si256((__m256i*)sum_chars, sum64qi);
    for (int i = 0; i < 32; i++) checksum += sum_chars[i];
    
    // Reduce 32hi
    __m256i v32hi_low = _mm512_extracti64x4_epi64(result32hi, 0);
    short sum_shorts[16];
    _mm256_storeu_si256((__m256i*)sum_shorts, v32hi_low);
    for (int i = 0; i < 16; i++) checksum += sum_shorts[i];
    
    // Reduce 16si
    int sum_ints[16];
    _mm512_storeu_si512(sum_ints, result16si);
    for (int i = 0; i < 16; i++) checksum += sum_ints[i];
    
    // Reduce 8df
    double sum_doubles[8];
    _mm512_storeu_pd(sum_doubles, result8df);
    for (int i = 0; i < 8; i++) checksum += sum_doubles[i];
    
    return checksum;
}

int main(int argc, char** argv) {
    // Use argc as seed for runtime variability
    unsigned seed = (unsigned)(argc > 1 ? atoi(argv[1]) : 12345);
    
    printf("Testing AVX-512 blend intrinsics with seed: %u\n", seed);
    
    double checksum = pipeline_test(seed);
    
    printf("Final checksum: %f\n", checksum);
    printf("Volatile results (prevent DCE):\n");
    printf("  v64qi: %016llx%016llx\n", 
           ((unsigned long long*)&v64qi_result)[1],
           ((unsigned long long*)&v64qi_result)[0]);
    
    return 0;
}
