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
typedef __m512  v512f;
typedef __mmask16 mask16;
#elif defined(__AVX2__)
#include <immintrin.h>
typedef __m256i v256i;
typedef __m256  v256f;
#elif defined(__ARM_NEON)
#include <arm_neon.h>
typedef int32x4_t v128i;
typedef float32x4_t v128f;
#else
/* Fallback scalar types */
typedef struct { int32_t v[4]; } v128i;
typedef struct { float v[4]; } v128f;
#endif

/* Prevent optimization of critical variables */
#define NOOPT __attribute__((optimize("O0")))
#define NOINLINE __attribute__((noinline))
#define TARGET_AVX2 __attribute__((target("avx2,avx512f")))

/* Complex expression with many temporaries */
NOINLINE TARGET_AVX2
static void test_many_args(int32_t* output, const int32_t* input1, 
                          const int32_t* input2, const int32_t* input3,
                          int count) {
    volatile int i = 0;  /* Prevent loop unrolling */
    
    for (; i < count; i += 16) {
        /* Load multiple vectors - creates many temporaries */
#ifdef __AVX512F__
        __m512i v0 = _mm512_loadu_si512((const __m512i*)(input1 + i));
        __m512i v1 = _mm512_loadu_si512((const __m512i*)(input2 + i));
        __m512i v2 = _mm512_loadu_si512((const __m512i*)(input3 + i));
        __m512i v3 = _mm512_loadu_si512((const __m512i*)(input1 + i + 16));
        __m512i v4 = _mm512_loadu_si512((const __m512i*)(input2 + i + 16));
        __m512i v5 = _mm512_loadu_si512((const __m512i*)(input3 + i + 16));
        
        /* Complex shuffle with many arguments - targeting 10-11 args */
        /* This creates a pattern that might use optabs with many operands */
        __m512i shuffled;
        
        /* Method 1: Use inline asm with 11 operands */
        asm volatile (
            "vpblendmd %[out], %[v0], %[v1], %[k0]\n\t"
            "vpaddd %[out], %[out], %[v2]\n\t"
            "vpslld %[out], %[out], %[shift]\n\t"
            "vpsrld %[out], %[out], %[shift2]\n\t"
            "vpord %[out], %[out], %[v3]\n\t"
            "vpandd %[out], %[out], %[mask]\n\t"
            : [out] "=v" (shuffled)
            : [v0] "v" (v0), [v1] "v" (v1), [v2] "v" (v2),
              [v3] "v" (v3), [k0] "Yk" ((__mmask16)0xAAAA),
              [shift] "i" (2), [shift2] "i" (1),
              [mask] "m" (*((const __m512i*)input1))
            : "memory"
        );
        
        /* Method 2: Chain operations to create complex expression tree */
        __m512i temp1 = _mm512_add_epi32(v0, v1);
        __m512i temp2 = _mm512_sub_epi32(v2, v3);
        __m512i temp3 = _mm512_mullo_epi32(v4, v5);
        
        /* Complex expression with many intermediate values */
        __m512i result = _mm512_add_epi32(
            _mm512_sub_epi32(
                _mm512_and_si512(temp1, temp2),
                _mm512_xor_si512(temp3, v0)
            ),
            _mm512_or_si512(
                _mm512_slli_epi32(v1, 3),
                _mm512_srli_epi32(v2, 2)
            )
        );
        
        /* Blend with mask - potentially uses many-argument optab */
        __mmask16 blend_mask = 0xCCCC;
        result = _mm512_mask_blend_epi32(blend_mask, result, shuffled);
        
        _mm512_storeu_si512((__m512i*)(output + i), result);
        
#elif defined(__AVX2__)
        /* AVX2 implementation with 256-bit vectors */
        __m256i v0 = _mm256_loadu_si256((const __m256i*)(input1 + i));
        __m256i v1 = _mm256_loadu_si256((const __m256i*)(input2 + i));
        __m256i v2 = _mm256_loadu_si256((const __m256i*)(input3 + i));
        __m256i v3 = _mm256_loadu_si256((const __m256i*)(input1 + i + 8));
        __m256i v4 = _mm256_loadu_si256((const __m256i*)(input2 + i + 8));
        
        /* Extended inline asm with many operands (10 operands) */
        __m256i result;
        asm volatile (
            "vpaddd %[res], %[a], %[b]\n\t"
            "vpsubd %[res], %[res], %[c]\n\t"
            "vpmulld %[res], %[res], %[d]\n\t"
            "vpslld %[res], %[res], %[s1]\n\t"
            "vpsrld %[res], %[res], %[s2]\n\t"
            : [res] "=x" (result)
            : [a] "x" (v0), [b] "x" (v1), [c] "x" (v2),
              [d] "x" (v3), [s1] "i" (3), [s2] "i" (1),
              "m" (*input1), "m" (*input2), "m" (*input3)
            : "memory"
        );
        
        _mm256_storeu_si256((__m256i*)(output + i), result);
        
#else
        /* Scalar fallback with complex expression tree */
        for (int j = 0; j < 16 && (i + j) < count; j++) {
            /* Complex expression with many operations */
            int32_t a = input1[i + j];
            int32_t b = input2[i + j];
            int32_t c = input3[i + j];
            int32_t d = input1[(i + j + 1) % count];
            int32_t e = input2[(i + j + 2) % count];
            
            /* Expression with many temporaries */
            int32_t t1 = a + b;
            int32_t t2 = c - d;
            int32_t t3 = e * a;
            int32_t t4 = t1 & t2;
            int32_t t5 = t3 ^ b;
            int32_t t6 = t4 | t5;
            int32_t t7 = t6 << 3;
            int32_t t8 = t7 >> 1;
            int32_t t9 = t8 + c;
            int32_t t10 = t9 - d;
            
            output[i + j] = t10;
        }
#endif
    }
}

/* Another function specifically targeting 11-argument builtins */
NOINLINE TARGET_AVX2
static void test_11_args(float* output, const float* input, int count) {
#ifdef __AVX512F__
    /* Create a complex permutation pattern */
    for (int i = 0; i < count; i += 16) {
        __m512 v0 = _mm512_loadu_ps(input + i);
        __m512 v1 = _mm512_loadu_ps(input + i + 16);
        __m512 v2 = _mm512_loadu_ps(input + i + 32);
        
        /* Complex blending with multiple masks and sources */
        __m512 result;
        
        /* Extended asm with 11 memory references and immediates */
        asm volatile (
            "vblendmps %[out], %[v0], %[v1], %[k0]\n\t"
            "vfmadd213ps %[out], %[v2], %[mem1]\n\t"
            "vxorps %[out], %[out], %[mem2]\n\t"
            : [out] "=v" (result)
            : [v0] "v" (v0), [v1] "v" (v1), [v2] "v" (v2),
              [k0] "Yk" ((__mmask16)0xF0F0),
              [mem1] "m" (*(const __m512*)(input)),
              [mem2] "m" (*(const __m512*)(input + 16)),
              "m" (*(const __m512*)(input + 32)),
              "m" (*(const __m512*)(input + 48)),
              "i" (1), "i" (2), "i" (3)
            : "memory"
        );
        
        _mm512_storeu_ps(output + i, result);
    }
#endif
}

/* Function using GCC vector builtins with many arguments */
NOINLINE
static void test_vector_builtins(int32_t* output, const int32_t* input, int count) {
    typedef int32_t v4si __attribute__((vector_size(16)));
    
    for (int i = 0; i < count; i += 4) {
        /* Load vectors */
        v4si v0 = *(const v4si*)(input + i);
        v4si v1 = *(const v4si*)(input + i + 4);
        v4si v2 = *(const v4si*)(input + i + 8);
        v4si v3 = *(const v4si*)(input + i + 12);
        
        /* Complex expression with type conversions and shuffles */
        /* This might trigger optabs with many arguments */
        v4si temp = __builtin_shuffle(v0, v1, 
            (v4si){0, 4, 1, 5});  /* 4 args so far */
        
        /* Add more operations to increase argument count */
        temp = temp + __builtin_shuffle(v2, v3,
            (v4si){2, 6, 3, 7});  /* Another 4 args */
        
        /* Convert and shuffle with immediate indices */
        typedef float v4sf __attribute__((vector_size(16)));
        v4sf f0 = __builtin_convertvector(v0, v4sf);
        v4sf f1 = __builtin_convertvector(v1, v4sf);
        
        /* Complex expression tree */
        v4sf ftemp = f0 * f1 + __builtin_shuffle(f0, f1,
            (v4si){1, 0, 3, 2});
        
        /* Convert back and blend */
        v4si itemp = __builtin_convertvector(ftemp, v4si);
        v4si result = __builtin_shuffle(temp, itemp,
            (v4si){0, 4, 2, 6});
        
        *(v4si*)(output + i) = result;
    }
}

int main() {
    const int SIZE = 1024;
    int32_t* input1 = (int32_t*)aligned_alloc(64, SIZE * sizeof(int32_t));
    int32_t* input2 = (int32_t*)aligned_alloc(64, SIZE * sizeof(int32_t));
    int32_t* input3 = (int32_t*)aligned_alloc(64, SIZE * sizeof(int32_t));
    int32_t* output = (int32_t*)aligned_alloc(64, SIZE * sizeof(int32_t));
    float*   foutput = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float*   finput = (float*)aligned_alloc(64, SIZE * sizeof(float));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        input1[i] = (int32_t)prng_next();
        input2[i] = (int32_t)prng_next();
        input3[i] = (int32_t)prng_next();
        finput[i] = (float)prng_next() / (float)UINT32_MAX;
        output[i] = 0;
        foutput[i] = 0.0f;
    }
    
    /* Test different patterns */
    test_many_args(output, input1, input2, input3, SIZE);
    
#ifdef __AVX512F__
    test_11_args(foutput, finput, SIZE);
#endif
    
    test_vector_builtins(output, input1, SIZE);
    
    /* Compute checksum */
    int64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += output[i];
        checksum += (int64_t)foutput[i];
    }
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    /* Cleanup */
    free(input1);
    free(input2);
    free(input3);
    free(output);
    free(foutput);
    free(finput);
    
    return 0;
}
