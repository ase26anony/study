#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Simple PRNG for reproducible results */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Inhibit optimization helpers */
static inline void inhibit_opt(volatile void* p) {
    asm volatile("" : "+r"(p) : : "memory");
}

/* Force dependency chain */
static inline uint64_t create_dependency(uint64_t x) {
    uint64_t result;
    asm volatile("rorq $17, %0" : "=r"(result) : "0"(x));
    return result;
}

/* Vector types for different architectures */
#ifdef __AVX512F__
typedef __m512i v512i;
typedef __m512 v512f;
typedef __m512d v512d;
typedef __mmask16 kmask;
#endif

#ifdef __AVX2__
typedef __m256i v256i;
typedef __m256 v256f;
typedef __m256d v256d;
#endif

#ifdef __SSE4_2__
typedef __m128i v128i;
typedef __m128 v128f;
typedef __m128d v128d;
#endif

/* Complex expression with many temporaries */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args_avx512(float* output, const float* input1, 
                                  const float* input2, const float* input3,
                                  int n) {
    volatile int i = 0;
    uint64_t dep = 0xDEADBEEF;
    
    for (; i < n; i += 16) {
        /* Create many intermediate values to force complex expansion */
        __m512 v1 = _mm512_loadu_ps(&input1[i]);
        __m512 v2 = _mm512_loadu_ps(&input2[i]);
        __m512 v3 = _mm512_loadu_ps(&input3[i]);
        
        /* Complex dependency chain */
        dep = create_dependency(dep);
        uint64_t mask_val = dep & 0xFFFF;
        
        /* Create multiple mask values for blending */
        __mmask16 m1 = _mm512_int2mask(mask_val);
        __mmask16 m2 = _mm512_int2mask(mask_val >> 4);
        __mmask16 m3 = _mm512_int2mask(mask_val >> 8);
        __mmask16 m4 = _mm512_int2mask(mask_val >> 12);
        
        /* Force many argument operation through inline asm */
        __m512 result;
        asm volatile (
            "vmovaps %1, %0\n\t"
            "vblendmps %2, %3, %0{%4}\n\t"
            "vblendmps %5, %0, %0{%6}\n\t"
            "vaddps %7, %0, %0\n\t"
            "vmulps %8, %0, %0\n\t"
            "vfmsub231ps %9, %10, %0\n\t"
            : "=v"(result)
            : "v"(v1), "v"(v2), "v"(v3), "k"(m1),
              "v"(_mm512_set1_ps(2.0f)), "k"(m2),
              "v"(_mm512_set1_ps(1.5f)), "v"(_mm512_set1_ps(0.5f)),
              "v"(_mm512_set1_ps(3.0f)), "v"(_mm512_set1_ps(1.0f)),
              "m"(*input1), "m"(*input2), "m"(*input3)
            : "memory"
        );
        
        /* Another complex operation with many arguments using builtins */
        __m512 temp = _mm512_add_ps(v1, v2);
        temp = _mm512_mul_ps(temp, v3);
        
        /* Shuffle with many arguments - potentially triggering 10-11 arg optab */
        __m512 shuffled;
        asm volatile (
            "vpermps %1, %2, %0\n\t"
            : "=v"(shuffled)
            : "v"(temp), "v"(_mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)),
              "m"(*input1), "m"(*input2), "m"(*input3),
              "i"(16), "i"(32), "i"(48), "i"(64), "i"(80)
            : "memory"
        );
        
        /* Final blend with many control inputs */
        __m512 final = _mm512_mask_blend_ps(m1, result, shuffled);
        final = _mm512_mask_blend_ps(m2, final, v1);
        final = _mm512_mask_blend_ps(m3, final, v2);
        final = _mm512_mask_blend_ps(m4, final, v3);
        
        _mm512_storeu_ps(&output[i], final);
    }
}

/* Alternative implementation using vector builtins with many arguments */
__attribute__((noinline, target("avx2")))
static void test_many_args_avx2(int32_t* output, const int32_t* input1,
                                const int32_t* input2, const int32_t* input3,
                                const int32_t* input4, int n) {
    volatile int i = 0;
    
    for (; i < n; i += 8) {
        /* Load multiple vectors */
        __m256i v1 = _mm256_loadu_si256((const __m256i*)&input1[i]);
        __m256i v2 = _mm256_loadu_si256((const __m256i*)&input2[i]);
        __m256i v3 = _mm256_loadu_si256((const __m256i*)&input3[i]);
        __m256i v4 = _mm256_loadu_si256((const __m256i*)&input4[i]);
        
        /* Complex inline asm with 10-11 operands */
        __m256i result;
        asm volatile (
            "vpaddd %1, %2, %0\n\t"
            "vpsubd %3, %0, %0\n\t"
            "vpmulld %4, %0, %0\n\t"
            "vpslld $3, %0, %0\n\t"
            "vpsrld $1, %0, %0\n\t"
            "vpblendvb %5, %6, %0, %0\n\t"
            : "=x"(result)
            : "x"(v1), "x"(v2), "x"(v3), "x"(v4),
              "x"(_mm256_set1_epi32(0xFF)), "x"(_mm256_set1_epi32(0xAA)),
              "m"(*input1), "m"(*input2), "m"(*input3), "m"(*input4),
              "i"(1), "i"(2), "i"(3)
            : "memory"
        );
        
        /* Another operation with many constants */
        __m256i temp = _mm256_add_epi32(v1, _mm256_set_epi32(7,6,5,4,3,2,1,0));
        temp = _mm256_mullo_epi32(temp, _mm256_set_epi32(8,7,6,5,4,3,2,1));
        
        /* Complex shuffle with many immediate arguments */
        __m256i shuffled;
        asm volatile (
            "vpshufd $0x1B, %1, %0\n\t"
            "vpermq $0x4E, %0, %0\n\t"
            : "=x"(shuffled)
            : "x"(temp),
              "m"(*input1), "m"(*input2), "m"(*input3), "m"(*input4),
              "i"(0), "i"(1), "i"(2), "i"(3), "i"(4), "i"(5)
            : "memory"
        );
        
        /* Blend with multiple control vectors */
        __m256i mask1 = _mm256_cmpgt_epi32(v1, v2);
        __m256i mask2 = _mm256_cmpgt_epi32(v3, v4);
        __m256i final = _mm256_blendv_epi8(result, shuffled, mask1);
        final = _mm256_blendv_epi8(final, temp, mask2);
        
        _mm256_storeu_si256((__m256i*)&output[i], final);
    }
}

/* SSE version for compatibility */
__attribute__((noinline, target("sse4.2")))
static void test_many_args_sse(float* output, const float* input1,
                               const float* input2, const float* input3,
                               const float* input4, const float* input5,
                               int n) {
    volatile int i = 0;
    
    for (; i < n; i += 4) {
        /* Load 5 input vectors - many arguments */
        __m128 v1 = _mm_loadu_ps(&input1[i]);
        __m128 v2 = _mm_loadu_ps(&input2[i]);
        __m128 v3 = _mm_loadu_ps(&input3[i]);
        __m128 v4 = _mm_loadu_ps(&input4[i]);
        __m128 v5 = _mm_loadu_ps(&input5[i]);
        
        /* Complex expression with many temporaries */
        __m128 t1 = _mm_add_ps(v1, v2);
        __m128 t2 = _mm_mul_ps(v3, v4);
        __m128 t3 = _mm_sub_ps(t1, t2);
        __m128 t4 = _mm_div_ps(v5, _mm_set1_ps(2.0f));
        __m128 t5 = _mm_add_ps(t3, t4);
        
        /* Shuffle with many lane selections */
        __m128 shuffled = _mm_shuffle_ps(t5, t5, _MM_SHUFFLE(3,2,1,0));
        shuffled = _mm_shuffle_ps(shuffled, shuffled, _MM_SHUFFLE(1,0,3,2));
        
        /* Inline asm with many memory operands */
        __m128 result;
        asm volatile (
            "movaps %1, %0\n\t"
            "addps %2, %0\n\t"
            "mulps %3, %0\n\t"
            "subps %4, %0\n\t"
            "divps %5, %0\n\t"
            : "=x"(result)
            : "x"(v1), "x"(v2), "x"(v3), "x"(v4), "x"(v5),
              "m"(*input1), "m"(*input2), "m"(*input3),
              "m"(*input4), "m"(*input5)
            : "memory"
        );
        
        /* Blend with computed mask */
        __m128 mask = _mm_cmpgt_ps(v1, v2);
        __m128 final = _mm_blendv_ps(result, shuffled, mask);
        
        _mm_storeu_ps(&output[i], final);
    }
}

/* Generic integer version with many arguments */
__attribute__((noinline))
static void test_many_args_generic(int64_t* output, const int64_t* inputs, int n) {
    volatile int i = 0;
    
    for (; i < n; i += 2) {
        /* Load many scalar values */
        int64_t a = inputs[i];
        int64_t b = inputs[i + 1];
        int64_t c = inputs[(i + 2) % n];
        int64_t d = inputs[(i + 3) % n];
        int64_t e = inputs[(i + 4) % n];
        int64_t f = inputs[(i + 5) % n];
        int64_t g = inputs[(i + 6) % n];
        int64_t h = inputs[(i + 7) % n];
        int64_t j = inputs[(i + 8) % n];
        int64_t k = inputs[(i + 9) % n];
        
        /* Complex multi-statement expression with many temporaries */
        int64_t t1 = (a + b) * c;
        int64_t t2 = (d - e) ^ f;
        int64_t t3 = (g << 3) | (h >> 2);
        int64_t t4 = (j * k) + t1;
        int64_t t5 = (t2 & t3) | t4;
        int64_t t6 = (t5 << 1) ^ (t5 >> 63);
        
        /* Inline asm with 10-11 arguments */
        int64_t result;
        asm volatile (
            "mov %1, %0\n\t"
            "add %2, %0\n\t"
            "imul %3, %0\n\t"
            "xor %4, %0\n\t"
            "or %5, %0\n\t"
            "and %6, %0\n\t"
            "sal $2, %0\n\t"
            "sar $1, %0\n\t"
            : "=r"(result)
            : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f),
              "r"(g), "r"(h), "r"(j), "r"(k),
              "m"(*inputs)
            : "cc", "memory"
        );
        
        /* Final computation mixing everything */
        output[i] = result + t6;
        output[i + 1] = (result - t6) ^ (a + b + c + d + e + f + g + h + j + k);
    }
}

int main(void) {
    const int N = 1024;
    
    /* Allocate and initialize arrays with pseudo-random data */
    float* fdata1 = (float*)aligned_alloc(64, N * sizeof(float));
    float* fdata2 = (float*)aligned_alloc(64, N * sizeof(float));
    float* fdata3 = (float*)aligned_alloc(64, N * sizeof(float));
    float* fdata4 = (float*)aligned_alloc(64, N * sizeof(float));
    float* fdata5 = (float*)aligned_alloc(64, N * sizeof(float));
    float* foutput = (float*)aligned_alloc(64, N * sizeof(float));
    
    int32_t* idata1 = (int32_t*)aligned_alloc(64, N * sizeof(int32_t));
    int32_t* idata2 = (int32_t*)aligned_alloc(64, N * sizeof(int32_t));
    int32_t* idata3 = (int32_t*)aligned_alloc(64, N * sizeof(int32_t));
    int32_t* idata4 = (int32_t*)aligned_alloc(64, N * sizeof(int32_t));
    int32_t* ioutput = (int32_t*)aligned_alloc(64, N * sizeof(int32_t));
    
    int64_t* ldata = (int64_t*)aligned_alloc(64, N * sizeof(int64_t));
    int64_t* loutput = (int64_t*)aligned_alloc(64, N * sizeof(int64_t));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        fdata1[i] = (prng_next() % 1000) / 100.0f;
        fdata2[i] = (prng_next() % 1000) / 100.0f;
        fdata3[i] = (prng_next() % 1000) / 100.0f;
        fdata4[i] = (prng_next() % 1000) / 100.0f;
        fdata5[i] = (prng_next() % 1000) / 100.0f;
        
        idata1[i] = prng_next() % 1000;
        idata2[i] = prng_next() % 1000;
        idata3[i] = prng_next() % 1000;
        idata4[i] = prng_next() % 1000;
        
        ldata[i] = prng_next() % 1000;
    }
    
    /* Call test functions with many arguments */
    #ifdef __AVX512F__
    test_many_args_avx512(foutput, fdata1, fdata2, fdata3, N);
    #endif
    
    #ifdef __AVX2__
    test_many_args_avx2(ioutput, idata1, idata2, idata3, idata4, N);
    #endif
    
    test_many_args_sse(foutput, fdata1, fdata2, fdata3, fdata4, fdata5, N);
    test_many_args_generic(loutput, ldata, N);
    
    /* Compute checksums */
    float fsum = 0;
    int32_t isum = 0;
    int64_t lsum = 0;
    
    for (int i = 0; i < N; i++) {
        fsum += foutput[i];
        isum += ioutput[i];
        lsum += loutput[i];
    }
    
    printf("Float checksum: %f\n", fsum);
    printf("Int32 checksum: %d\n", isum);
    printf("Int64 checksum: %ld\n", lsum);
    
    /* Cleanup */
    free(fdata1); free(fdata2); free(fdata3); free(fdata4); free(fdata5);
    free(foutput);
    free(idata1); free(idata2); free(idata3); free(idata4);
    free(ioutput);
    free(ldata); free(loutput);
    
    return 0;
}
