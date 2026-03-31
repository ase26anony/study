#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global volatile variables to prevent optimization
volatile __m512i v64qi_result;
volatile __m512i v32hi_result;
volatile __m512h v32hf_result;
volatile __m512bh v32bf_result;
volatile __m512i v16si_result;
volatile __m512i v8di_result;
volatile __m512d v8df_result;
volatile __m512 v16sf_result;

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
__m512i compute_v64qi_mask(__m512i a, __m512i b) {
    // Generate mask based on comparison
    __mmask64 mask = _mm512_cmpgt_epi8_mask(a, b);
    // Mix with runtime value to prevent constant folding
    volatile int seed = rand();
    mask ^= (__mmask64)(seed & 0xFF);
    return mask;
}

__attribute__((target("avx512bw")))
__m512i compute_v32hi_mask(__m512i a, __m512i b) {
    __mmask32 mask = _mm512_cmpgt_epi16_mask(a, b);
    volatile int seed = rand();
    mask ^= (__mmask32)(seed & 0xFFFF);
    return mask;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h compute_v32hf_mask(__m512h a, __m512h b) {
    __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
    volatile int seed = rand();
    mask ^= (__mmask32)(seed & 0xFFFF);
    return mask;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh compute_v32bf_mask(__m512bh a, __m512bh b) {
    // Convert to float for comparison
    __m512 a_f = _mm512_cvtpbh_ps(a);
    __m512 b_f = _mm512_cvtpbh_ps(b);
    __mmask16 mask16 = _mm512_cmp_ps_mask(a_f, b_f, _CMP_GT_OQ);
    __mmask32 mask = (__mmask32)mask16;
    volatile int seed = rand();
    mask ^= (__mmask32)(seed & 0xFFFF);
    return mask;
}

__attribute__((target("avx512f")))
__m512i compute_v16si_mask(__m512i a, __m512i b) {
    __mmask16 mask = _mm512_cmpgt_epi32_mask(a, b);
    volatile int seed = rand();
    mask ^= (__mmask16)(seed & 0xFF);
    return mask;
}

__attribute__((target("avx512f")))
__m512i compute_v8di_mask(__m512i a, __m512i b) {
    __mmask8 mask = _mm512_cmpgt_epi64_mask(a, b);
    volatile int seed = rand();
    mask ^= (__mmask8)(seed & 0xFF);
    return mask;
}

__attribute__((target("avx512f")))
__m512d compute_v8df_mask(__m512d a, __m512d b) {
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
    volatile int seed = rand();
    mask ^= (__mmask8)(seed & 0xFF);
    return mask;
}

__attribute__((target("avx512f")))
__m512 compute_v16sf_mask(__m512 a, __m512 b) {
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    volatile int seed = rand();
    mask ^= (__mmask16)(seed & 0xFF);
    return mask;
}

// Blend function implementations
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using the intrinsic
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Additional data-dependent computation
    result = _mm512_add_epi8(result, _mm512_set1_epi8(1));
    
    // Store to volatile to prevent elimination
    v64qi_result = result;
    
    return result;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Mix with converted data from v64qi
    result = _mm512_add_epi16(result, _mm512_srai_epi16(result, 4));
    
    v32hi_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Additional FP operation
    result = _mm512_add_ph(result, _mm512_set1_ph(1.0f));
    
    v32hf_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Convert to float and back for additional computation
    __m512 temp = _mm512_cvtpbh_ps(result);
    temp = _mm512_add_ps(temp, _mm512_set1_ps(0.5f));
    result = _mm512_cvtne2ps_pbh(temp, temp);
    
    v32bf_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    // Use result from v32hi as input
    result = _mm512_add_epi32(result, 
        _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(v32hi_result, 0)));
    
    v16si_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    // Chain from v16si
    result = _mm512_add_epi64(result,
        _mm512_cvtepi32_epi64(_mm512_extracti32x8_epi32(v16si_result, 0)));
    
    v8di_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    // Convert from integer
    result = _mm512_add_pd(result,
        _mm512_cvtepi64_pd(_mm512_extracti64x4_epi64(v8di_result, 0)));
    
    v8df_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Convert from double
    result = _mm512_add_ps(result,
        _mm512_cvtpd_ps(_mm512_extractf64x4_pd(v8df_result, 0)));
    
    v16sf_result = result;
    return result;
}

// Multi-stage pipeline that chains blends
__attribute__((target("avx512f,avx512bw,avx512fp16,avx512bf16")))
double run_blend_pipeline(int seed) {
    srand(seed);
    
    // Initialize test data with pseudo-random values
    int8_t data8[64];
    int16_t data16[32];
    int32_t data32[16];
    int64_t data64[8];
    float dataf[16];
    double datad[8];
    uint16_t datahf[32];  // FP16 storage
    uint16_t databf[32];  // BF16 storage
    
    for (int i = 0; i < 64; i++) data8[i] = (rand() % 256) - 128;
    for (int i = 0; i < 32; i++) data16[i] = (rand() % 65536) - 32768;
    for (int i = 0; i < 16; i++) data32[i] = rand();
    for (int i = 0; i < 8; i++) data64[i] = ((int64_t)rand() << 32) | rand();
    for (int i = 0; i < 16; i++) dataf[i] = (float)rand() / RAND_MAX;
    for (int i = 0; i < 8; i++) datad[i] = (double)rand() / RAND_MAX;
    for (int i = 0; i < 32; i++) datahf[i] = rand() & 0xFFFF;
    for (int i = 0; i < 32; i++) databf[i] = rand() & 0xFFFF;
    
    // Load into vectors
    __m512i v64qi_a = _mm512_loadu_si512(data8);
    __m512i v64qi_b = _mm512_loadu_si512(data8 + 32);
    
    __m512i v32hi_a = _mm512_loadu_si512(data16);
    __m512i v32hi_b = _mm512_loadu_si512(data16 + 16);
    
    __m512h v32hf_a = _mm512_loadu_ph(datahf);
    __m512h v32hf_b = _mm512_loadu_ph(datahf + 16);
    
    __m512bh v32bf_a = _mm512_loadu_ph(databf);
    __m512bh v32bf_b = _mm512_loadu_ph(databf + 16);
    
    __m512i v16si_a = _mm512_loadu_si512(data32);
    __m512i v16si_b = _mm512_loadu_si512(data32 + 8);
    
    __m512i v8di_a = _mm512_loadu_si512(data64);
    __m512i v8di_b = _mm512_loadu_si512(data64 + 4);
    
    __m512d v8df_a = _mm512_loadu_pd(datad);
    __m512d v8df_b = _mm512_loadu_pd(datad + 4);
    
    __m512 v16sf_a = _mm512_loadu_ps(dataf);
    __m512 v16sf_b = _mm512_loadu_ps(dataf + 8);
    
    // Compute masks using runtime data
    __mmask64 mask64 = compute_v64qi_mask(v64qi_a, v64qi_b);
    __mmask32 mask32 = compute_v32hi_mask(v32hi_a, v32hi_b);
    __mmask32 mask32hf = compute_v32hf_mask(v32hf_a, v32hf_b);
    __mmask32 mask32bf = compute_v32bf_mask(v32bf_a, v32bf_b);
    __mmask16 mask16 = compute_v16si_mask(v16si_a, v16si_b);
    __mmask8 mask8 = compute_v8di_mask(v8di_a, v8di_b);
    __mmask8 mask8df = compute_v8df_mask(v8df_a, v8df_b);
    __mmask16 mask16sf = compute_v16sf_mask(v16sf_a, v16sf_b);
    
    // Execute all blend operations in sequence
    __m512i r64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
    __m512i r32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
    __m512h r32hf = blend_v32hf(v32hf_a, v32hf_b, mask32hf);
    __m512bh r32bf = blend_v32bf(v32bf_a, v32bf_b, mask32bf);
    __m512i r16si = blend_v16si(v16si_a, v16si_b, mask16);
    __m512i r8di = blend_v8di(v8di_a, v8di_b, mask8);
    __m512d r8df = blend_v8df(v8df_a, v8df_b, mask8df);
    __m512 r16sf = blend_v16sf(v16sf_a, v16sf_b, mask16sf);
    
    // Compute checksum from all results
    double checksum = 0.0;
    
    // Horizontal sums
    checksum += _mm512_reduce_add_epi64(r64qi);
    checksum += _mm512_reduce_add_epi64(r32hi);
    
    // FP16 horizontal sum (convert to float first)
    __m512 r32hf_f = _mm512_cvtph_ps(r32hf);
    checksum += _mm512_reduce_add_ps(r32hf_f);
    
    // BF16 horizontal sum
    __m512 r32bf_f = _mm512_cvtpbh_ps(r32bf);
    checksum += _mm512_reduce_add_ps(r32bf_f);
    
    checksum += _mm512_reduce_add_epi32(r16si);
    checksum += _mm512_reduce_add_epi64(r8di);
    checksum += _mm512_reduce_add_pd(r8df);
    checksum += _mm512_reduce_add_ps(r16sf);
    
    return checksum;
}

int main(int argc, char** argv) {
    // Use argc as seed for runtime variability
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Running AVX-512 blend coverage test with seed: %d\n", seed);
    
    double checksum = run_blend_pipeline(seed);
    
    printf("Final checksum: %f\n", checksum);
    printf("All blend operations executed.\n");
    
    // Verify all volatile results were written
    printf("Volatile results written: %d\n", 
        (v64qi_result[0] != 0) + (v32hi_result[0] != 0) +
        (v16si_result[0] != 0) + (v8di_result[0] != 0));
    
    return 0;
}
