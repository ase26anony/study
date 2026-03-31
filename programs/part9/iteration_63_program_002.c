#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent dead code elimination */
__attribute__((noinline, noipa)) 
void use_result(void* ptr) {
    /* Use inline assembly as a compiler barrier */
    __asm__ volatile ("" : : "r"(ptr) : "memory");
}

/* Simple LCG for pseudo-random values */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize arrays with pseudo-random data */
static void init_array(void* arr, size_t size) {
    uint32_t* ptr = (uint32_t*)arr;
    size_t words = (size + 3) / 4;
    for (size_t i = 0; i < words; i++) {
        ptr[i] = lcg_rand();
    }
}

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QI: 64-byte integer (char) */
void blend_v64qi(uint8_t* dst, const uint8_t* src1, const uint8_t* src2, size_t n) {
    for (size_t i = 0; i < n; i += 64) {
        __m512i v1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        
        /* Data-dependent mask: compare elements for inequality */
        __mmask64 mask = _mm512_cmpneq_epi8_mask(v1, v2);
        
        /* This should trigger gen_avx512bw_blendmv64qi */
        __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
        
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V32HI: 32-word integer (short) */
void blend_v32hi(int16_t* dst, const int16_t* src1, const int16_t* src2, size_t n) {
    for (size_t i = 0; i < n; i += 32) {
        __m512i v1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        
        /* Data-dependent mask: compare for less-than */
        __mmask32 mask = _mm512_cmplt_epi16_mask(v1, v2);
        
        /* This should trigger gen_avx512bw_blendmv32hi */
        __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
        
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V32HF: 32-half-float */
void blend_v32hf(_Float16* dst, const _Float16* src1, const _Float16* src2, size_t n) {
    for (size_t i = 0; i < n; i += 32) {
        __m512h v1 = _mm512_loadu_ph(src1 + i);
        __m512h v2 = _mm512_loadu_ph(src2 + i);
        
        /* Data-dependent mask: compare for ordered less-than */
        __mmask32 mask = _mm512_cmplt_ph_mask(v1, v2);
        
        /* This should trigger gen_avx512bw_blendmv32hf */
        __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
        
        _mm512_storeu_ph(dst + i, result);
    }
}

/* V32BF: 32-bfloat16 */
void blend_v32bf(__bf16* dst, const __bf16* src1, const __bf16* src2, size_t n) {
    for (size_t i = 0; i < n; i += 32) {
        /* Load as epi16 and convert to bfloat16 vectors */
        __m512bh v1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512bh v2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        
        /* Convert to float for comparison */
        __m512 f1 = _mm512_cvtpbh_ps(v1);
        __m512 f2 = _mm512_cvtpbh_ps(v2);
        
        /* Data-dependent mask */
        __mmask16 mask_f32 = _mm512_cmp_ps_mask(f1, f2, _CMP_LT_OQ);
        
        /* Expand mask from 16-bit to 32-bit for bfloat16 blend */
        __mmask32 mask = _cvtu32_mask32(_cvtmask16_u32(mask_f32));
        mask = _knot_mask32(mask); /* Invert to ensure non-constant pattern */
        
        /* This should trigger gen_avx512bw_blendmv32bf */
        __m512bh result = _mm512_mask_blend_ph(mask, v1, v2);
        
        _mm512_storeu_si512((__m512i*)(dst + i), (__m512i)result);
    }
}

/* V16SI: 16-dword integer (int) */
void blend_v16si(int32_t* dst, const int32_t* src1, const int32_t* src2, size_t n) {
    for (size_t i = 0; i < n; i += 16) {
        __m512i v1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        
        /* Data-dependent mask: compare for equality */
        __mmask16 mask = _mm512_cmpeq_epi32_mask(v1, v2);
        
        /* This should trigger gen_avx512f_blendmv16si */
        __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
        
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V8DI: 8-qword integer (long) */
void blend_v8di(int64_t* dst, const int64_t* src1, const int64_t* src2, size_t n) {
    for (size_t i = 0; i < n; i += 8) {
        __m512i v1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        
        /* Data-dependent mask: compare for greater-than */
        __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, v2);
        
        /* This should trigger gen_avx512f_blendmv8di */
        __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
        
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V8DF: 8-double */
void blend_v8df(double* dst, const double* src1, const double* src2, size_t n) {
    for (size_t i = 0; i < n; i += 8) {
        __m512d v1 = _mm512_loadu_pd(src1 + i);
        __m512d v2 = _mm512_loadu_pd(src2 + i);
        
        /* Data-dependent mask: compare for not-equal */
        __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_NEQ_OQ);
        
        /* This should trigger gen_avx512f_blendmv8df */
        __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
        
        _mm512_storeu_pd(dst + i, result);
    }
}

/* V16SF: 16-single-float */
void blend_v16sf(float* dst, const float* src1, const float* src2, size_t n) {
    for (size_t i = 0; i < n; i += 16) {
        __m512 v1 = _mm512_loadu_ps(src1 + i);
        __m512 v2 = _mm512_loadu_ps(src2 + i);
        
        /* Data-dependent mask: compare for ordered greater-than */
        __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_GT_OQ);
        
        /* This should trigger gen_avx512f_blendmv16sf */
        __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
        
        _mm512_storeu_ps(dst + i, result);
    }
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

int main(void) {
    /* Runtime CPU feature detection */
    if (!__builtin_cpu_supports("avx512f") || !__builtin_cpu_supports("avx512bw")) {
        printf("AVX-512F and AVX-512BW not supported on this CPU\n");
        return 0;
    }
    
    const size_t ARRAY_SIZE = 1024;
    const size_t ITERATIONS = 100;
    
    /* Allocate and initialize arrays for each type */
    
    /* V64QI */
    uint8_t src1_v64qi[ARRAY_SIZE];
    uint8_t src2_v64qi[ARRAY_SIZE];
    uint8_t dst_v64qi[ARRAY_SIZE];
    
    /* V32HI */
    int16_t src1_v32hi[ARRAY_SIZE];
    int16_t src2_v32hi[ARRAY_SIZE];
    int16_t dst_v32hi[ARRAY_SIZE];
    
    /* V32HF */
    _Float16 src1_v32hf[ARRAY_SIZE];
    _Float16 src2_v32hf[ARRAY_SIZE];
    _Float16 dst_v32hf[ARRAY_SIZE];
    
    /* V32BF */
    __bf16 src1_v32bf[ARRAY_SIZE];
    __bf16 src2_v32bf[ARRAY_SIZE];
    __bf16 dst_v32bf[ARRAY_SIZE];
    
    /* V16SI */
    int32_t src1_v16si[ARRAY_SIZE];
    int32_t src2_v16si[ARRAY_SIZE];
    int32_t dst_v16si[ARRAY_SIZE];
    
    /* V8DI */
    int64_t src1_v8di[ARRAY_SIZE];
    int64_t src2_v8di[ARRAY_SIZE];
    int64_t dst_v8di[ARRAY_SIZE];
    
    /* V8DF */
    double src1_v8df[ARRAY_SIZE];
    double src2_v8df[ARRAY_SIZE];
    double dst_v8df[ARRAY_SIZE];
    
    /* V16SF */
    float src1_v16sf[ARRAY_SIZE];
    float src2_v16sf[ARRAY_SIZE];
    float dst_v16sf[ARRAY_SIZE];
    
    /* Initialize all arrays */
    init_array(src1_v64qi, sizeof(src1_v64qi));
    init_array(src2_v64qi, sizeof(src2_v64qi));
    
    init_array(src1_v32hi, sizeof(src1_v32hi));
    init_array(src2_v32hi, sizeof(src2_v32hi));
    
    init_array(src1_v32hf, sizeof(src1_v32hf));
    init_array(src2_v32hf, sizeof(src2_v32hf));
    
    init_array(src1_v32bf, sizeof(src1_v32bf));
    init_array(src2_v32bf, sizeof(src2_v32bf));
    
    init_array(src1_v16si, sizeof(src1_v16si));
    init_array(src2_v16si, sizeof(src2_v16si));
    
    init_array(src1_v8di, sizeof(src1_v8di));
    init_array(src2_v8di, sizeof(src2_v8di));
    
    init_array(src1_v8df, sizeof(src1_v8df));
    init_array(src2_v8df, sizeof(src2_v8df));
    
    init_array(src1_v16sf, sizeof(src1_v16sf));
    init_array(src2_v16sf, sizeof(src2_v16sf));
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    
    /* Execute blend operations multiple times to encourage vectorization */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Update source arrays slightly each iteration */
        for (size_t i = 0; i < ARRAY_SIZE; i++) {
            src1_v64qi[i] += iter;
            src2_v64qi[i] += iter * 2;
            
            if (i % 32 == 0) {
                src1_v32hi[i/2] += iter;
                src2_v32hi[i/2] -= iter;
                
                src1_v32hf[i/2] += iter * 0.1f;
                src2_v32hf[i/2] -= iter * 0.1f;
                
                src1_v32bf[i/2] += iter * 0.05f;
                src2_v32bf[i/2] -= iter * 0.05f;
            }
            
            if (i % 64 == 0) {
                src1_v16si[i/4] += iter;
                src2_v16si[i/4] -= iter;
                
                src1_v16sf[i/4] += iter * 0.01f;
                src2_v16sf[i/4] -= iter * 0.01f;
            }
            
            if (i % 128 == 0) {
                src1_v8di[i/8] += iter;
                src2_v8di[i/8] -= iter;
                
                src1_v8df[i/8] += iter * 0.001;
                src2_v8df[i/8] -= iter * 0.001;
            }
        }
        
        /* Perform blends for each vector mode */
        blend_v64qi(dst_v64qi, src1_v64qi, src2_v64qi, ARRAY_SIZE);
        blend_v32hi(dst_v32hi, src1_v32hi, src2_v32hi, ARRAY_SIZE / 2);
        blend_v32hf(dst_v32hf, src1_v32hf, src2_v32hf, ARRAY_SIZE / 2);
        blend_v32bf(dst_v32bf, src1_v32bf, src2_v32bf, ARRAY_SIZE / 2);
        blend_v16si(dst_v16si, src1_v16si, src2_v16si, ARRAY_SIZE / 4);
        blend_v8di(dst_v8di, src1_v8di, src2_v8di, ARRAY_SIZE / 8);
        blend_v8df(dst_v8df, src1_v8df, src2_v8df, ARRAY_SIZE / 8);
        blend_v16sf(dst_v16sf, src1_v16sf, src2_v16sf, ARRAY_SIZE / 4);
    }
    
#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */
    
    /* Force compiler to materialize all results */
    use_result(dst_v64qi);
    use_result(dst_v32hi);
    use_result(dst_v32hf);
    use_result(dst_v32bf);
    use_result(dst_v16si);
    use_result(dst_v8di);
    use_result(dst_v8df);
    use_result(dst_v16sf);
    
    return 0;
}
