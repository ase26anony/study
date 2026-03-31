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

/* Function declarations with explicit target attributes */
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    /* Force RTL expansion for V64QImode */
    return _mm512_mask_blend_epi8(mask, a, b);
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    /* Force RTL expansion for V32HImode */
    return _mm512_mask_blend_epi16(mask, a, b);
}

__attribute__((target("avx512fp16,avx512bw")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    /* Force RTL expansion for V32HFmode */
    return _mm512_mask_blend_ph(mask, a, b);
}

__attribute__((target("avx512bf16,avx512bw")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    /* Force RTL expansion for V32BFmode */
    return _mm512_mask_blend_ph(mask, a, b);
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    /* Force RTL expansion for V16SImode */
    return _mm512_mask_blend_epi32(mask, a, b);
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    /* Force RTL expansion for V8DImode */
    return _mm512_mask_blend_epi64(mask, a, b);
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    /* Force RTL expansion for V8DFmode */
    return _mm512_mask_blend_pd(mask, a, b);
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    /* Force RTL expansion for V16SFmode */
    return _mm512_mask_blend_ps(mask, a, b);
}

/* Multi-stage pipeline: V64QI -> V32HI -> V16SI -> V8DI */
__attribute__((target("avx512bw,avx512f")))
uint64_t pipeline_blend_operations(int seed) {
    /* Stage 1: V64QI blending with data-dependent mask */
    char data1[64], data2[64];
    for (int i = 0; i < 64; i++) {
        data1[i] = (char)((i + seed) % 128);
        data2[i] = (char)((i * 3 + seed) % 128);
    }
    
    __m512i v64qi_a = _mm512_loadu_si512(data1);
    __m512i v64qi_b = _mm512_loadu_si512(data2);
    
    /* Create runtime-dependent mask */
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        if ((i + seed) % 3 == 0) {
            mask64 |= (1ULL << i);
        }
    }
    
    __m512i v64qi_res = blend_v64qi(v64qi_a, v64qi_b, mask64);
    v64qi_result = v64qi_res; /* Volatile store */
    
    /* Stage 2: Convert to V32HI and blend */
    __m512i v32hi_a = _mm512_srai_epi16(v64qi_res, 2); /* Arithmetic shift */
    __m512i v32hi_b = _mm512_slli_epi16(v64qi_res, 1); /* Logical shift */
    
    __mmask32 mask32 = 0;
    for (int i = 0; i < 32; i++) {
        if ((i * seed) % 5 == 0) {
            mask32 |= (1U << i);
        }
    }
    
    __m512i v32hi_res = blend_v32hi(v32hi_a, v32hi_b, mask32);
    v32hi_result = v32hi_res;
    
    /* Stage 3: Convert to V16SI and blend */
    __m512i v16si_a = _mm512_srai_epi32(v32hi_res, 4);
    __m512i v16si_b = _mm512_slli_epi32(v32hi_res, 2);
    
    __mmask16 mask16 = 0;
    for (int i = 0; i < 16; i++) {
        mask16 |= ((seed >> i) & 1) << i;
    }
    
    __m512i v16si_res = blend_v16si(v16si_a, v16si_b, mask16);
    v16si_result = v16si_res;
    
    /* Stage 4: Convert to V8DI and blend */
    __m512i v8di_a = _mm512_srai_epi64(v16si_res, 8);
    __m512i v8di_b = _mm512_slli_epi64(v16si_res, 4);
    
    __mmask8 mask8 = (__mmask8)(seed & 0xFF);
    
    __m512i v8di_res = blend_v8di(v8di_a, v8di_b, mask8);
    v8di_result = v8di_res;
    
    /* Horizontal sum of all results */
    uint64_t sum = 0;
    char* ptr = (char*)&v8di_res;
    for (int i = 0; i < 64; i++) {
        sum += ptr[i];
    }
    
    return sum;
}

/* Floating-point pipeline: V16SF -> V8DF with type conversions */
__attribute__((target("avx512f")))
double fp_pipeline_blend(int seed) {
    /* Stage 1: V16SF blending */
    float fdata1[16], fdata2[16];
    for (int i = 0; i < 16; i++) {
        fdata1[i] = (float)((i + seed) * 0.1f);
        fdata2[i] = (float)((i * 2 + seed) * 0.2f);
    }
    
    __m512 v16sf_a = _mm512_loadu_ps(fdata1);
    __m512 v16sf_b = _mm512_loadu_ps(fdata2);
    
    __mmask16 mask16_fp = 0;
    for (int i = 0; i < 16; i++) {
        if (fdata1[i] > fdata2[i]) {
            mask16_fp |= (1U << i);
        }
    }
    
    __m512 v16sf_res = blend_v16sf(v16sf_a, v16sf_b, mask16_fp);
    v16sf_result = v16sf_res;
    
    /* Stage 2: Convert to V8DF and blend */
    __m512d v8df_a = _mm512_cvtps_pd(_mm512_extractf32x8_ps(v16sf_res, 0));
    __m512d v8df_b = _mm512_cvtps_pd(_mm512_extractf32x8_ps(v16sf_res, 1));
    
    __mmask8 mask8_fp = 0;
    double d1[8], d2[8];
    _mm512_storeu_pd(d1, v8df_a);
    _mm512_storeu_pd(d2, v8df_b);
    for (int i = 0; i < 8; i++) {
        if (d1[i] != d2[i]) {
            mask8_fp |= (1U << i);
        }
    }
    
    __m512d v8df_res = blend_v8df(v8df_a, v8df_b, mask8_fp);
    v8df_result = v8df_res;
    
    /* Horizontal sum */
    return _mm512_reduce_add_pd(v8df_res);
}

/* Half-precision and bfloat16 blending */
__attribute__((target("avx512fp16,avx512bf16,avx512bw")))
uint32_t half_precision_blend(int seed) {
    /* V32HF blending */
    _Float16 hdata1[32], hdata2[32];
    for (int i = 0; i < 32; i++) {
        hdata1[i] = (_Float16)((i + seed) * 0.05f);
        hdata2[i] = (_Float16)((i * 3 + seed) * 0.1f);
    }
    
    __m512h v32hf_a = _mm512_loadu_ph(hdata1);
    __m512h v32hf_b = _mm512_loadu_ph(hdata2);
    
    __mmask32 mask32_hf = 0;
    for (int i = 0; i < 32; i++) {
        if ((i + seed) % 4 == 0) {
            mask32_hf |= (1U << i);
        }
    }
    
    __m512h v32hf_res = blend_v32hf(v32hf_a, v32hf_b, mask32_hf);
    v32hf_result = v32hf_res;
    
    /* V32BF blending - convert from half-precision */
    __m512bh v32bf_a = _mm512_cvtph_bf16(v32hf_res);
    __m512bh v32bf_b = _mm512_slli_epi16((__m512i)v32bf_a, 1);
    
    __mmask32 mask32_bf = ~mask32_hf; /* Complementary mask */
    
    __m512bh v32bf_res = blend_v32bf(v32bf_a, v32bf_b, mask32_bf);
    v32bf_result = v32bf_res;
    
    /* Compute checksum */
    uint16_t* ptr = (uint16_t*)&v32bf_res;
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += ptr[i];
    }
    
    return sum;
}

/* Main test driver */
int main(int argc, char** argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Testing AVX-512 blend intrinsics coverage...\n");
    
    /* Execute all blend operations */
    uint64_t int_sum = pipeline_blend_operations(seed);
    double fp_sum = fp_pipeline_blend(seed);
    uint32_t half_sum = half_precision_blend(seed);
    
    /* Additional standalone tests to ensure all cases are hit */
    {
        /* V64QI standalone */
        char data[64];
        for (int i = 0; i < 64; i++) data[i] = i;
        __m512i a = _mm512_loadu_si512(data);
        __m512i b = _mm512_set1_epi8(0xFF);
        __mmask64 m = 0xAAAAAAAAAAAAAAAAULL;
        __m512i res = blend_v64qi(a, b, m);
        v64qi_result = res;
    }
    
    {
        /* V32HI standalone */
        short data[32];
        for (int i = 0; i < 32; i++) data[i] = i * 2;
        __m512i a = _mm512_loadu_si512(data);
        __m512i b = _mm512_set1_epi16(0x7FFF);
        __mmask32 m = 0x55555555;
        __m512i res = blend_v32hi(a, b, m);
        v32hi_result = res;
    }
    
    {
        /* V16SI standalone */
        int data[16];
        for (int i = 0; i < 16; i++) data[i] = i * 1000;
        __m512i a = _mm512_loadu_si512(data);
        __m512i b = _mm512_set1_epi32(0x7FFFFFFF);
        __mmask16 m = 0xAAAA;
        __m512i res = blend_v16si(a, b, m);
        v16si_result = res;
    }
    
    {
        /* V8DI standalone */
        long long data[8];
        for (int i = 0; i < 8; i++) data[i] = i * 1000000LL;
        __m512i a = _mm512_loadu_si512(data);
        __m512i b = _mm512_set1_epi64(0x7FFFFFFFFFFFFFFFLL);
        __mmask8 m = 0xAA;
        __m512i res = blend_v8di(a, b, m);
        v8di_result = res;
    }
    
    {
        /* V8DF standalone */
        double data[8];
        for (int i = 0; i < 8; i++) data[i] = i * 1.5;
        __m512d a = _mm512_loadu_pd(data);
        __m512d b = _mm512_set1_pd(3.14159);
        __mmask8 m = 0x55;
        __m512d res = blend_v8df(a, b, m);
        v8df_result = res;
    }
    
    {
        /* V16SF standalone */
        float data[16];
        for (int i = 0; i < 16; i++) data[i] = i * 0.5f;
        __m512 a = _mm512_loadu_ps(data);
        __m512 b = _mm512_set1_ps(2.71828f);
        __mmask16 m = 0xAAAA;
        __m512 res = blend_v16sf(a, b, m);
        v16sf_result = res;
    }
    
    printf("Results: int_sum=%lu, fp_sum=%f, half_sum=%u\n", 
           int_sum, fp_sum, half_sum);
    
    /* Verify all volatile results are used */
    printf("Volatile checks: %d %d %d %d %d %d %d %d\n",
           ((char*)&v64qi_result)[0],
           ((short*)&v32hi_result)[0],
           ((_Float16*)&v32hf_result)[0],
           ((uint16_t*)&v32bf_result)[0],
           ((int*)&v16si_result)[0],
           ((long long*)&v8di_result)[0],
           ((double*)&v8df_result)[0],
           ((float*)&v16sf_result)[0]);
    
    return 0;
}
