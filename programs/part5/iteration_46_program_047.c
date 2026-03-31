#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

/* Simple PRNG for deterministic testing */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Prevent optimization of arguments */
#define NOOPT __attribute__((noinline, noclone))
#define VOLATILE_VAR(var) asm volatile("" : "+r"(var))

/* Target-specific functions */
#ifdef __AVX512F__
#define TARGET_ATTR __attribute__((target("avx512f,avx512bw,avx512vl")))
#elif defined(__AVX2__)
#define TARGET_ATTR __attribute__((target("avx2")))
#else
#define TARGET_ATTR
#endif

/* Complex expression with many temporaries */
NOOPT TARGET_ATTR
static void test_many_args(float* restrict out, 
                          const float* restrict in1,
                          const float* restrict in2,
                          const float* restrict in3,
                          const float* restrict in4,
                          int n) {
    volatile int i; /* Prevent loop unrolling */
    
    for (i = 0; i < n; i += 8) {
        /* Load multiple vectors */
        __m256 v1 = _mm256_loadu_ps(&in1[i]);
        __m256 v2 = _mm256_loadu_ps(&in2[i]);
        __m256 v3 = _mm256_loadu_ps(&in3[i]);
        __m256 v4 = _mm256_loadu_ps(&in4[i]);
        
        /* Create many intermediate values to force temporaries */
        __m256 t1 = _mm256_add_ps(v1, v2);
        __m256 t2 = _mm256_sub_ps(v3, v4);
        __m256 t3 = _mm256_mul_ps(t1, t2);
        __m256 t4 = _mm256_add_ps(v1, v4);
        __m256 t5 = _mm256_sub_ps(v2, v3);
        
        /* Complex shuffle with many arguments - aiming for 10+ args */
        /* Use inline asm with many operands */
        __m256 result;
        
        /* Method 1: Inline assembly with 11 operands */
        asm volatile (
            "vmovaps %1, %0\n\t"
            "vaddps %2, %0, %0\n\t"
            "vsubps %3, %0, %0\n\t"
            "vmulps %4, %0, %0\n\t"
            "vaddps %5, %0, %0\n\t"
            "vsubps %6, %0, %0\n\t"
            "vaddps %7, %0, %0\n\t"
            "vsubps %8, %0, %0\n\t"
            "vmulps %9, %0, %0\n\t"
            "vaddps %10, %0, %0"
            : "=x"(result)
            : "x"(v1), "x"(v2), "x"(v3), "x"(v4),
              "x"(t1), "x"(t2), "x"(t3), "x"(t4), "x"(t5),
              "i"(0x1F) /* immediate constant */
            : "memory"
        );
        
        /* Method 2: Complex builtin usage with many arguments */
        /* This creates a dependency chain forcing many temporaries */
        float* aligned_temp = (float*)__builtin_assume_aligned(&out[i], 32);
        
        /* Multi-statement expression with many array operations */
        for (int j = 0; j < 8; j++) {
            /* Complex expression with many operations */
            float val = 
                ((in1[i+j] * 2.0f + in2[i+j] / 3.0f) - 
                 (in3[i+j] * 4.0f - in4[i+j] / 5.0f)) *
                ((in1[i+j] + in2[i+j]) * (in3[i+j] - in4[i+j])) /
                ((in1[i+j] * in2[i+j]) + (in3[i+j] * in4[i+j]) + 1.0f);
            
            /* More operations to increase argument count */
            val = val * 2.0f - 1.0f;
            val = val / (val + 2.0f);
            val = 1.0f / (val + 1.0f);
            
            aligned_temp[j] = val;
        }
        
        /* Blend the two results */
        __m256 scalar_result = _mm256_loadu_ps(&out[i]);
        result = _mm256_add_ps(result, scalar_result);
        
        _mm256_storeu_ps(&out[i], result);
        
        /* Prevent CSE and constant propagation */
        VOLATILE_VAR(i);
    }
}

/* Function using vector builtins with exactly 10 arguments */
NOOPT TARGET_ATTR
static __m256i complex_shuffle_10_args(__m256i a, __m256i b, __m256i c, 
                                      __m256i d, __m256i e, __m256i f,
                                      __m256i g, __m256i h, __m256i i,
                                      int mask) {
    /* Create complex shuffle pattern */
    __m256i result;
    
    /* Inline asm with exactly 10 arguments */
    asm volatile (
        "vpunpcklbw %1, %2, %0\n\t"
        "vpunpckhbw %3, %4, %10\n\t"
        "vpaddb %5, %0, %0\n\t"
        "vpsubb %6, %10, %10\n\t"
        "vpaddb %7, %0, %0\n\t"
        "vpsubb %8, %10, %10\n\t"
        "vpblendvb %9, %0, %10, %0"
        : "=x"(result), "+x"(a)
        : "x"(b), "x"(c), "x"(d), "x"(e), "x"(f), "x"(g), "x"(h), "i"(mask)
        : "memory"
    );
    
    return result;
}

/* Function using exactly 11 arguments */
NOOPT TARGET_ATTR
static __m256 complex_math_11_args(__m256 a, __m256 b, __m256 c, __m256 d,
                                  __m256 e, __m256 f, __m256 g, __m256 h,
                                  __m256 i, __m256 j, float k) {
    __m256 result;
    
    /* Extended asm with 11 input operands */
    asm volatile (
        "vaddps %2, %1, %0\n\t"
        "vmulps %3, %0, %0\n\t"
        "vsubps %4, %0, %0\n\t"
        "vdivps %5, %0, %0\n\t"
        "vaddps %6, %0, %0\n\t"
        "vsubps %7, %0, %0\n\t"
        "vmulps %8, %0, %0\n\t"
        "vaddps %9, %0, %0\n\t"
        "vsubps %10, %0, %0\n\t"
        "vmulps %11, %0, %0"
        : "=x"(result)
        : "0"(a), "x"(b), "x"(c), "x"(d), "x"(e),
          "x"(f), "x"(g), "x"(h), "x"(i), "x"(j),
          "x"(_mm256_set1_ps(k))
        : "memory"
    );
    
    return result;
}

int main(void) {
    const int N = 1024;
    float *in1 = (float*)aligned_alloc(32, N * sizeof(float));
    float *in2 = (float*)aligned_alloc(32, N * sizeof(float));
    float *in3 = (float*)aligned_alloc(32, N * sizeof(float));
    float *in4 = (float*)aligned_alloc(32, N * sizeof(float));
    float *out = (float*)aligned_alloc(32, N * sizeof(float));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        in1[i] = (float)(prng_next() % 1000) / 100.0f;
        in2[i] = (float)(prng_next() % 1000) / 100.0f;
        in3[i] = (float)(prng_next() % 1000) / 100.0f;
        in4[i] = (float)(prng_next() % 1000) / 100.0f;
        out[i] = 0.0f;
    }
    
    /* Test the many-argument functions */
    test_many_args(out, in1, in2, in3, in4, N);
    
    /* Also test the exact 10 and 11 argument functions */
    if (N >= 32) {
        __m256i vi1 = _mm256_set1_epi32(1);
        __m256i vi2 = _mm256_set1_epi32(2);
        __m256i vi3 = _mm256_set1_epi32(3);
        __m256i vi4 = _mm256_set1_epi32(4);
        __m256i vi5 = _mm256_set1_epi32(5);
        __m256i vi6 = _mm256_set1_epi32(6);
        __m256i vi7 = _mm256_set1_epi32(7);
        __m256i vi8 = _mm256_set1_epi32(8);
        __m256i vi9 = _mm256_set1_epi32(9);
        
        __m256i shuffle_result = complex_shuffle_10_args(
            vi1, vi2, vi3, vi4, vi5, vi6, vi7, vi8, vi9, 0xFF
        );
        
        __m256 vf1 = _mm256_set1_ps(1.0f);
        __m256 vf2 = _mm256_set1_ps(2.0f);
        __m256 vf3 = _mm256_set1_ps(3.0f);
        __m256 vf4 = _mm256_set1_ps(4.0f);
        __m256 vf5 = _mm256_set1_ps(5.0f);
        __m256 vf6 = _mm256_set1_ps(6.0f);
        __m256 vf7 = _mm256_set1_ps(7.0f);
        __m256 vf8 = _mm256_set1_ps(8.0f);
        __m256 vf9 = _mm256_set1_ps(9.0f);
        __m256 vf10 = _mm256_set1_ps(10.0f);
        
        __m256 math_result = complex_math_11_args(
            vf1, vf2, vf3, vf4, vf5, vf6, vf7, vf8, vf9, vf10, 11.0f
        );
        
        /* Store results to prevent optimization */
        _mm256_storeu_ps(&out[0], _mm256_castsi256_ps(shuffle_result));
        _mm256_storeu_ps(&out[8], math_result);
    }
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        checksum += (double)out[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    free(in1);
    free(in2);
    free(in3);
    free(in4);
    free(out);
    
    return 0;
}
