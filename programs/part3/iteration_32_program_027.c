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

/* Function prototypes with explicit target attributes */
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

/* Data-dependent computation to generate runtime masks */
__attribute__((noinline))
__mmask64 compute_mask64(int seed) {
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        mask |= ((seed + i) % 3 == 0) ? (1ULL << i) : 0;
    }
    return mask;
}

__attribute__((noinline))
__mmask32 compute_mask32(int seed) {
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        mask |= ((seed + i * 2) % 5 == 0) ? (1U << i) : 0;
    }
    return mask;
}

__attribute__((noinline))
__mmask16 compute_mask16(int seed) {
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        mask |= ((seed + i * 3) % 7 == 0) ? (1 << i) : 0;
    }
    return mask;
}

__attribute__((noinline))
__mmask8 compute_mask8(int seed) {
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        mask |= ((seed + i * 5) % 11 == 0) ? (1 << i) : 0;
    }
    return mask;
}

/* V64QI blend function - triggers case E_V64QImode */
__attribute__((target("avx512bw"), noinline))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    /* Force RTL expansion with runtime mask */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Additional data-dependent computation */
    __m512i cmp = _mm512_cmpeq_epi8_mask(a, b);
    __m512i masked = _mm512_mask_blend_epi8(cmp, result, a);
    
    /* Store to volatile to prevent elimination */
    v64qi_result = masked;
    
    return masked;
}

/* V32HI blend function - triggers case E_V32HImode */
__attribute__((target("avx512bw"), noinline))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    /* Mixed-width operation to stress expander */
    __m512i shifted = _mm512_slli_epi16(result, 3);
    __m512i blended = _mm512_mask_blend_epi16(mask >> 1, shifted, b);
    
    v32hi_result = blended;
    return blended;
}

/* V32HF blend function - triggers case E_V32HFmode */
__attribute__((target("avx512bw,avx512fp16"), noinline))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Arithmetic to create data dependency */
    __m512h half = _mm512_set1_ph(0.5f);
    __m512h scaled = _mm512_mul_ph(result, half);
    __m512h final = _mm512_mask_blend_ph(mask ^ 0xAAAAAAAA, scaled, b);
    
    v32hf_result = final;
    return final;
}

/* V32BF blend function - triggers case E_V32BFmode */
__attribute__((target("avx512bw,avx512bf16"), noinline))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Type conversion sequence */
    __m512i as_int = _mm512_castph_si512(result);
    __m512i shifted = _mm512_slli_epi16(as_int, 1);
    __m512bh converted = _mm512_castsi512_ph(shifted);
    __m512bh final = _mm512_mask_blend_ph(mask | 0x55555555, converted, b);
    
    v32bf_result = final;
    return final;
}

/* V16SI blend function - triggers case E_V16SImode */
__attribute__((target("avx512f"), noinline))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    /* Multi-stage pipeline: result feeds into another blend */
    __m512i abs_val = _mm512_abs_epi32(result);
    __m512i final = _mm512_mask_blend_epi32(mask ^ 0xAAAA, abs_val, b);
    
    v16si_result = final;
    return final;
}

/* V8DI blend function - triggers case E_V8DImode */
__attribute__((target("avx512f"), noinline))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    /* Cross-type operation */
    __m512d as_double = _mm512_castsi512_pd(result);
    __m512d sqrt_val = _mm512_sqrt_pd(as_double);
    __m512i back_to_int = _mm512_castpd_si512(sqrt_val);
    __m512i final = _mm512_mask_blend_epi64(mask | 0xAA, back_to_int, b);
    
    v8di_result = final;
    return final;
}

/* V8DF blend function - triggers case E_V8DFmode */
__attribute__((target("avx512f"), noinline))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    /* Complex floating-point computation */
    __m512d recip = _mm512_rcp14_pd(result);
    __m512d blended = _mm512_mask_blend_pd(mask ^ 0x55, recip, b);
    
    /* Store through volatile pointer */
    volatile __m512d* volatile_ptr = &v8df_result;
    *volatile_ptr = blended;
    
    return blended;
}

/* V16SF blend function - triggers case E_V16SFmode */
__attribute__((target("avx512f"), noinline))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    /* Mixed precision computation */
    __m512 rsqrt = _mm512_rsqrt14_ps(result);
    __m512 scaled = _mm512_mul_ps(rsqrt, _mm512_set1_ps(2.0f));
    __m512 final = _mm512_mask_blend_ps(mask & 0x5555, scaled, b);
    
    v16sf_result = final;
    return final;
}

/* Multi-stage pipeline: V64QI -> V16SI conversion and blend */
__attribute__((noinline))
int pipeline_v64qi_to_v16si(int seed) {
    /* Create V64QI vectors */
    char data_a[64], data_b[64];
    for (int i = 0; i < 64; i++) {
        data_a[i] = (char)(seed + i);
        data_b[i] = (char)(seed - i);
    }
    
    __m512i a = _mm512_loadu_si512(data_a);
    __m512i b = _mm512_loadu_si512(data_b);
    __mmask64 mask64 = compute_mask64(seed);
    
    /* First blend: V64QI */
    __m512i blended_qi = blend_v64qi(a, b, mask64);
    
    /* Convert to V16SI for second blend */
    __m512i a_si = _mm512_cvtepi8_epi32(_mm512_castsi512_si128(blended_qi));
    __m512i b_si = _mm512_cvtepi8_epi32(_mm512_castsi512_si128(b));
    __mmask16 mask16 = compute_mask16(seed);
    
    /* Second blend: V16SI */
    __m512i blended_si = blend_v16si(a_si, b_si, mask16);
    
    /* Horizontal sum for checksum */
    return _mm512_reduce_add_epi32(blended_si);
}

int main(int argc, char** argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    int checksum = 0;
    
    /* Initialize test data with runtime values */
    char char_data[64];
    short short_data[32];
    int int_data[16];
    long long long_data[8];
    float float_data[16];
    double double_data[8];
    __fp16 fp16_data[32];
    __bf16 bf16_data[32];
    
    for (int i = 0; i < 64; i++) char_data[i] = rand() % 256;
    for (int i = 0; i < 32; i++) short_data[i] = rand() % 65536;
    for (int i = 0; i < 16; i++) int_data[i] = rand();
    for (int i = 0; i < 8; i++) long_data[i] = ((long long)rand() << 32) | rand();
    for (int i = 0; i < 16; i++) float_data[i] = (float)rand() / RAND_MAX;
    for (int i = 0; i < 8; i++) double_data[i] = (double)rand() / RAND_MAX;
    for (int i = 0; i < 32; i++) fp16_data[i] = (__fp16)((float)rand() / RAND_MAX);
    for (int i = 0; i < 32; i++) bf16_data[i] = (__bf16)((float)rand() / RAND_MAX);
    
    /* Execute all blend functions with runtime masks */
    
    /* V64QI */
    __m512i a64qi = _mm512_loadu_si512(char_data);
    __m512i b64qi = _mm512_loadu_si512(char_data + 32);
    __mmask64 mask64 = compute_mask64(seed);
    __m512i res64qi = blend_v64qi(a64qi, b64qi, mask64);
    checksum += _mm512_reduce_add_epi8(res64qi);
    
    /* V32HI */
    __m512i a32hi = _mm512_loadu_si512(short_data);
    __m512i b32hi = _mm512_loadu_si512(short_data + 16);
    __mmask32 mask32 = compute_mask32(seed);
    __m512i res32hi = blend_v32hi(a32hi, b32hi, mask32);
    checksum += _mm512_reduce_add_epi16(res32hi);
    
    /* V32HF */
    __m512h a32hf = _mm512_loadu_ph(fp16_data);
    __m512h b32hf = _mm512_loadu_ph(fp16_data + 16);
    __m512h res32hf = blend_v32hf(a32hf, b32hf, mask32);
    /* Reduce manually for fp16 */
    for (int i = 0; i < 32; i++) checksum += (int)fp16_data[i];
    
    /* V32BF */
    __m512bh a32bf = _mm512_loadu_ph(bf16_data);
    __m512bh b32bf = _mm512_loadu_ph(bf16_data + 16);
    __m512bh res32bf = blend_v32bf(a32bf, b32bf, mask32);
    for (int i = 0; i < 32; i++) checksum += (int)bf16_data[i];
    
    /* V16SI */
    __m512i a16si = _mm512_loadu_si512(int_data);
    __m512i b16si = _mm512_loadu_si512(int_data + 8);
    __mmask16 mask16 = compute_mask16(seed);
    __m512i res16si = blend_v16si(a16si, b16si, mask16);
    checksum += _mm512_reduce_add_epi32(res16si);
    
    /* V8DI */
    __m512i a8di = _mm512_loadu_si512(long_data);
    __m512i b8di = _mm512_loadu_si512(long_data + 4);
    __mmask8 mask8 = compute_mask8(seed);
    __m512i res8di = blend_v8di(a8di, b8di, mask8);
    checksum += (int)_mm512_reduce_add_epi64(res8di);
    
    /* V8DF */
    __m512d a8df = _mm512_loadu_pd(double_data);
    __m512d b8df = _mm512_loadu_pd(double_data + 4);
    __m512d res8df = blend_v8df(a8df, b8df, mask8);
    /* Reduce manually for double */
    for (int i = 0; i < 8; i++) checksum += (int)double_data[i];
    
    /* V16SF */
    __m512 a16sf = _mm512_loadu_ps(float_data);
    __m512 b16sf = _mm512_loadu_ps(float_data + 8);
    __m512 res16sf = blend_v16sf(a16sf, b16sf, mask16);
    /* Reduce manually for float */
    for (int i = 0; i < 16; i++) checksum += (int)float_data[i];
    
    /* Execute multi-stage pipeline */
    checksum += pipeline_v64qi_to_v16si(seed);
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
