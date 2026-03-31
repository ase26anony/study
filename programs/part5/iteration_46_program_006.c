#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Simple PRNG for reproducible results */
static uint32_t seed = 123456789;
static inline uint32_t prng() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Vector types for different architectures */
#ifdef __AVX512F__
#include <immintrin.h>
typedef __m512i v512i;
typedef __m512 v512f;
typedef __mmask16 vmask16;
#define VECTOR_SIZE 16
#elif defined(__AVX2__)
#include <immintrin.h>
typedef __m256i v256i;
typedef __m256 v256f;
#define VECTOR_SIZE 8
#elif defined(__ARM_NEON)
#include <arm_neon.h>
typedef int32x4_t v128i;
typedef float32x4_t v128f;
#define VECTOR_SIZE 4
#else
/* Fallback to generic types */
typedef int32_t v128i __attribute__((vector_size(16)));
typedef float v128f __attribute__((vector_size(16)));
#define VECTOR_SIZE 4
#endif

/* Prevent optimization of arguments */
#define NOOPT __attribute__((noinline, noclone))
#define VOLATILE_VAR(var) asm volatile("" : "+r"(var))

/* Function with many arguments to trigger optab expansion */
#ifdef __AVX512F__
__attribute__((target("avx512f,avx512vl"))) NOOPT
static v512i complex_shuffle_10args(v512i a, v512i b, v512i c, v512i d,
                                    v512i e, v512i f, v512i g, v512i h,
                                    int imm1, int imm2) {
    /* This should trigger 10-argument optab expansion */
    v512i result;
    
    /* Complex inline asm with 10 operands */
    asm volatile (
        "vpternlogd %[imm2], %[a], %[b], %[c]\n\t"
        "vpternlogd %[imm1], %[d], %[e], %[f]\n\t"
        "vpblendmd %[g]%{%%k%[imm1]%}, %[h], %[res]"
        : [res] "=v" (result)
        : [a] "v" (a), [b] "v" (b), [c] "v" (c),
          [d] "v" (d), [e] "v" (e), [f] "v" (f),
          [g] "v" (g), [h] "v" (h),
          [imm1] "r" (imm1), [imm2] "r" (imm2)
        : "memory"
    );
    
    return result;
}

__attribute__((target("avx512f"))) NOOPT
static void test_many_args_avx512(int32_t* restrict out,
                                  const int32_t* restrict in1,
                                  const int32_t* restrict in2,
                                  int n) {
    volatile int i = 0;  /* Prevent loop unrolling */
    
    for (; i < n; VOLATILE_VAR(i)) {
        /* Load 8 vectors - will be used as arguments */
        v512i v0 = _mm512_loadu_si512((const __m512i*)(in1 + i * VECTOR_SIZE));
        v512i v1 = _mm512_loadu_si512((const __m512i*)(in1 + (i + 1) * VECTOR_SIZE));
        v512i v2 = _mm512_loadu_si512((const __m512i*)(in2 + i * VECTOR_SIZE));
        v512i v3 = _mm512_loadu_si512((const __m512i*)(in2 + (i + 1) * VECTOR_SIZE));
        v512i v4 = _mm512_loadu_si512((const __m512i*)(in1 + (i + 2) * VECTOR_SIZE));
        v512i v5 = _mm512_loadu_si512((const __m512i*)(in2 + (i + 2) * VECTOR_SIZE));
        v512i v6 = _mm512_loadu_si512((const __m512i*)(in1 + (i + 3) * VECTOR_SIZE));
        v512i v7 = _mm512_loadu_si512((const __m512i*)(in2 + (i + 3) * VECTOR_SIZE));
        
        /* Create complex expression with many temporaries */
        int imm1 = (i * 7) & 0xFF;
        int imm2 = (i * 13) & 0xFF;
        
        /* This call should trigger 10-argument optab expansion */
        v512i result = complex_shuffle_10args(v0, v1, v2, v3, v4, v5, v6, v7,
                                             imm1, imm2);
        
        /* Additional complex operation with builtin that may use many args */
        v512i shuffled = _mm512_permutex2var_epi32(v0, v1, v2);
        
        /* Blend with another operation */
        __mmask16 mask = _mm512_cmp_epi32_mask(result, shuffled, _MM_CMPINT_EQ);
        v512i final = _mm512_mask_blend_epi32(mask, result, shuffled);
        
        _mm512_storeu_si512((__m512i*)(out + i * VECTOR_SIZE), final);
        
        i += 4;  /* Process 4 vectors per iteration */
    }
}
#endif

/* Generic version for other architectures */
NOOPT
static void test_many_args_generic(int32_t* restrict out,
                                   const int32_t* restrict in1,
                                   const int32_t* restrict in2,
                                   int n) {
    volatile int i = 0;
    
    for (; i < n; VOLATILE_VAR(i)) {
        /* Create complex expression with many intermediate values */
        /* This forces the expander to handle many operands */
        
        /* Load multiple values */
        int32_t a0 = in1[i];
        int32_t a1 = in1[i + 1];
        int32_t a2 = in1[i + 2];
        int32_t a3 = in1[i + 3];
        int32_t a4 = in1[i + 4];
        int32_t a5 = in1[i + 5];
        int32_t a6 = in1[i + 6];
        int32_t a7 = in1[i + 7];
        int32_t b0 = in2[i];
        int32_t b1 = in2[i + 1];
        
        /* Complex expression with many operations */
        /* This may be expanded into a single optab with many args */
        int32_t result = ((a0 * b0) >> (a1 & 0x1F)) +
                        ((a2 * b1) >> (a3 & 0x1F)) +
                        ((a4 * a5) ^ (a6 * a7)) +
                        ((a0 ^ a2) | (a1 ^ a3)) +
                        ((a4 & a5) + (a6 & a7));
        
        /* Inline asm with 11 arguments */
        asm volatile (
            "imul %[a0], %[a1]\n\t"
            "imul %[a2], %[a3]\n\t"
            "add %[a4], %[a5]\n\t"
            "xor %[a6], %[a7]\n\t"
            "or %[b0], %[b1]\n\t"
            "add %[imm1], %[res]"
            : [res] "=r" (result)
            : [a0] "r" (a0), [a1] "r" (a1), [a2] "r" (a2),
              [a3] "r" (a3), [a4] "r" (a4), [a5] "r" (a5),
              [a6] "r" (a6), [a7] "r" (a7), [b0] "r" (b0),
              [b1] "r" (b1), [imm1] "i" (12345)
            : "cc", "memory"
        );
        
        out[i] = result;
        
        i += 8;
    }
}

/* Vector builtin with many arguments - may trigger optab expansion */
NOOPT static v128i vector_convert_11args(v128f a, v128f b, v128f c, v128f d,
                                        v128f e, v128f f, v128f g, v128f h,
                                        int imm1, int imm2, int imm3) {
    /* Complex conversion chain */
    v128i result;
    
    /* Use __builtin_convertvector with multiple sources */
    v128f temp1 = __builtin_shufflevector(a, b, 0, 2, 4, 6);
    v128f temp2 = __builtin_shufflevector(c, d, 1, 3, 5, 7);
    v128f temp3 = __builtin_shufflevector(e, f, imm1, imm2, imm3, 0);
    v128f temp4 = __builtin_shufflevector(g, h, 1, 3, 2, 0);
    
    /* Combine all - this complex expression may require many-arg optab */
    v128f combined = temp1 + temp2 * temp3 - temp4;
    
    /* Final conversion - could be expanded as 11-argument optab */
    result = __builtin_convertvector(combined, v128i);
    
    return result;
}

int main() {
    const int N = 1024;
    const int TOTAL_SIZE = N * VECTOR_SIZE;
    
    /* Allocate aligned memory for better vectorization */
    int32_t* in1 = aligned_alloc(64, TOTAL_SIZE * sizeof(int32_t));
    int32_t* in2 = aligned_alloc(64, TOTAL_SIZE * sizeof(int32_t));
    int32_t* out = aligned_alloc(64, TOTAL_SIZE * sizeof(int32_t));
    
    if (!in1 || !in2 || !out) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < TOTAL_SIZE; i++) {
        in1[i] = (int32_t)prng();
        in2[i] = (int32_t)prng();
        out[i] = 0;
    }
    
    /* Call the many-argument test function */
#ifdef __AVX512F__
    test_many_args_avx512(out, in1, in2, N);
#else
    test_many_args_generic(out, in1, in2, N);
#endif
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (int i = 0; i < TOTAL_SIZE; i++) {
        checksum += (uint64_t)out[i];
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    /* Cleanup */
    free(in1);
    free(in2);
    free(out);
    
    return 0;
}
