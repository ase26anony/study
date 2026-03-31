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
    
    // Use runtime value to modify mask (prevent constant folding)
    volatile int r = rand() & 0xFF;
    mask ^= (__mmask64)(r & 1);
    
    return blend_v64qi(a, b, mask);
}

__attribute__((target("avx512bw")))
__m512i compute_v32hi_mask(__m512i a, __m512i b) {
    // Generate mask from comparison
    __mmask32 mask = _mm512_cmpgt_epi16_mask(a, b);
    
    // Add data-dependent variation
    volatile int r = rand() & 0xFFFF;
    mask ^= (__mmask32)((r >> 8) & 1);
    
    return blend_v32hi(a, b, mask);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h compute_v32hf_mask(__m512h a, __m512h b) {
    // Compare and generate mask
    __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
    
    // Data-dependent mask modification
    volatile int r = rand() & 0xFF;
    mask ^= (__mmask32)((r & 0xF) << 4);
    
    return blend_v32hf(a, b, mask);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh compute_v32bf_mask(__m512bh a, __m512bh b) {
    // Convert to float for comparison
    __m512 a_f = _mm512_cvtpbh_ps(a);
    __m512 b_f = _mm512_cvtpbh_ps(b);
    
    // Generate mask from float comparison
    __mmask16 mask16 = _mm512_cmp_ps_mask(a_f, b_f, _CMP_GT_OQ);
    __mmask32 mask = _mm512_kunpackd(mask16, mask16);
    
    // Add randomness
    volatile int r = rand() & 0x3;
    mask ^= (__mmask32)(r);
    
    return blend_v32bf(a, b, mask);
}

__attribute__((target("avx512f")))
__m512i compute_v16si_mask(__m512i a, __m512i b) {
    // Generate mask from comparison
    __mmask16 mask = _mm512_cmpgt_epi32_mask(a, b);
    
    // Data-dependent variation
    volatile int r = rand() & 0xF;
    mask ^= (__mmask16)(r);
    
    return blend_v16si(a, b, mask);
}

__attribute__((target("avx512f")))
__m512i compute_v8di_mask(__m512i a, __m512i b) {
    // Generate mask from comparison
    __mmask8 mask = _mm512_cmpgt_epi64_mask(a, b);
    
    // Add randomness
    volatile int r = rand() & 0x7;
    mask ^= (__mmask8)(r);
    
    return blend_v8di(a, b, mask);
}

__attribute__((target("avx512f")))
__m512d compute_v8df_mask(__m512d a, __m512d b) {
    // Generate mask from comparison
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
    
    // Data-dependent variation
    volatile double r = (double)(rand() & 0x7);
    mask ^= (__mmask8)((int)r & 1);
    
    return blend_v8df(a, b, mask);
}

__attribute__((target("avx512f")))
__m512 compute_v16sf_mask(__m512 a, __m512 b) {
    // Generate mask from comparison
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    
    // Add randomness
    volatile float r = (float)(rand() & 0xF);
    mask ^= (__mmask16)((int)r & 1);
    
    return blend_v16sf(a, b, mask);
}

// Blend function implementations
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion for V64QImode
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Store to volatile to prevent elimination
    v64qi_result = result;
    
    // Multi-stage pipeline: convert to wider type for next operation
    __m512i extended = _mm512_cvtepi8_epi16(_mm512_castsi512_si256(result));
    return _mm512_add_epi16(result, extended);
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Force RTL expansion for V32HImode
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Store to volatile
    v32hi_result = result;
    
    // Prepare for integer blending
    __m512i shifted = _mm512_slli_epi16(result, 1);
    return _mm512_add_epi16(result, shifted);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Force RTL expansion for V32HFmode
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Store to volatile
    v32hf_result = result;
    
    // Mixed precision: convert to float for next operation
    __m512 float_result = _mm512_cvtph_ps(result);
    __m512h converted_back = _mm512_cvtps_ph(float_result, _MM_FROUND_CUR_DIRECTION);
    
    return _mm512_add_ph(result, converted_back);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Force RTL expansion for V32BFmode
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Store to volatile
    v32bf_result = result;
    
    // Convert to float and back for data flow
    __m512 float_a = _mm512_cvtpbh_ps(a);
    __m512 float_b = _mm512_cvtpbh_ps(b);
    __m512 float_blend = _mm512_add_ps(float_a, float_b);
    
    return _mm512_cvtne2ps_pbh(float_blend, float_blend);
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Force RTL expansion for V16SImode
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    // Store to volatile
    v16si_result = result;
    
    // Prepare for double blending
    __m512d double_vec = _mm512_cvtepi32_pd(_mm512_castsi512_si256(result));
    __m512i int_result = _mm512_cvtpd_epi32(double_vec);
    
    return _mm512_add_epi32(result, int_result);
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Force RTL expansion for V8DImode
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    // Store to volatile
    v8di_result = result;
    
    // Mixed type operation
    __m512d double_vec = _mm512_cvtepi64_pd(result);
    __m512i int_result = _mm512_cvtpd_epi64(double_vec);
    
    return _mm512_add_epi64(result, int_result);
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Force RTL expansion for V8DFmode
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    // Store to volatile
    v8df_result = result;
    
    // Convert to integer and back
    __m512i int_vec = _mm512_cvtpd_epi64(result);
    __m512d double_again = _mm512_cvtepi64_pd(int_vec);
    
    return _mm512_add_pd(result, double_again);
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Force RTL expansion for V16SFmode
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Store to volatile
    v16sf_result = result;
    
    // Convert to integer and back
    __m512i int_vec = _mm512_cvtps_epi32(result);
    __m512 float_again = _mm512_cvtepi32_ps(int_vec);
    
    return _mm512_add_ps(result, float_again);
}

// Multi-stage pipeline that chains different blend operations
__attribute__((target("avx512f,avx512bw,avx512fp16,avx512bf16")))
double pipeline_blend_operations(int seed) {
    srand(seed);
    
    // Initialize test data with seed-dependent values
    char char_data[64];
    short short_data[32];
    int int_data[16];
    long long long_data[8];
    float float_data[16];
    double double_data[8];
    uint16_t fp16_data[32];
    uint16_t bf16_data[32];
    
    for (int i = 0; i < 64; i++) char_data[i] = (char)(rand() % 256 - 128);
    for (int i = 0; i < 32; i++) short_data[i] = (short)(rand() % 65536 - 32768);
    for (int i = 0; i < 16; i++) int_data[i] = rand();
    for (int i = 0; i < 8; i++) long_data[i] = ((long long)rand() << 32) | rand();
    for (int i = 0; i < 16; i++) float_data[i] = (float)rand() / RAND_MAX;
    for (int i = 0; i < 8; i++) double_data[i] = (double)rand() / RAND_MAX;
    for (int i = 0; i < 32; i++) fp16_data[i] = rand() & 0xFFFF;
    for (int i = 0; i < 32; i++) bf16_data[i] = rand() & 0xFFFF;
    
    // Load into vectors
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)char_data);
    __m512i v64qi_b = _mm512_loadu_si512((__m512i*)(char_data + 32));
    
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)short_data);
    __m512i v32hi_b = _mm512_loadu_si512((__m512i*)(short_data + 16));
    
    __m512h v32hf_a = _mm512_loadu_ph(fp16_data);
    __m512h v32hf_b = _mm512_loadu_ph(fp16_data + 16);
    
    __m512bh v32bf_a = _mm512_loadu_bh(bf16_data);
    __m512bh v32bf_b = _mm512_loadu_bh(bf16_data + 16);
    
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)int_data);
    __m512i v16si_b = _mm512_loadu_si512((__m512i*)(int_data + 8));
    
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)long_data);
    __m512i v8di_b = _mm512_loadu_si512((__m512i*)(long_data + 4));
    
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_loadu_pd(double_data + 4);
    
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_loadu_ps(float_data + 8);
    
    // Execute all blend operations in sequence
    __m512i r1 = compute_v64qi_mask(v64qi_a, v64qi_b);
    __m512i r2 = compute_v32hi_mask(v32hi_a, v32hi_b);
    __m512h r3 = compute_v32hf_mask(v32hf_a, v32hf_b);
    __m512bh r4 = compute_v32bf_mask(v32bf_a, v32bf_b);
    __m512i r5 = compute_v16si_mask(v16si_a, v16si_b);
    __m512i r6 = compute_v8di_mask(v8di_a, v8di_b);
    __m512d r7 = compute_v8df_mask(v8df_a, v8df_b);
    __m512 r8 = compute_v16sf_mask(v16sf_a, v16sf_b);
    
    // Compute checksum from all results
    double checksum = 0.0;
    
    // Horizontal adds for checksum
    checksum += (double)_mm512_reduce_add_epi64(r1);
    checksum += (double)_mm512_reduce_add_epi64(r2);
    
    // Convert half-precision to double for checksum
    __m512 r3_f = _mm512_cvtph_ps(r3);
    checksum += (double)_mm512_reduce_add_ps(r3_f);
    
    // Convert bfloat16 to double
    __m512 r4_f = _mm512_cvtpbh_ps(r4);
    checksum += (double)_mm512_reduce_add_ps(r4_f);
    
    checksum += (double)_mm512_reduce_add_epi32(r5);
    checksum += (double)_mm512_reduce_add_epi64(r6);
    checksum += _mm512_reduce_add_pd(r7);
    checksum += (double)_mm512_reduce_add_ps(r8);
    
    return checksum;
}

int main(int argc, char** argv) {
    // Use argc as seed for runtime variation
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    printf("Testing AVX-512 blend operations with seed: %d\n", seed);
    
    // Run pipeline multiple times with different seeds
    double total_checksum = 0.0;
    for (int i = 0; i < 3; i++) {
        total_checksum += pipeline_blend_operations(seed + i);
    }
    
    printf("Final checksum: %f\n", total_checksum);
    
    // Force use of volatile results
    printf("Volatile results exist (preventing DCE)\n");
    
    return 0;
}
