#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
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

// V64QI: 64x 8-bit integers
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Data-dependent computation to prevent folding
    __m512i check = _mm512_xor_si512(a, b);
    result = _mm512_add_epi8(result, check);
    
    // Store to volatile to force materialization
    v64qi_result = result;
    return result;
}

// V32HI: 32x 16-bit integers
__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Create complex mask from data to prevent constant folding
    __m512i cmp = _mm512_cmpgt_epi16_mask(a, b);
    __mmask32 dynamic_mask = mask & cmp;
    
    __m512i result = _mm512_mask_blend_epi16(dynamic_mask, a, b);
    
    // Multi-stage: convert to 32-bit for next operation
    __m512i extended = _mm512_madd_epi16(result, _mm512_set1_epi16(1));
    
    v32hi_result = result;
    return extended;
}

// V32HF: 32x half-precision floats (requires -mavx512fp16)
#ifdef __AVX512FP16__
__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Generate mask from comparison to prevent optimization
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, b, _CMP_LT_OQ);
    __mmask32 blend_mask = mask ^ cmp_mask;  // XOR for non-trivial mask
    
    __m512h result = _mm512_mask_blend_ph(blend_mask, a, b);
    
    // Mixed precision: convert to float for further operations
    __m512 float_result = _mm512_cvtph_ps(result);
    
    v32hf_result = result;
    return result;
}
#endif

// V32BF: 32x bfloat16 (requires -mavx512bf16)
#ifdef __AVX512BF16__
__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Use same intrinsic as V32HF but with bfloat16 type
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Convert to float32 for computation
    __m512 float_vals = _mm512_cvtne2ps_pbh(a, b);
    result = _mm512_cvtneeph_ps(float_vals);
    
    v32bf_result = result;
    return result;
}
#endif

// V16SI: 16x 32-bit integers
__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Create data-dependent mask
    __m512i diff = _mm512_sub_epi32(a, b);
    __mmask16 sign_mask = _mm512_cmplt_epi32_mask(diff, _mm512_setzero_si512());
    __mmask16 blend_mask = mask | sign_mask;
    
    __m512i result = _mm512_mask_blend_epi32(blend_mask, a, b);
    
    // Feed into next stage: convert to float
    __m512 float_vec = _mm512_cvtepi32_ps(result);
    
    v16si_result = result;
    return result;
}

// V8DI: 8x 64-bit integers
__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Complex mask generation
    __mmask8 eq_mask = _mm512_cmpeq_epi64_mask(a, b);
    __mmask8 neq_mask = ~eq_mask;
    __mmask8 blend_mask = mask & neq_mask;
    
    __m512i result = _mm512_mask_blend_epi64(blend_mask, a, b);
    
    // Convert to double for mixed-type pipeline
    __m512d double_vec = _mm512_cvtepi64_pd(result);
    
    v8di_result = result;
    return result;
}

// V8DF: 8x double-precision floats
__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Generate mask from comparison
    __mmask8 lt_mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    __mmask8 blend_mask = mask ^ lt_mask;
    
    __m512d result = _mm512_mask_blend_pd(blend_mask, a, b);
    
    // Convert to integer for type mixing
    __m512i int_vec = _mm512_cvtpd_epi64(result);
    
    v8df_result = result;
    return result;
}

// V16SF: 16x single-precision floats
__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Create complex, data-dependent mask
    __m512 abs_a = _mm512_abs_ps(a);
    __m512 abs_b = _mm512_abs_ps(b);
    __mmask16 abs_mask = _mm512_cmp_ps_mask(abs_a, abs_b, _CMP_GT_OQ);
    __mmask16 blend_mask = mask | abs_mask;
    
    __m512 result = _mm512_mask_blend_ps(blend_mask, a, b);
    
    // Convert to integer for pipeline continuation
    __m512i int_vec = _mm512_cvtps_epi32(result);
    
    v16sf_result = result;
    return result;
}

// Initialize data with pseudo-random values based on argc
void init_data(int argc, 
               __m512i* v64qi_a, __m512i* v64qi_b,
               __m512i* v32hi_a, __m512i* v32hi_b,
               __m512h* v32hf_a, __m512h* v32hf_b,
               __m512bh* v32bf_a, __m512bh* v32bf_b,
               __m512i* v16si_a, __m512i* v16si_b,
               __m512i* v8di_a, __m512i* v8di_b,
               __m512d* v8df_a, __m512d* v8df_b,
               __m512* v16sf_a, __m512* v16sf_b) {
    
    uint64_t seed = argc * 1103515245ULL + 12345;
    
    // Initialize 64x 8-bit integers
    char v64qi_data_a[64], v64qi_data_b[64];
    for (int i = 0; i < 64; i++) {
        seed = seed * 1103515245ULL + 12345;
        v64qi_data_a[i] = (char)(seed >> 56);
        v64qi_data_b[i] = (char)(seed >> 48);
    }
    *v64qi_a = _mm512_loadu_si512(v64qi_data_a);
    *v64qi_b = _mm512_loadu_si512(v64qi_data_b);
    
    // Initialize 32x 16-bit integers
    short v32hi_data_a[32], v32hi_data_b[32];
    for (int i = 0; i < 32; i++) {
        seed = seed * 1103515245ULL + 12345;
        v32hi_data_a[i] = (short)(seed >> 48);
        v32hi_data_b[i] = (short)(seed >> 32);
    }
    *v32hi_a = _mm512_loadu_si512(v32hi_data_a);
    *v32hi_b = _mm512_loadu_si512(v32hi_data_b);
    
    // Initialize 32x half-precision floats
    #ifdef __AVX512FP16__
    _Float16 v32hf_data_a[32], v32hf_data_b[32];
    for (int i = 0; i < 32; i++) {
        seed = seed * 1103515245ULL + 12345;
        v32hf_data_a[i] = (_Float16)((seed & 0xFFFF) / 65536.0f);
        v32hf_data_b[i] = (_Float16)(((seed >> 16) & 0xFFFF) / 65536.0f);
    }
    *v32hf_a = _mm512_loadu_ph(v32hf_data_a);
    *v32hf_b = _mm512_loadu_ph(v32hf_data_b);
    #endif
    
    // Initialize 32x bfloat16
    #ifdef __AVX512BF16__
    __bf16 v32bf_data_a[32], v32bf_data_b[32];
    for (int i = 0; i < 32; i++) {
        seed = seed * 1103515245ULL + 12345;
        uint16_t bf_val = (uint16_t)(seed & 0xFFFF);
        memcpy(&v32bf_data_a[i], &bf_val, sizeof(__bf16));
        bf_val = (uint16_t)((seed >> 16) & 0xFFFF);
        memcpy(&v32bf_data_b[i], &bf_val, sizeof(__bf16));
    }
    *v32bf_a = _mm512_loadu_si512(v32bf_data_a);
    *v32bf_b = _mm512_loadu_si512(v32bf_data_b);
    #endif
    
    // Initialize 16x 32-bit integers
    int v16si_data_a[16], v16si_data_b[16];
    for (int i = 0; i < 16; i++) {
        seed = seed * 1103515245ULL + 12345;
        v16si_data_a[i] = (int)seed;
        v16si_data_b[i] = (int)(seed >> 32);
    }
    *v16si_a = _mm512_loadu_si512(v16si_data_a);
    *v16si_b = _mm512_loadu_si512(v16si_data_b);
    
    // Initialize 8x 64-bit integers
    long long v8di_data_a[8], v8di_data_b[8];
    for (int i = 0; i < 8; i++) {
        seed = seed * 1103515245ULL + 12345;
        v8di_data_a[i] = (long long)seed;
        v8di_data_b[i] = (long long)(seed ^ 0xAAAAAAAAAAAAAAAAULL);
    }
    *v8di_a = _mm512_loadu_si512(v8di_data_a);
    *v8di_b = _mm512_loadu_si512(v8di_data_b);
    
    // Initialize 8x doubles
    double v8df_data_a[8], v8df_data_b[8];
    for (int i = 0; i < 8; i++) {
        seed = seed * 1103515245ULL + 12345;
        v8df_data_a[i] = (double)seed / 1e9;
        v8df_data_b[i] = (double)(seed >> 32) / 1e9;
    }
    *v8df_a = _mm512_loadu_pd(v8df_data_a);
    *v8df_b = _mm512_loadu_pd(v8df_data_b);
    
    // Initialize 16x floats
    float v16sf_data_a[16], v16sf_data_b[16];
    for (int i = 0; i < 16; i++) {
        seed = seed * 1103515245ULL + 12345;
        v16sf_data_a[i] = (float)seed / 1e6f;
        v16sf_data_b[i] = (float)(seed >> 32) / 1e6f;
    }
    *v16sf_a = _mm512_loadu_ps(v16sf_data_a);
    *v16sf_b = _mm512_loadu_ps(v16sf_data_b);
}

int main(int argc, char** argv) {
    // Declare all vectors
    __m512i v64qi_a, v64qi_b;
    __m512i v32hi_a, v32hi_b;
    __m512h v32hf_a, v32hf_b;
    __m512bh v32bf_a, v32bf_b;
    __m512i v16si_a, v16si_b;
    __m512i v8di_a, v8di_b;
    __m512d v8df_a, v8df_b;
    __m512 v16sf_a, v16sf_b;
    
    // Initialize with runtime-dependent values
    init_data(argc, 
              &v64qi_a, &v64qi_b,
              &v32hi_a, &v32hi_b,
              &v32hf_a, &v32hf_b,
              &v32bf_a, &v32bf_b,
              &v16si_a, &v16si_b,
              &v8di_a, &v8di_b,
              &v8df_a, &v8df_b,
              &v16sf_a, &v16sf_b);
    
    // Generate masks from runtime data to prevent constant folding
    __mmask64 mask64 = 0;
    __mmask32 mask32 = 0;
    __mmask16 mask16 = 0;
    __mmask8 mask8 = 0;
    
    // Create non-trivial masks based on argc
    for (int i = 0; i < 64; i++) {
        if ((argc + i) % 3 == 0) mask64 |= (1ULL << i);
        if (i < 32 && (argc + i) % 2 == 0) mask32 |= (1U << i);
        if (i < 16 && (argc + i) % 4 == 0) mask16 |= (1U << i);
        if (i < 8 && (argc + i) % 5 == 0) mask8 |= (1U << i);
    }
    
    // Execute all blend operations in a pipeline
    __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    __m512i extended32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    #ifdef __AVX512FP16__
    __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
    #endif
    
    #ifdef __AVX512BF16__
    __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
    #endif
    
    __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16);
    
    __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8);
    
    __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8);
    
    __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Compute checksum from all results to prevent dead code elimination
    uint64_t checksum = 0;
    
    // Extract values from results (forces materialization)
    char* r64qi = (char*)&result64qi;
    for (int i = 0; i < 64; i++) checksum += r64qi[i];
    
    short* r32hi = (short*)&extended32hi;
    for (int i = 0; i < 32; i++) checksum += r32hi[i];
    
    #ifdef __AVX512FP16__
    _Float16* r32hf = (_Float16*)&result32hf;
    for (int i = 0; i < 32; i++) checksum += *(uint16_t*)&r32hf[i];
    #endif
    
    #ifdef __AVX512BF16__
    __bf16* r32bf = (__bf16*)&result32bf;
    for (int i = 0; i < 32; i++) checksum += *(uint16_t*)&r32bf[i];
    #endif
    
    int* r16si = (int*)&result16si;
    for (int i = 0; i < 16; i++) checksum += r16si[i];
    
    long long* r8di = (long long*)&result8di;
    for (int i = 0; i < 8; i++) checksum += r8di[i];
    
    double* r8df = (double*)&result8df;
    for (int i = 0; i < 8; i++) checksum += (uint64_t)r8df[i];
    
    float* r16sf = (float*)&result16sf;
    for (int i = 0; i < 16; i++) checksum += (uint32_t)r16sf[i];
    
    printf("Checksum: %lu\n", checksum);
    
    return 0;
}
