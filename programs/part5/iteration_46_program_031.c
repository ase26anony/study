#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

/* Simple PRNG for reproducible results */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Inhibit optimization helpers */
#define NO_INLINE __attribute__((noinline))
#define NO_OPTIMIZE __attribute__((optimize("O0")))
#define VOLATILE_VAR(var) asm volatile("" : "+r"(var))

/* Target-specific attributes */
#ifdef __AVX512F__
#define TARGET_ATTR __attribute__((target("avx512f,avx512bw,avx512vl")))
#elif defined(__AVX2__)
#define TARGET_ATTR __attribute__((target("avx2")))
#else
#define TARGET_ATTR
#endif

/* Complex expression with many temporaries */
NO_INLINE TARGET_ATTR
static void test_many_args(float* restrict out, 
                          const float* restrict in1,
                          const float* restrict in2,
                          const float* restrict in3,
                          const float* restrict in4,
                          int n) {
    volatile int i = 0;  /* Prevent loop unrolling */
    
    for (i = 0; i < n; i += 8) {
        /* Load multiple vectors - creates many temporaries */
        __m256 v1 = _mm256_loadu_ps(&in1[i]);
        __m256 v2 = _mm256_loadu_ps(&in2[i]);
        __m256 v3 = _mm256_loadu_ps(&in3[i]);
        __m256 v4 = _mm256_loadu_ps(&in4[i]);
        
        /* Complex multi-step expression with many intermediates */
        __m256 t1 = _mm256_add_ps(v1, v2);
        __m256 t2 = _mm256_sub_ps(v3, v4);
        __m256 t3 = _mm256_mul_ps(t1, t2);
        __m256 t4 = _mm256_div_ps(v1, _mm256_set1_ps(2.0f));
        __m256 t5 = _mm256_fmadd_ps(t3, t4, v2);
        
        /* Create a complex shuffle/blend operation with many arguments */
        /* This should trigger the 10-11 argument optab expansion */
#ifdef __AVX512F__
        /* AVX-512 provides more opportunities for many-argument operations */
        __m512 v1_512 = _mm512_castps256_ps512(v1);
        __m512 v2_512 = _mm512_castps256_ps512(v2);
        __m512 v3_512 = _mm512_castps256_ps512(v3);
        __m512 v4_512 = _mm512_castps256_ps512(v4);
        
        /* Complex blend with multiple masks and sources */
        __mmask16 mask1 = 0xAAAA;  /* 1010101010101010 */
        __mmask16 mask2 = 0x5555;  /* 0101010101010101 */
        __mmask16 mask3 = 0xCCCC;  /* 1100110011001100 */
        
        /* Extended inline asm with 11 operands */
        __m512 result;
        asm volatile (
            "vmovaps %[v1], %%zmm0\n\t"
            "vmovaps %[v2], %%zmm1\n\t"
            "vmovaps %[v3], %%zmm2\n\t"
            "vmovaps %[v4], %%zmm3\n\t"
            "kxnorw %k[mask1], %k[mask1], %k4\n\t"
            "kxnorw %k[mask2], %k[mask2], %k5\n\t"
            "kxnorw %k[mask3], %k[mask3], %k6\n\t"
            "vblendmps %%zmm0, %%zmm1, %%zmm7 %{%%k4%}\n\t"
            "vblendmps %%zmm2, %%zmm3, %%zmm8 %{%%k5%}\n\t"
            "vblendmps %%zmm7, %%zmm8, %[res] %{%%k6%}\n\t"
            : [res] "=v" (result)
            : [v1] "v" (v1_512),
              [v2] "v" (v2_512),
              [v3] "v" (v3_512),
              [v4] "v" (v4_512),
              [mask1] "k" (mask1),
              [mask2] "k" (mask2),
              [mask3] "k" (mask3),
              "m" (*in1),  /* Memory constraint to add more operands */
              "m" (*in2),
              "m" (*in3),
              "m" (*in4)
            : "zmm0", "zmm1", "zmm2", "zmm3", "zmm7", "zmm8",
              "k4", "k5", "k6", "memory"
        );
        
        /* Store result */
        _mm256_storeu_ps(&out[i], _mm512_castps512_ps256(result));
#else
        /* AVX2 version with complex shuffle */
        /* Create a shuffle with many immediate arguments */
        __m256 shuffled;
        
        /* Inline asm with 10 operands for AVX2 */
        asm volatile (
            "vmovaps %[v1], %%ymm0\n\t"
            "vmovaps %[v2], %%ymm1\n\t"
            "vmovaps %[v3], %%ymm2\n\t"
            "vmovaps %[v4], %%ymm3\n\t"
            "vperm2f128 $0x21, %%ymm0, %%ymm1, %%ymm4\n\t"
            "vperm2f128 $0x12, %%ymm2, %%ymm3, %%ymm5\n\t"
            "vblendps $0xAA, %%ymm4, %%ymm5, %[shuf]\n\t"
            : [shuf] "=v" (shuffled)
            : [v1] "v" (v1),
              [v2] "v" (v2),
              [v3] "v" (v3),
              [v4] "v" (v4),
              "m" (*in1),  /* Memory constraints add to operand count */
              "m" (*in2),
              "m" (*in3),
              "m" (*in4),
              "i" (0x21),  /* Immediate operands */
              "i" (0x12),
              "i" (0xAA)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "memory"
        );
        
        /* Final blend with t5 */
        __m256 result = _mm256_blend_ps(shuffled, t5, 0xCC);
        _mm256_storeu_ps(&out[i], result);
#endif
    }
    
    /* Force dependency chain */
    VOLATILE_VAR(i);
}

/* Alternative approach using GCC vector extensions */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

NO_INLINE TARGET_ATTR
static void test_vector_shuffle(float* restrict out,
                               const float* restrict in,
                               int n) {
    /* Complex shuffle operation using GCC builtins */
    for (int i = 0; i < n; i += 8) {
        v8sf v1 = *(const v8sf*)&in[i];
        v8sf v2 = *(const v8sf*)&in[i + 8];
        v8sf v3 = *(const v8sf*)&in[i + 16];
        v8sf v4 = *(const v8sf*)&in[i + 24];
        
        /* Create complex permutation with many arguments */
        /* __builtin_shufflevector can take many arguments */
        v8sf shuffled;
        
        /* Inhibit constant propagation */
        volatile int idx0 = 0, idx1 = 7, idx2 = 1, idx3 = 6;
        volatile int idx4 = 2, idx5 = 5, idx6 = 3, idx7 = 4;
        VOLATILE_VAR(idx0); VOLATILE_VAR(idx1); VOLATILE_VAR(idx2); VOLATILE_VAR(idx3);
        VOLATILE_VAR(idx4); VOLATILE_VAR(idx5); VOLATILE_VAR(idx6); VOLATILE_VAR(idx7);
        
        /* Complex expression that might expand to many-argument optab */
        shuffled = __builtin_shufflevector(v1, v2, v3, v4,
                                          idx0, idx1, idx2, idx3,
                                          idx4, idx5, idx6, idx7);
        
        /* Additional operations to create more temporaries */
        v8sf temp1 = shuffled + v1;
        v8sf temp2 = v2 * v3;
        v8sf temp3 = temp1 - temp2;
        v8sf temp4 = v4 / 2.0f;
        v8sf temp5 = temp3 * temp4;
        
        /* Final store */
        *(v8sf*)&out[i] = temp5;
    }
}

/* Test with integer types for different optab patterns */
NO_INLINE TARGET_ATTR
static void test_integer_ops(int32_t* restrict out,
                            const int32_t* restrict in1,
                            const int32_t* restrict in2,
                            const int32_t* restrict in3,
                            const int32_t* restrict in4,
                            int n) {
    for (int i = 0; i < n; i += 8) {
        __m256i iv1 = _mm256_loadu_si256((const __m256i*)&in1[i]);
        __m256i iv2 = _mm256_loadu_si256((const __m256i*)&in2[i]);
        __m256i iv3 = _mm256_loadu_si256((const __m256i*)&in3[i]);
        __m256i iv4 = _mm256_loadu_si256((const __m256i*)&in4[i]);
        
        /* Complex integer operation chain */
        __m256i t1 = _mm256_add_epi32(iv1, iv2);
        __m256i t2 = _mm256_sub_epi32(iv3, iv4);
        __m256i t3 = _mm256_mullo_epi32(t1, t2);
        __m256i t4 = _mm256_srai_epi32(iv1, 3);
        __m256i t5 = _mm256_slli_epi32(iv2, 2);
        
        /* Inline asm with many operands for integer operations */
        __m256i result;
        asm volatile (
            "vpaddd %[iv1], %[iv2], %%ymm0\n\t"
            "vpsubd %[iv3], %[iv4], %%ymm1\n\t"
            "vpmulld %%ymm0, %%ymm1, %%ymm2\n\t"
            "vpsrad $3, %[iv1], %%ymm3\n\t"
            "vpslld $2, %[iv2], %%ymm4\n\t"
            "vpblendd $0xAA, %%ymm2, %%ymm3, %%ymm5\n\t"
            "vpblendd $0x55, %%ymm4, %%ymm5, %[res]\n\t"
            : [res] "=v" (result)
            : [iv1] "v" (iv1),
              [iv2] "v" (iv2),
              [iv3] "v" (iv3),
              [iv4] "v" (iv4),
              "m" (*in1),
              "m" (*in2),
              "m" (*in3),
              "m" (*in4),
              "i" (3),    /* shift amounts */
              "i" (2),
              "i" (0xAA),
              "i" (0x55)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "memory"
        );
        
        _mm256_storeu_si256((__m256i*)&out[i], result);
    }
}

int main(void) {
    const int N = 1024;
    float* in1 = aligned_alloc(32, N * sizeof(float));
    float* in2 = aligned_alloc(32, N * sizeof(float));
    float* in3 = aligned_alloc(32, N * sizeof(float));
    float* in4 = aligned_alloc(32, N * sizeof(float));
    float* out = aligned_alloc(32, N * sizeof(float));
    
    int32_t* iin1 = aligned_alloc(32, N * sizeof(int32_t));
    int32_t* iin2 = aligned_alloc(32, N * sizeof(int32_t));
    int32_t* iin3 = aligned_alloc(32, N * sizeof(int32_t));
    int32_t* iin4 = aligned_alloc(32, N * sizeof(int32_t));
    int32_t* iout = aligned_alloc(32, N * sizeof(int32_t));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        in1[i] = (prng_next() % 1000) / 100.0f;
        in2[i] = (prng_next() % 1000) / 100.0f;
        in3[i] = (prng_next() % 1000) / 100.0f;
        in4[i] = (prng_next() % 1000) / 100.0f;
        
        iin1[i] = prng_next() % 1000;
        iin2[i] = prng_next() % 1000;
        iin3[i] = prng_next() % 1000;
        iin4[i] = prng_next() % 1000;
    }
    
    /* Call the test functions */
    test_many_args(out, in1, in2, in3, in4, N);
    test_vector_shuffle(out, in1, N);
    test_integer_ops(iout, iin1, iin2, iin3, iin4, N);
    
    /* Compute checksum */
    float checksum_f = 0.0f;
    int32_t checksum_i = 0;
    for (int i = 0; i < N; i++) {
        checksum_f += out[i];
        checksum_i += iout[i];
    }
    
    printf("Float checksum: %f\n", checksum_f);
    printf("Integer checksum: %d\n", checksum_i);
    
    /* Cleanup */
    free(in1); free(in2); free(in3); free(in4); free(out);
    free(iin1); free(iin2); free(iin3); free(iin4); free(iout);
    
    return 0;
}
