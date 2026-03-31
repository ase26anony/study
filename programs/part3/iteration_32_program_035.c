#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile globals to prevent optimization */
volatile __m512i v64qi_result;
volatile __m512i v32hi_result;
volatile __m512h v32hf_result;
volatile __m512bh v32bf_result;
volatile __m512i v16si_result;
volatile __m512i v8di_result;
volatile __m512d v8df_result;
volatile __m512 v16sf_result;

/* Function prototypes with target attributes */
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

/* V64QI: 64x 8-bit integers */
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    /* Force RTL expansion for E_V64QImode */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Data-dependent computation to prevent folding */
    __m512i temp = _mm512_add_epi8(result, _mm512_set1_epi8(1));
    result = _mm512_mask_blend_epi8(mask ^ 0xAAAAAAAAAAAAAAAA, result, temp);
    
    v64qi_result = result; /* Volatile store */
    return result;
}

/* V32HI: 32x 16-bit integers */
__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    /* Force RTL expansion for E_V32HImode */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    /* Multi-stage pipeline: use previous blend result */
    __m512i shifted = _mm512_slli_epi16(result, 1);
    result = _mm512_mask_blend_epi16(mask | 0x55555555, result, shifted);
    
    v32hi_result = result; /* Volatile store */
    return result;
}

/* V32HF: 32x half-precision floats */
__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    /* Force RTL expansion for E_V32HFmode */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Mixed precision: convert to float, operate, convert back */
    __m512 float_vec = _mm512_cvtph_ps(result);
    __m512 float_scaled = _mm512_mul_ps(float_vec, _mm512_set1_ps(2.0f));
    __m512h result2 = _mm512_cvtps_ph(float_scaled, _MM_FROUND_CUR_DIRECTION);
    
    /* Second blend with modified mask */
    result = _mm512_mask_blend_ph(mask ^ 0x33333333, result, result2);
    
    v32hf_result = result; /* Volatile store */
    return result;
}

/* V32BF: 32x bfloat16 */
__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    /* Force RTL expansion for E_V32BFmode */
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Convert to float and back to create data dependency */
    __m512 float_vec = _mm512_cvtpbh_ps(result);
    __m512 float_add = _mm512_add_ps(float_vec, _mm512_set1_ps(1.0f));
    __m512bh result2 = _mm512_cvtne2ps_pbh(float_add, float_add);
    
    /* Blend with converted result */
    result = _mm512_mask_blend_ph(mask | 0x0F0F0F0F, result, result2);
    
    v32bf_result = result; /* Volatile store */
    return result;
}

/* V16SI: 16x 32-bit integers */
__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    /* Force RTL expansion for E_V16SImode */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    /* Create predicate from comparison */
    __mmask16 cmp_mask = _mm512_cmpgt_epi32_mask(result, _mm512_set1_epi32(0));
    result = _mm512_mask_blend_epi32(cmp_mask & mask, result, 
                                     _mm512_abs_epi32(result));
    
    v16si_result = result; /* Volatile store */
    return result;
}

/* V8DI: 8x 64-bit integers */
__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    /* Force RTL expansion for E_V8DImode */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    /* Chain blends for pipeline */
    __m512i rotated = _mm512_rorv_epi64(result, _mm512_set1_epi64(32));
    result = _mm512_mask_blend_epi64(mask ^ 0xAA, result, rotated);
    
    v8di_result = result; /* Volatile store */
    return result;
}

/* V8DF: 8x double-precision floats */
__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    /* Force RTL expansion for E_V8DFmode */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    /* Data-dependent computation with comparison */
    __mmask8 lt_mask = _mm512_cmp_pd_mask(result, _mm512_set1_pd(0.0), _CMP_LT_OQ);
    __m512d abs_result = _mm512_abs_pd(result);
    result = _mm512_mask_blend_pd(lt_mask | mask, result, abs_result);
    
    v8df_result = result; /* Volatile store */
    return result;
}

/* V16SF: 16x single-precision floats */
__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    /* Force RTL expansion for E_V16SFmode */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    /* Complex dataflow with multiple blends */
    __m512 recip = _mm512_rcp14_ps(result);
    __m512 blended_recip = _mm512_mask_blend_ps(mask ^ 0x5555, result, recip);
    
    /* Final blend mixing original and reciprocal */
    result = _mm512_mask_blend_ps(mask | 0x3333, blended_recip, result);
    
    v16sf_result = result; /* Volatile store */
    return result;
}

/* Helper to generate runtime-dependent masks */
__mmask64 generate_mask64(int seed) {
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (((seed + i) * 1103515245) % 3 == 0) {
            mask |= (1ULL << i);
        }
    }
    return mask;
}

__mmask32 generate_mask32(int seed) {
    return (__mmask32)generate_mask64(seed);
}

__mmask16 generate_mask16(int seed) {
    return (__mmask16)generate_mask64(seed);
}

__mmask8 generate_mask8(int seed) {
    return (__mmask8)generate_mask64(seed);
}

int main(int argc, char **argv) {
    /* Use argc for runtime-dependent behavior */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Initialize test data with runtime values */
    char char_data[64];
    short short_data[32];
    int int_data[16];
    long long long_data[8];
    float float_data[16];
    double double_data[8];
    uint16_t half_data[32];
    uint16_t bfloat_data[32];
    
    for (int i = 0; i < 64; i++) {
        char_data[i] = (char)((seed + i * 13) % 256 - 128);
        if (i < 32) {
            short_data[i] = (short)((seed + i * 17) % 65536 - 32768);
            half_data[i] = (uint16_t)((seed + i * 19) % 65536);
            bfloat_data[i] = (uint16_t)((seed + i * 23) % 65536);
        }
        if (i < 16) {
            int_data[i] = (seed + i * 29) * 1103515245;
            float_data[i] = (float)((seed + i * 31) % 100) / 10.0f;
        }
        if (i < 8) {
            long_data[i] = (long long)(seed + i * 37) * 1103515245;
            double_data[i] = (double)((seed + i * 41) % 100) / 10.0;
        }
    }
    
    /* Load vectors */
    __m512i v64qi_a = _mm512_loadu_si512((void*)char_data);
    __m512i v64qi_b = _mm512_loadu_si512((void*)(char_data + 32));
    
    __m512i v32hi_a = _mm512_loadu_si512((void*)short_data);
    __m512i v32hi_b = _mm512_loadu_si512((void*)(short_data + 16));
    
    __m512h v32hf_a = _mm512_loadu_ph((void*)half_data);
    __m512h v32hf_b = _mm512_loadu_ph((void*)(half_data + 16));
    
    __m512bh v32bf_a = _mm512_loadu_ph((void*)bfloat_data);
    __m512bh v32bf_b = _mm512_loadu_ph((void*)(bfloat_data + 16));
    
    __m512i v16si_a = _mm512_loadu_si512((void*)int_data);
    __m512i v16si_b = _mm512_loadu_si512((void*)(int_data + 8));
    
    __m512i v8di_a = _mm512_loadu_si512((void*)long_data);
    __m512i v8di_b = _mm512_loadu_si512((void*)(long_data + 4));
    
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_loadu_pd(double_data + 4);
    
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_loadu_ps(float_data + 8);
    
    /* Generate runtime-dependent masks */
    __mmask64 mask64 = generate_mask64(seed);
    __mmask32 mask32 = generate_mask32(seed);
    __mmask16 mask16 = generate_mask16(seed);
    __mmask8 mask8 = generate_mask8(seed);
    
    /* Execute all blend functions */
    __m512i r64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
    __m512i r32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
    __m512h r32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
    __m512bh r32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
    __m512i r16si = blend_v16si(v16si_a, v16si_b, mask16);
    __m512i r8di = blend_v8di(v8di_a, v8di_b, mask8);
    __m512d r8df = blend_v8df(v8df_a, v8df_b, mask8);
    __m512 r16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    /* Compute checksum to verify execution */
    uint64_t checksum = 0;
    
    /* Horizontal reductions */
    checksum += _mm512_reduce_add_epi64(r64qi);
    checksum += _mm512_reduce_add_epi64(r32hi);
    
    /* For half-precision, convert to float first */
    __m512 r32hf_float = _mm512_cvtph_ps(r32hf);
    checksum += (uint64_t)_mm512_reduce_add_ps(r32hf_float);
    
    __m512 r32bf_float = _mm512_cvtpbh_ps(r32bf);
    checksum += (uint64_t)_mm512_reduce_add_ps(r32bf_float);
    
    checksum += _mm512_reduce_add_epi64(r16si);
    checksum += _mm512_reduce_add_epi64(r8di);
    checksum += (uint64_t)_mm512_reduce_add_pd(r8df);
    checksum += (uint64_t)_mm512_reduce_add_ps(r16sf);
    
    printf("Checksum: %lu\n", checksum);
    return 0;
}
