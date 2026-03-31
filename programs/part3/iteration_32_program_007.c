#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Volatile globals to prevent optimization
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

// Data-dependent computation functions
__attribute__((target("avx512bw")))
__m512i compute_v64qi_mask(__m512i a, __m512i b) {
    // Compare and create dynamic mask
    __mmask64 mask = _mm512_cmpeq_epi8_mask(a, b);
    // XOR with pattern to ensure non-constant mask
    mask ^= 0xAAAAAAAAAAAAAAAA;
    return _mm512_mask_blend_epi8(mask, a, b);
}

__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion with runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Additional data-dependent operation to prevent folding
    __m512i cmp = _mm512_cmpgt_epi8_mask(a, b);
    result = _mm512_mask_blend_epi8(cmp, result, a);
    
    // Store to volatile to force materialization
    g_v64qi_result = result;
    return result;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Multi-stage blending with type conversion
    __m512i blended = _mm512_mask_blend_epi16(mask, a, b);
    
    // Create another mask based on data
    __mmask32 mask2 = _mm512_cmpeq_epi16_mask(blended, _mm512_setzero_si512());
    mask2 ^= 0x55555555; // Make non-constant
    
    // Second blend operation
    __m512i result = _mm512_mask_blend_epi16(mask2, blended, a);
    
    g_v32hi_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Blend half-precision floats
    __m512h blended = _mm512_mask_blend_ph(mask, a, b);
    
    // Additional operation: compare and blend again
    __mmask32 mask2 = _mm512_cmp_ph_mask(blended, _mm512_setzero_ph(), _CMP_EQ_OQ);
    mask2 ^= 0xAAAAAAAA; // Non-constant
    
    __m512h result = _mm512_mask_blend_ph(mask2, blended, a);
    
    // Convert to float and back to stress mode transitions
    __m512 floats = _mm512_cvtph_ps(result);
    __m512h converted = _mm512_cvtps_ph(floats, _MM_FROUND_CUR_DIRECTION);
    
    g_v32hf_result = converted;
    return converted;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Blend bfloat16 values
    __m512bh blended = _mm512_mask_blend_ph(mask, a, b);
    
    // Convert to float for computation
    __m512 floats_a = _mm512_cvtpbh_ps(a);
    __m512 floats_b = _mm512_cvtpbh_ps(b);
    
    // Create new mask based on float comparison
    __mmask16 float_mask = _mm512_cmp_ps_mask(floats_a, floats_b, _CMP_LT_OQ);
    
    // Expand mask for bf16 blending
    __mmask32 bf_mask = _mm512_kunpackw(float_mask, float_mask);
    bf_mask ^= 0xCCCCCCCC; // Make non-constant
    
    __m512bh result = _mm512_mask_blend_ph(bf_mask, blended, a);
    
    g_v32bf_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Integer 32-bit blending
    __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
    
    // Create mask from data-dependent computation
    __m512i sum = _mm512_add_epi32(a, b);
    __mmask16 mask2 = _mm512_cmplt_epi32_mask(sum, _mm512_set1_epi32(1000));
    mask2 ^= 0xAAAA; // Non-constant
    
    __m512i result = _mm512_mask_blend_epi32(mask2, blended, sum);
    
    g_v16si_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Integer 64-bit blending
    __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
    
    // Multi-stage: blend then shift
    __m512i shifted = _mm512_slli_epi64(blended, 1);
    
    // Create new mask
    __mmask8 mask2 = _mm512_cmpgt_epi64_mask(a, b);
    mask2 ^= 0x55; // Non-constant
    
    __m512i result = _mm512_mask_blend_epi64(mask2, blended, shifted);
    
    g_v8di_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Double-precision blending
    __m512d blended = _mm512_mask_blend_pd(mask, a, b);
    
    // Arithmetic operation then blend again
    __m512d squared = _mm512_mul_pd(blended, blended);
    
    __mmask8 mask2 = _mm512_cmp_pd_mask(blended, squared, _CMP_LT_OQ);
    mask2 ^= 0xAA; // Non-constant
    
    __m512d result = _mm512_mask_blend_pd(mask2, blended, squared);
    
    g_v8df_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Single-precision blending
    __m512 blended = _mm512_mask_blend_ps(mask, a, b);
    
    // Complex data flow: blend -> math -> blend
    __m512 recip = _mm512_rcp14_ps(blended);
    
    __mmask16 mask2 = _mm512_cmp_ps_mask(blended, recip, _CMP_GT_OQ);
    mask2 ^= 0x5555; // Non-constant
    
    __m512 result = _mm512_mask_blend_ps(mask2, blended, recip);
    
    // Additional: convert to int and back
    __m512i as_int = _mm512_cvtps_epi32(result);
    __m512 back_to_float = _mm512_cvtepi32_ps(as_int);
    
    g_v16sf_result = back_to_float;
    return back_to_float;
}

// Multi-stage pipeline: process data through multiple blend types
__attribute__((target("avx512bw,avx512f")))
uint64_t process_pipeline(uint8_t* char_data, uint16_t* short_data,
                         int32_t* int_data, int64_t* long_data,
                         float* float_data, double* double_data,
                         __fp16* half_data, __bf16* bf16_data) {
    uint64_t checksum = 0;
    
    // Stage 1: V64QI from char data
    __m512i v64qi_a = _mm512_loadu_si512(char_data);
    __m512i v64qi_b = _mm512_loadu_si512(char_data + 64);
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        mask64 |= ((uint64_t)(char_data[i] > 128) << i);
    }
    mask64 ^= 0xAAAAAAAAAAAAAAAA; // Ensure non-constant
    __m512i v64qi_result = blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    // Stage 2: V32HI from short data (packed from char result)
    __m512i v32hi_a = _mm512_loadu_si512(short_data);
    __m512i v32hi_b = _mm512_loadu_si512(short_data + 32);
    __mmask32 mask32 = 0;
    for (int i = 0; i < 32; i++) {
        mask32 |= ((short_data[i] % 3 == 0) << i);
    }
    mask32 ^= 0x55555555;
    __m512i v32hi_result = blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    // Stage 3: V32HF from half-precision
    __m512h v32hf_a = _mm512_loadu_ph(half_data);
    __m512h v32hf_b = _mm512_loadu_ph(half_data + 32);
    __mmask32 mask32_hf = _mm512_cmp_ph_mask(v32hf_a, v32hf_b, _CMP_NEQ_UQ);
    mask32_hf ^= 0xCCCCCCCC;
    __m512h v32hf_result = blend_v32hf(v32hf_a, v32hf_b, mask32_hf);
    
    // Stage 4: V32BF from bfloat16
    __m512bh v32bf_a = _mm512_loadu_bf16(bf16_data);
    __m512bh v32bf_b = _mm512_loadu_bf16(bf16_data + 32);
    __mmask32 mask32_bf = 0;
    for (int i = 0; i < 32; i++) {
        mask32_bf |= ((i % 4 == 0) << i);
    }
    mask32_bf ^= 0xF0F0F0F0;
    __m512bh v32bf_result = blend_v32bf(v32bf_a, v32bf_b, mask32_bf);
    
    // Stage 5: V16SI from int data
    __m512i v16si_a = _mm512_loadu_si512(int_data);
    __m512i v16si_b = _mm512_loadu_si512(int_data + 16);
    __mmask16 mask16 = _mm512_cmplt_epi32_mask(
        _mm512_loadu_si512(int_data),
        _mm512_set1_epi32(1000000)
    );
    mask16 ^= 0xAAAA;
    __m512i v16si_result = blend_v16si(v16si_a, v16si_b, mask16);
    
    // Stage 6: V8DI from long data
    __m512i v8di_a = _mm512_loadu_si512(long_data);
    __m512i v8di_b = _mm512_loadu_si512(long_data + 8);
    __mmask8 mask8 = 0;
    for (int i = 0; i < 8; i++) {
        mask8 |= ((long_data[i] & 1) << i);
    }
    mask8 ^= 0x55;
    __m512i v8di_result = blend_v8di(v8di_a, v8di_b, mask8);
    
    // Stage 7: V8DF from double data
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_loadu_pd(double_data + 8);
    __mmask8 mask8_df = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_LT_OQ);
    mask8_df ^= 0xAA;
    __m512d v8df_result = blend_v8df(v8df_a, v8df_b, mask8_df);
    
    // Stage 8: V16SF from float data
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_loadu_ps(float_data + 16);
    __mmask16 mask16_sf = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_GT_OQ);
    mask16_sf ^= 0x5555;
    __m512 v16sf_result = blend_v16sf(v16sf_a, v16sf_b, mask16_sf);
    
    // Compute checksum from all results
    uint8_t* ptr = (uint8_t*)&v64qi_result;
    for (int i = 0; i < 64; i++) checksum += ptr[i];
    
    ptr = (uint8_t*)&v32hi_result;
    for (int i = 0; i < 64; i++) checksum += ptr[i];
    
    // Add results from all blend operations
    checksum += _mm512_reduce_add_epi32(v16si_result);
    checksum += _mm512_reduce_add_epi64(v8di_result);
    
    return checksum;
}

int main(int argc, char** argv) {
    // Use argc as seed for pseudo-random but deterministic data
    unsigned seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    // Allocate and initialize test data
    uint8_t char_data[128];
    uint16_t short_data[64];
    int32_t int_data[32];
    int64_t long_data[16];
    float float_data[32];
    double double_data[16];
    __fp16 half_data[64];
    __bf16 bf16_data[64];
    
    // Fill with pseudo-random data
    for (int i = 0; i < 128; i++) char_data[i] = rand() % 256;
    for (int i = 0; i < 64; i++) short_data[i] = rand() % 65536;
    for (int i = 0; i < 32; i++) int_data[i] = rand() - RAND_MAX/2;
    for (int i = 0; i < 16; i++) long_data[i] = (int64_t)rand() << 32 | rand();
    for (int i = 0; i < 32; i++) float_data[i] = (float)rand() / RAND_MAX;
    for (int i = 0; i < 16; i++) double_data[i] = (double)rand() / RAND_MAX;
    for (int i = 0; i < 64; i++) half_data[i] = (__fp16)((float)rand() / RAND_MAX);
    for (int i = 0; i < 64; i++) bf16_data[i] = (__bf16)((float)rand() / RAND_MAX);
    
    // Run the pipeline
    uint64_t checksum = process_pipeline(char_data, short_data, int_data,
                                        long_data, float_data, double_data,
                                        half_data, bf16_data);
    
    printf("Checksum: %lu\n", checksum);
    
    // Force use of volatile results
    printf("Volatile results exist\n");
    
    return 0;
}
