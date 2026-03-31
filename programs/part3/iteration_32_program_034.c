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

/* Data-dependent computation with runtime masks */
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    /* Force RTL expansion by using intrinsic with runtime mask */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Additional data-dependent operations to prevent folding */
    __m512i cmp = _mm512_cmpeq_epi8_mask(a, b);
    __m512i masked = _mm512_mask_blend_epi8(cmp, result, a);
    
    /* Store to volatile to prevent elimination */
    v64qi_result = masked;
    
    return masked;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    /* Multi-stage blending with computed predicate */
    __m512i temp = _mm512_mask_blend_epi16(mask, a, b);
    
    /* Create another mask from comparison */
    __mmask32 cmp_mask = _mm512_cmpeq_epi16_mask(a, b);
    __m512i result = _mm512_mask_blend_epi16(cmp_mask, temp, a);
    
    v32hi_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    /* Half-precision blending with type conversions */
    __m512h blended = _mm512_mask_blend_ph(mask, a, b);
    
    /* Additional operation to prevent constant folding */
    __m512h abs_result = _mm512_abs_ph(blended);
    
    v32hf_result = abs_result;
    return abs_result;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    /* BF16 blending - requires explicit cast */
    __m512bh blended = _mm512_mask_blend_ph(mask, a, b);
    
    v32bf_result = blended;
    return blended;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    /* Integer 32-bit blending with arithmetic */
    __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
    
    /* Mix with other operations */
    __m512i added = _mm512_add_epi32(blended, a);
    __m512i result = _mm512_mask_blend_epi32(mask >> 1, added, b);
    
    v16si_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    /* 64-bit integer blending */
    __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
    
    /* Chain multiple blends */
    __mmask8 alt_mask = mask ^ 0xFF;
    __m512i result = _mm512_mask_blend_epi64(alt_mask, blended, a);
    
    v8di_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    /* Double-precision floating point blending */
    __m512d blended = _mm512_mask_blend_pd(mask, a, b);
    
    /* Mathematical operation to prevent elimination */
    __m512d sqrt_result = _mm512_sqrt_pd(blended);
    
    v8df_result = sqrt_result;
    return sqrt_result;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    /* Single-precision floating point blending */
    __m512 blended = _mm512_mask_blend_ps(mask, a, b);
    
    /* Complex operation chain */
    __m512 recip = _mm512_rcp14_ps(blended);
    __m512 result = _mm512_mask_blend_ps(mask >> 1, recip, a);
    
    v16sf_result = result;
    return result;
}

/* Mixed precision pipeline function */
__attribute__((target("avx512f,avx512bw")))
uint64_t mixed_pipeline(unsigned seed) {
    /* Initialize with seed-dependent values */
    char char_data[64];
    short short_data[32];
    int int_data[16];
    long long long_data[8];
    float float_data[16];
    double double_data[8];
    
    /* Fill arrays with pseudo-random values */
    for (int i = 0; i < 64; i++) {
        char_data[i] = (char)((seed + i * 13) % 256 - 128);
        if (i < 32) short_data[i] = (short)((seed + i * 17) % 65536 - 32768);
        if (i < 16) int_data[i] = (int)((seed + i * 19) * 1103515245);
        if (i < 8) {
            long_data[i] = (long long)((seed + i * 23) * 1103515245);
            double_data[i] = (double)((seed + i * 29) % 100) / 10.0;
        }
        if (i < 16) float_data[i] = (float)((seed + i * 31) % 100) / 10.0f;
    }
    
    /* Stage 1: Blend 64x char (V64QI) */
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)char_data);
    __m512i v64qi_b = _mm512_loadu_si512((__m512i*)(char_data + 32));
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        if ((char_data[i] & 1) != 0) mask64 |= (1ULL << i);
    }
    __m512i v64qi_result = blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    /* Stage 2: Convert and blend 32x short (V32HI) */
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)short_data);
    __m512i v32hi_b = _mm512_slli_epi16(v32hi_a, 1);
    __mmask32 mask32 = 0;
    for (int i = 0; i < 32; i++) {
        if ((short_data[i] & 2) != 0) mask32 |= (1U << i);
    }
    __m512i v32hi_result = blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    /* Stage 3: Blend 16x int (V16SI) */
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)int_data);
    __m512i v16si_b = _mm512_add_epi32(v16si_a, _mm512_set1_epi32(1));
    __mmask16 mask16 = 0;
    for (int i = 0; i < 16; i++) {
        if ((int_data[i] & 4) != 0) mask16 |= (1 << i);
    }
    __m512i v16si_result = blend_v16si(v16si_a, v16si_b, mask16);
    
    /* Stage 4: Blend 8x long (V8DI) */
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)long_data);
    __m512i v8di_b = _mm512_slli_epi64(v8di_a, 2);
    __mmask8 mask8 = 0;
    for (int i = 0; i < 8; i++) {
        if ((long_data[i] & 8) != 0) mask8 |= (1 << i);
    }
    __m512i v8di_result = blend_v8di(v8di_a, v8di_b, mask8);
    
    /* Stage 5: Blend 16x float (V16SF) */
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_mul_ps(v16sf_a, _mm512_set1_ps(2.0f));
    __mmask16 mask16f = 0;
    for (int i = 0; i < 16; i++) {
        if (((int)float_data[i] & 1) != 0) mask16f |= (1 << i);
    }
    __m512 v16sf_result = blend_v16sf(v16sf_a, v16sf_b, mask16f);
    
    /* Stage 6: Blend 8x double (V8DF) */
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_div_pd(v8df_a, _mm512_set1_pd(2.0));
    __mmask8 mask8d = 0;
    for (int i = 0; i < 8; i++) {
        if (((int)double_data[i] & 1) != 0) mask8d |= (1 << i);
    }
    __m512d v8df_result = blend_v8df(v8df_a, v8df_b, mask8d);
    
    /* Compute checksum from all results */
    uint64_t checksum = 0;
    
    /* Reduce v64qi_result */
    __m512i sum64qi = _mm512_sad_epu8(v64qi_result, _mm512_setzero_si512());
    checksum += _mm512_extract_epi64(sum64qi, 0);
    checksum += _mm512_extract_epi64(sum64qi, 1);
    
    /* Reduce v32hi_result */
    __m512i sum32hi = _mm512_madd_epi16(v32hi_result, _mm512_set1_epi16(1));
    checksum += _mm512_extract_epi64(sum32hi, 0);
    checksum += _mm512_extract_epi64(sum32hi, 1);
    
    /* Reduce v16si_result */
    __m512i sum16si = _mm512_add_epi32(v16si_result, _mm512_setzero_si512());
    for (int i = 0; i < 4; i++) {
        checksum += _mm512_extract_epi64(sum16si, i);
    }
    
    /* Reduce v8di_result */
    for (int i = 0; i < 8; i++) {
        checksum += _mm512_extract_epi64(v8di_result, i);
    }
    
    return checksum;
}

int main(int argc, char** argv) {
    /* Use argc as seed for runtime variability */
    unsigned seed = (unsigned)argc;
    
    /* Initialize test data arrays */
    char char_arr[64];
    short short_arr[32];
    int int_arr[16];
    long long long_arr[8];
    float float_arr[16];
    double double_arr[8];
    
    /* Fill with seed-dependent values */
    for (int i = 0; i < 64; i++) {
        char_arr[i] = (char)((seed + i * 37) % 256);
        if (i < 32) short_arr[i] = (short)((seed + i * 41) % 65536);
        if (i < 16) int_arr[i] = (int)((seed + i * 43) * 1664525 + 1013904223);
        if (i < 8) {
            long_arr[i] = (long long)((seed + i * 47) * 1664525LL + 1013904223LL);
            double_arr[i] = (double)((seed + i * 53) % 1000) / 100.0;
        }
        if (i < 16) float_arr[i] = (float)((seed + i * 59) % 1000) / 100.0f;
    }
    
    /* Call all blend functions with runtime masks */
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)char_arr);
    __m512i v64qi_b = _mm512_loadu_si512((__m512i*)(char_arr + 32));
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL; /* Alternating pattern */
    blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)short_arr);
    __m512i v32hi_b = _mm512_srli_epi16(v32hi_a, 1);
    __mmask32 mask32 = 0x55555555; /* Alternating pattern */
    blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    /* For half-precision types, we need to use appropriate initializers */
    __m512h v32hf_a = _mm512_setzero_ph();
    __m512h v32hf_b = _mm512_set1_ph((_Float16)1.0);
    blend_v32hf(v32hf_a, v32hf_b, mask32);
    
    __m512bh v32bf_a = _mm512_setzero_bh();
    __m512bh v32bf_b = _mm512_set1_bh((__bf16)1.0);
    blend_v32bf(v32bf_a, v32bf_b, mask32);
    
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)int_arr);
    __m512i v16si_b = _mm512_sub_epi32(v16si_a, _mm512_set1_epi32(1));
    __mmask16 mask16 = 0xAAAA; /* Alternating pattern */
    blend_v16si(v16si_a, v16si_b, mask16);
    
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)long_arr);
    __m512i v8di_b = _mm512_srli_epi64(v8di_a, 1);
    __mmask8 mask8 = 0xAA; /* Alternating pattern */
    blend_v8di(v8di_a, v8di_b, mask8);
    
    __m512d v8df_a = _mm512_loadu_pd(double_arr);
    __m512d v8df_b = _mm512_add_pd(v8df_a, _mm512_set1_pd(1.0));
    blend_v8df(v8df_a, v8df_b, mask8);
    
    __m512 v16sf_a = _mm512_loadu_ps(float_arr);
    __m512 v16sf_b = _mm512_sub_ps(v16sf_a, _mm512_set1_ps(1.0f));
    blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    /* Execute mixed precision pipeline */
    uint64_t checksum = mixed_pipeline(seed);
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
