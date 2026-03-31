#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Volatile globals to prevent optimization
volatile __m512i v64qi_result;
volatile __m512i v32hi_result;
volatile __m512h v32hf_result;
volatile __m512bh v32bf_result;
volatile __m512i v16si_result;
volatile __m512i v8di_result;
volatile __m512d v8df_result;
volatile __m512 v16sf_result;

// Force RTL expansion by preventing inlining
__attribute__((noinline, target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Use runtime-derived mask to prevent constant folding
    return _mm512_mask_blend_epi8(mask, a, b);
}

__attribute__((noinline, target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Data-dependent computation
    __m512i cmp = _mm512_cmpgt_epi16_mask(a, b);
    __mmask32 combined_mask = mask & cmp;
    return _mm512_mask_blend_epi16(combined_mask, a, b);
}

__attribute__((noinline, target("avx512fp16,avx512bw")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Mixed precision: convert from integer, blend, then operate
    __m512h threshold = _mm512_set1_ph(0.5f);
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, threshold, _CMP_GT_OQ);
    __mmask32 final_mask = mask ^ cmp_mask; // XOR to ensure non-constant
    return _mm512_mask_blend_ph(final_mask, a, b);
}

__attribute__((noinline, target("avx512bf16,avx512bw")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Use bfloat16 specific operations
    __m512bh ones = _mm512_set1_epi16(0x3F80); // 1.0 in bfloat16
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(
        _mm512_castbh_ph(a), 
        _mm512_castbh_ph(ones), 
        _CMP_LT_OQ
    );
    return _mm512_mask_blend_ph(mask | cmp_mask, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Multi-stage: use result from previous operations
    __m512i shifted = _mm512_slli_epi32(a, 1);
    __m512i blended = _mm512_mask_blend_epi32(mask, shifted, b);
    
    // Additional operation to create data dependency
    return _mm512_add_epi32(blended, _mm512_set1_epi32(1));
}

__attribute__((noinline, target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Complex mask computation
    __m512i diff = _mm512_sub_epi64(a, b);
    __mmask8 sign_mask = _mm512_cmpgt_epi64_mask(diff, _mm512_setzero_si512());
    __mmask8 final_mask = mask & sign_mask;
    
    return _mm512_mask_blend_epi64(final_mask, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Type conversion before blending
    __m512d abs_a = _mm512_abs_pd(a);
    __m512d abs_b = _mm512_abs_pd(b);
    
    // Blend based on magnitude comparison
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(abs_a, abs_b, _CMP_GT_OQ);
    return _mm512_mask_blend_pd(mask | cmp_mask, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Multi-operation pipeline
    __m512 sqrt_a = _mm512_sqrt_ps(a);
    __m512 sqrt_b = _mm512_sqrt_ps(b);
    
    // Blend with computed mask
    __m512 blended = _mm512_mask_blend_ps(mask, sqrt_a, sqrt_b);
    
    // Final operation to prevent elimination
    return _mm512_mul_ps(blended, _mm512_set1_ps(2.0f));
}

// Multi-stage pipeline function
__attribute__((noinline, target("avx512f,avx512bw")))
uint64_t vector_pipeline(uint8_t* char_data, uint16_t* short_data,
                         int32_t* int_data, int64_t* long_data,
                         float* float_data, double* double_data,
                         uint16_t* half_data, uint16_t* bf16_data,
                         unsigned seed) {
    uint64_t checksum = 0;
    
    // Initialize vectors with runtime data
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)char_data);
    __m512i v64qi_b = _mm512_loadu_si512((__m512i*)(char_data + 64));
    
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)short_data);
    __m512i v32hi_b = _mm512_loadu_si512((__m512i*)(short_data + 32));
    
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)int_data);
    __m512i v16si_b = _mm512_loadu_si512((__m512i*)(int_data + 16));
    
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)long_data);
    __m512i v8di_b = _mm512_loadu_si512((__m512i*)(long_data + 8));
    
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_loadu_ps(float_data + 16);
    
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_loadu_pd(double_data + 8);
    
    __m512h v32hf_a = _mm512_loadu_ph(half_data);
    __m512h v32hf_b = _mm512_loadu_ph(half_data + 32);
    
    __m512bh v32bf_a = _mm512_loadu_ph(bf16_data);
    __m512bh v32bf_b = _mm512_loadu_ph(bf16_data + 32);
    
    // Generate runtime masks based on seed
    __mmask64 mask64 = (__mmask64)(seed * 0x5DEECE66DULL + 0xB);
    __mmask32 mask32 = (__mmask32)(seed * 0x5DEECE66DULL + 0xB);
    __mmask16 mask16 = (__mmask16)(seed * 0x5DEECE66DULL + 0xB);
    __mmask8 mask8 = (__mmask8)(seed * 0x5DEECE66DULL + 0xB);
    
    // Execute all blend operations in sequence
    __m512i blended_v64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
    v64qi_result = blended_v64qi;
    
    __m512i blended_v32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
    v32hi_result = blended_v32hi;
    
    __m512h blended_v32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
    v32hf_result = blended_v32hf;
    
    __m512bh blended_v32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
    v32bf_result = blended_v32bf;
    
    __m512i blended_v16si = blend_v16si(v16si_a, v16si_b, mask16);
    v16si_result = blended_v16si;
    
    __m512i blended_v8di = blend_v8di(v8di_a, v8di_b, mask8);
    v8di_result = blended_v8di;
    
    __m512d blended_v8df = blend_v8df(v8df_a, v8df_b, mask8);
    v8df_result = blended_v8df;
    
    __m512 blended_v16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
    v16sf_result = blended_v16sf;
    
    // Compute checksum from all results
    uint8_t* ptr = (uint8_t*)&blended_v64qi;
    for (int i = 0; i < 64; i++) {
        checksum += ptr[i];
    }
    
    ptr = (uint8_t*)&blended_v32hi;
    for (int i = 0; i < 64; i++) {
        checksum += ptr[i];
    }
    
    // Horizontal adds for verification
    checksum += _mm512_reduce_add_epi32(blended_v16si);
    checksum += _mm512_reduce_add_epi64(blended_v8di);
    
    return checksum;
}

int main(int argc, char** argv) {
    // Use argc to seed random generation
    unsigned seed = (unsigned)(argc > 1 ? atoi(argv[1]) : 42);
    srand(seed);
    
    // Allocate and initialize test data
    uint8_t char_data[128];
    uint16_t short_data[64];
    int32_t int_data[32];
    int64_t long_data[16];
    float float_data[32];
    double double_data[16];
    uint16_t half_data[64];
    uint16_t bf16_data[64];
    
    // Fill with pseudo-random values
    for (int i = 0; i < 128; i++) char_data[i] = rand() % 256;
    for (int i = 0; i < 64; i++) short_data[i] = rand() % 65536;
    for (int i = 0; i < 32; i++) int_data[i] = rand();
    for (int i = 0; i < 16; i++) long_data[i] = ((int64_t)rand() << 32) | rand();
    for (int i = 0; i < 32; i++) float_data[i] = (float)rand() / RAND_MAX;
    for (int i = 0; i < 16; i++) double_data[i] = (double)rand() / RAND_MAX;
    
    // Initialize half-precision data (FP16 and BF16)
    for (int i = 0; i < 64; i++) {
        half_data[i] = 0x3C00; // 1.0 in FP16
        bf16_data[i] = 0x3F80; // 1.0 in BF16
    }
    
    // Run the vector pipeline
    uint64_t checksum = vector_pipeline(char_data, short_data, int_data, 
                                        long_data, float_data, double_data,
                                        half_data, bf16_data, seed);
    
    printf("Final checksum: %lu\n", checksum);
    
    // Force use of volatile results
    printf("Volatile results exist (preventing DCE)\n");
    
    return 0;
}
