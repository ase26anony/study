#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Simple PRNG for deterministic testing */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Vector types for different architectures */
#ifdef __AVX512F__
#include <immintrin.h>
typedef __m512i v512i;
typedef __m512 v512f;
typedef __mmask16 mask16;
#elif defined(__AVX2__)
#include <immintrin.h>
typedef __m256i v256i;
typedef __m256 v256f;
#elif defined(__ARM_NEON)
#include <arm_neon.h>
typedef int32x4_t v128i;
typedef float32x4_t v128f;
#else
/* Fallback to generic types */
typedef int32_t v128i __attribute__((vector_size(16)));
typedef float v128f __attribute__((vector_size(16)));
#endif

/* Prevent optimization of variables */
#define NOOPT __attribute__((optimize("O0")))
#define NOINLINE __attribute__((noinline))
#define TARGET_AVX2 __attribute__((target("avx2")))
#define TARGET_AVX512F __attribute__((target("avx512f")))

/* Complex expression with many temporaries */
NOINLINE TARGET_AVX512F
static v512i complex_shuffle_10_args(v512i a, v512i b, v512i c, v512i d,
                                     v512i e, v512i f, v512i g, v512i h,
                                     int imm1, int imm2) {
    /* This should trigger the 10-argument case */
    v512i result;
    
    /* Use inline asm with 10 operands to force optab expansion */
    __asm__ volatile (
        "vpternlogd %[imm2], %[a], %[b], %[c]\n\t"
        "vpternlogd %[imm1], %[d], %[e], %[f]\n\t"
        "vpblendmd %[g]%{%%k%[mask]%}, %[h], %[out]"
        : [out] "=v" (result)
        : [a] "v" (a), [b] "v" (b), [c] "v" (c),
          [d] "v" (d), [e] "v" (e), [f] "v" (f),
          [g] "v" (g), [h] "v" (h),
          [imm1] "i" (imm1), [imm2] "i" (imm2),
          [mask] "i" (0xFF)
        : "memory"
    );
    
    return result;
}

NOINLINE TARGET_AVX512F
static v512i complex_shuffle_11_args(v512i a, v512i b, v512i c, v512i d,
                                     v512i e, v512i f, v512i g, v512i h,
                                     v512i i, int imm1, int imm2) {
    /* This should trigger the 11-argument case */
    v512i result;
    
    /* Complex expression with many temporaries */
    v512i t1 = _mm512_add_epi32(a, b);
    v512i t2 = _mm512_sub_epi32(c, d);
    v512i t3 = _mm512_mullo_epi32(e, f);
    v512i t4 = _mm512_and_si512(g, h);
    v512i t5 = _mm512_or_si512(i, t1);
    
    /* Use builtin with many arguments - this may expand to optab with 11 args */
    result = _mm512_ternarylogic_epi32(t1, t2, t3, 0x96);  /* 6 args so far */
    
    /* Chain more operations to reach 11 arguments in the expansion */
    result = _mm512_mask_blend_epi32(0xAAAA, result, t4);  /* Adds more args */
    result = _mm512_permutexvar_epi32(_mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0), 
                                      result);  /* Adds permutation control */
    
    /* Final complex operation that might be expanded as 11-arg optab */
    __asm__ volatile (
        "vpdpbusd %[t5], %[result], %[imm1]\n\t"
        "vpermt2d %[t4], %[result], %[out]"
        : [out] "=v" (result)
        : [result] "v" (result), [t4] "v" (t4), [t5] "v" (t5),
          [imm1] "i" (imm1), [imm2] "i" (imm2)
        : "memory"
    );
    
    return result;
}

/* Function with many vector arguments in a hot loop */
NOINLINE TARGET_AVX512F
static void test_many_args(int8_t* output, const int8_t* input1, const int8_t* input2,
                          const int8_t* input3, const int8_t* input4, int n) {
    volatile int counter = n;  /* Prevent loop unrolling */
    
    for (int idx = 0; idx < counter; idx += 64) {  /* Process 64 bytes at a time */
        /* Load multiple vectors */
        v512i vec1 = _mm512_loadu_si512((const v512i*)(input1 + idx));
        v512i vec2 = _mm512_loadu_si512((const v512i*)(input2 + idx));
        v512i vec3 = _mm512_loadu_si512((const v512i*)(input3 + idx));
        v512i vec4 = _mm512_loadu_si512((const v512i*)(input4 + idx));
        
        /* Create more vectors through operations */
        v512i vec5 = _mm512_add_epi8(vec1, vec2);
        v512i vec6 = _mm512_sub_epi8(vec3, vec4);
        v512i vec7 = _mm512_avg_epu8(vec1, vec3);
        v512i vec8 = _mm512_avg_epu8(vec2, vec4);
        
        /* Complex shuffle with 10 arguments */
        v512i result1 = complex_shuffle_10_args(vec1, vec2, vec3, vec4,
                                               vec5, vec6, vec7, vec8,
                                               idx & 0xFF, (idx >> 8) & 0xFF);
        
        /* Another vector for 11-argument case */
        v512i vec9 = _mm512_maddubs_epi16(vec1, vec2);
        v512i vec10 = _mm512_maddubs_epi16(vec3, vec4);
        
        /* Complex shuffle with 11 arguments */
        v512i result2 = complex_shuffle_11_args(vec1, vec2, vec3, vec4,
                                               vec5, vec6, vec7, vec8,
                                               vec9, idx & 0x1F, (idx >> 5) & 0x1F);
        
        /* Blend results */
        v512i final_result = _mm512_mask_blend_epi32(0xAAAA, result1, result2);
        
        /* Store result */
        _mm512_storeu_si512((v512i*)(output + idx), final_result);
    }
}

/* Alternative implementation for AVX2 */
NOINLINE TARGET_AVX2
static void test_many_args_avx2(int8_t* output, const int8_t* input1, const int8_t* input2,
                               const int8_t* input3, const int8_t* input4, int n) {
    volatile int counter = n;
    
    for (int idx = 0; idx < counter; idx += 32) {
        /* Load vectors */
        v256i vec1 = _mm256_loadu_si256((const v256i*)(input1 + idx));
        v256i vec2 = _mm256_loadu_si256((const v256i*)(input2 + idx));
        v256i vec3 = _mm256_loadu_si256((const v256i*)(input3 + idx));
        v256i vec4 = _mm256_loadu_si256((const v256i*)(input4 + idx));
        
        /* Create complex expression with many temporaries */
        v256i t1 = _mm256_add_epi8(vec1, vec2);
        v256i t2 = _mm256_sub_epi8(vec3, vec4);
        v256i t3 = _mm256_avg_epu8(vec1, vec3);
        v256i t4 = _mm256_avg_epu8(vec2, vec4);
        v256i t5 = _mm256_maddubs_epi16(vec1, vec2);
        v256i t6 = _mm256_maddubs_epi16(vec3, vec4);
        v256i t7 = _mm256_maddubs_epi16(t1, t2);
        v256i t8 = _mm256_maddubs_epi16(t3, t4);
        
        /* Complex inline asm with many operands */
        v256i result;
        __asm__ volatile (
            "vpmaddubsw %[t5], %[t1], %[tmp1]\n\t"
            "vpmaddubsw %[t6], %[t2], %[tmp2]\n\t"
            "vpblendvb %[t3], %[tmp1], %[tmp2], %[result]\n\t"
            "vpermq $0x1B, %[result], %[result]"
            : [result] "=v" (result), [tmp1] "=&v" (t1), [tmp2] "=&v" (t2)
            : [t1] "v" (t1), [t2] "v" (t2), [t3] "v" (t3),
              [t4] "v" (t4), [t5] "v" (t5), [t6] "v" (t6),
              [t7] "v" (t7), [t8] "v" (t8)
            : "memory"
        );
        
        _mm256_storeu_si256((v256i*)(output + idx), result);
    }
}

/* Generic fallback */
NOINLINE
static void test_many_args_generic(int8_t* output, const int8_t* input1, const int8_t* input2,
                                  const int8_t* input3, const int8_t* input4, int n) {
    volatile int counter = n;
    
    for (int idx = 0; idx < counter; idx += 16) {
        /* Complex multi-statement expression with many temporaries */
        v128i vec1, vec2, vec3, vec4;
        memcpy(&vec1, input1 + idx, 16);
        memcpy(&vec2, input2 + idx, 16);
        memcpy(&vec3, input3 + idx, 16);
        memcpy(&vec4, input4 + idx, 16);
        
        /* Build complex expression tree */
        v128i t1 = vec1 + vec2;
        v128i t2 = vec3 - vec4;
        v128i t3 = vec1 & vec3;
        v128i t4 = vec2 | vec4;
        v128i t5 = t1 ^ t2;
        v128i t6 = t3 & t4;
        v128i t7 = t5 | t6;
        v128i t8 = t1 + t3;
        v128i t9 = t2 - t4;
        v128i t10 = t5 * t6;
        
        /* Use __builtin_shuffle with many arguments */
        v128i result = __builtin_shuffle(t1, t2, t3, t4, t5, t6, t7, t8, 
                                        (typeof(t1)){0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});
        
        memcpy(output + idx, &result, 16);
    }
}

int main(void) {
    const int SIZE = 1024;
    const int ITERATIONS = 1000;
    
    /* Allocate and initialize arrays with pseudo-random data */
    int8_t* input1 = (int8_t*)aligned_alloc(64, SIZE);
    int8_t* input2 = (int8_t*)aligned_alloc(64, SIZE);
    int8_t* input3 = (int8_t*)aligned_alloc(64, SIZE);
    int8_t* input4 = (int8_t*)aligned_alloc(64, SIZE);
    int8_t* output = (int8_t*)aligned_alloc(64, SIZE);
    
    for (int i = 0; i < SIZE; i++) {
        input1[i] = (int8_t)(prng_next() & 0xFF);
        input2[i] = (int8_t)(prng_next() & 0xFF);
        input3[i] = (int8_t)(prng_next() & 0xFF);
        input4[i] = (int8_t)(prng_next() & 0xFF);
    }
    
    /* Run the test multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
#ifdef __AVX512F__
        test_many_args(output, input1, input2, input3, input4, SIZE);
#elif defined(__AVX2__)
        test_many_args_avx2(output, input1, input2, input3, input4, SIZE);
#else
        test_many_args_generic(output, input1, input2, input3, input4, SIZE);
#endif
    }
    
    /* Compute checksum */
    uint32_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += (uint32_t)output[i];
    }
    
    printf("Checksum: %u\n", checksum);
    
    /* Cleanup */
    free(input1);
    free(input2);
    free(input3);
    free(input4);
    free(output);
    
    return 0;
}
