#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Simple PRNG for reproducible results */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Vector types for different architectures */
#ifdef __AVX512F__
typedef __m512i v512i;
typedef __m512 v512f;
typedef __mmask16 v512mask;
#elif defined(__AVX2__)
typedef __m256i v256i;
typedef __m256 v256f;
#elif defined(__SSE4_2__)
typedef __m128i v128i;
typedef __m128 v128f;
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
typedef int32x4_t v128i;
typedef float32x4_t v128f;
#endif

/* Function to inhibit optimization */
static inline void inhibit_opt(volatile int* var) {
    asm volatile("" : "+r"(*var));
}

/* Complex expression with many temporaries */
__attribute__((noinline, target("avx2")))
void test_many_args_avx2(float* restrict out, 
                         const float* restrict in1,
                         const float* restrict in2,
                         const float* restrict in3,
                         int n) {
    volatile int iter_counter = 0;  /* Prevent loop unrolling */
    
    for (int i = 0; i < n; i += 8) {
        inhibit_opt(&iter_counter);
        
        /* Load multiple vectors */
        __m256 v0 = _mm256_loadu_ps(&in1[i]);
        __m256 v1 = _mm256_loadu_ps(&in2[i]);
        __m256 v2 = _mm256_loadu_ps(&in3[i]);
        __m256 v3 = _mm256_set1_ps(1.5f);
        __m256 v4 = _mm256_set1_ps(2.5f);
        __m256 v5 = _mm256_set1_ps(3.5f);
        
        /* Create many intermediate values */
        __m256 t0 = _mm256_add_ps(v0, v1);
        __m256 t1 = _mm256_mul_ps(v2, v3);
        __m256 t2 = _mm256_sub_ps(t0, t1);
        __m256 t3 = _mm256_fmadd_ps(v4, v5, t2);
        
        /* Complex inline asm with 10-11 operands */
        /* This should trigger the 10-argument case in optabs.cc */
        asm volatile (
            "vmovaps %1, %%ymm0\n\t"
            "vmovaps %2, %%ymm1\n\t"
            "vmovaps %3, %%ymm2\n\t"
            "vmovaps %4, %%ymm3\n\t"
            "vmovaps %5, %%ymm4\n\t"
            "vmovaps %6, %%ymm5\n\t"
            "vaddps %%ymm0, %%ymm1, %%ymm6\n\t"
            "vmulps %%ymm2, %%ymm3, %%ymm7\n\t"
            "vsubps %%ymm6, %%ymm7, %%ymm0\n\t"
            "vfmadd132ps %%ymm4, %%ymm5, %%ymm0\n\t"
            "vmovaps %%ymm0, %0"
            : "=m"(out[i])
            : "m"(in1[i]), "m"(in2[i]), "m"(in3[i]), 
              "m"(t0), "m"(t1), "m"(t2), "m"(t3),
              "i"(8), "i"(16), "i"(24)  /* Immediate constants */
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", 
              "ymm5", "ymm6", "ymm7", "memory"
        );
        
        iter_counter++;
    }
}

/* Alternative version using GCC vector builtins */
#ifdef __AVX512F__
__attribute__((noinline, target("avx512f")))
void test_many_args_avx512(float* restrict out,
                          const float* restrict in1,
                          const float* restrict in2,
                          const float* restrict in3,
                          const float* restrict in4,
                          const float* restrict in5,
                          int n) {
    volatile int iter_counter = 0;
    
    for (int i = 0; i < n; i += 16) {
        inhibit_opt(&iter_counter);
        
        /* Load 5 different vectors */
        __m512 v0 = _mm512_loadu_ps(&in1[i]);
        __m512 v1 = _mm512_loadu_ps(&in2[i]);
        __m512 v2 = _mm512_loadu_ps(&in3[i]);
        __m512 v3 = _mm512_loadu_ps(&in4[i]);
        __m512 v4 = _mm512_loadu_ps(&in5[i]);
        
        /* Create mask with complex pattern */
        __mmask16 mask = _mm512_cmp_ps_mask(v0, v1, _CMP_GT_OQ);
        
        /* Complex expression with many arguments */
        /* This should trigger the 11-argument case in optabs.cc */
        __m512 result = _mm512_mask_blend_ps(
            mask,
            _mm512_fmadd_ps(v0, v1, v2),
            _mm512_fnmadd_ps(v3, v4, 
                _mm512_set1_ps(prng_next() & 0xFF)),
            _mm512_set1_ps(1.0f),  /* arg 5 */
            _mm512_set1_ps(2.0f),  /* arg 6 */
            _mm512_set1_ps(3.0f),  /* arg 7 */
            _mm512_set1_ps(4.0f),  /* arg 8 */
            _mm512_set1_ps(5.0f),  /* arg 9 */
            _mm512_set1_ps(6.0f)   /* arg 10 */
        );
        
        /* Store with inline asm having 11 operands */
        asm volatile (
            "vmovups %1, %%zmm0\n\t"
            "vmovups %2, %%zmm1\n\t"
            "vmovups %3, %%zmm2\n\t"
            "vmovups %4, %%zmm3\n\t"
            "vmovups %5, %%zmm4\n\t"
            "vmovups %6, %%zmm5\n\t"
            "vmovups %7, %%zmm6\n\t"
            "vmovups %8, %%zmm7\n\t"
            "vmovups %9, %%zmm8\n\t"
            "vmovups %10, %%zmm9\n\t"
            "vmovups %11, %%zmm10\n\t"
            "vaddps %%zmm0, %%zmm1, %%zmm11\n\t"
            "vmulps %%zmm2, %%zmm3, %%zmm12\n\t"
            "vsubps %%zmm11, %%zmm12, %%zmm0\n\t"
            "vfmadd132ps %%zmm4, %%zmm5, %%zmm0\n\t"
            "vfmadd231ps %%zmm6, %%zmm7, %%zmm0\n\t"
            "vfmadd231ps %%zmm8, %%zmm9, %%zmm0\n\t"
            "vfmadd231ps %%zmm10, %%zmm0, %%zmm0\n\t"
            "vmovups %%zmm0, %0"
            : "=m"(out[i])
            : "m"(in1[i]), "m"(in2[i]), "m"(in3[i]), 
              "m"(in4[i]), "m"(in5[i]), "m"(v0), "m"(v1),
              "m"(v2), "m"(v3), "m"(v4), "m"(result),
              "i"(16), "i"(32), "i"(48), "i"(64)
            : "zmm0", "zmm1", "zmm2", "zmm3", "zmm4",
              "zmm5", "zmm6", "zmm7", "zmm8", "zmm9",
              "zmm10", "zmm11", "zmm12", "memory"
        );
        
        iter_counter++;
    }
}
#endif

/* ARM NEON version */
#ifdef __ARM_NEON
__attribute__((noinline))
void test_many_args_neon(float* restrict out,
                        const float* restrict in1,
                        const float* restrict in2,
                        const float* restrict in3,
                        const float* restrict in4,
                        const float* restrict in5,
                        int n) {
    volatile int iter_counter = 0;
    
    for (int i = 0; i < n; i += 4) {
        inhibit_opt(&iter_counter);
        
        /* Load multiple vectors */
        float32x4_t v0 = vld1q_f32(&in1[i]);
        float32x4_t v1 = vld1q_f32(&in2[i]);
        float32x4_t v2 = vld1q_f32(&in3[i]);
        float32x4_t v3 = vld1q_f32(&in4[i]);
        float32x4_t v4 = vld1q_f32(&in5[i]);
        
        /* Complex shuffle/permute with many arguments */
        /* Using GCC vector builtins for complex operations */
        typedef float32x4_t v4sf __attribute__((vector_size(16)));
        
        /* Create a complex expression with many temporaries */
        v4sf t0 = v0 + v1;
        v4sf t1 = v2 * v3;
        v4sf t2 = t0 - t1;
        v4sf t3 = v4 * t2;
        v4sf t4 = __builtin_shuffle(t0, t1, 
            (v4sf){0, 5, 2, 7});  /* Complex shuffle */
        v4sf t5 = __builtin_shuffle(t2, t3, 
            (v4sf){4, 1, 6, 3});  /* Another shuffle */
        
        /* Inline asm with 10 operands */
        asm volatile (
            "vld1.32 {%q0}, [%1]\n\t"
            "vld1.32 {%q1}, [%2]\n\t"
            "vld1.32 {%q2}, [%3]\n\t"
            "vld1.32 {%q3}, [%4]\n\t"
            "vld1.32 {%q4}, [%5]\n\t"
            "vadd.f32 %q0, %q0, %q1\n\t"
            "vmul.f32 %q2, %q2, %q3\n\t"
            "vsub.f32 %q0, %q0, %q2\n\t"
            "vmla.f32 %q0, %q4, %q0\n\t"
            "vst1.32 {%q0}, [%6]"
            : 
            : "w"(v0), "r"(&in1[i]), "r"(&in2[i]), 
              "r"(&in3[i]), "r"(&in4[i]), "r"(&in5[i]),
              "r"(&out[i]), "i"(4), "i"(8), "i"(12)
            : "memory", "q0", "q1", "q2", "q3", "q4"
        );
        
        iter_counter++;
    }
}
#endif

/* Generic version using GCC vector extensions */
__attribute__((noinline))
void test_many_args_generic(int* restrict out,
                           const int* restrict in1,
                           const int* restrict in2,
                           const int* restrict in3,
                           const int* restrict in4,
                           const int* restrict in5,
                           const int* restrict in6,
                           const int* restrict in7,
                           const int* restrict in8,
                           const int* restrict in9,
                           int n) {
    volatile int iter_counter = 0;
    
    for (int i = 0; i < n; i += 4) {
        inhibit_opt(&iter_counter);
        
        /* Use many different types to create complex RTL */
        char c1 = (char)(in1[i] & 0xFF);
        short s1 = (short)(in2[i] & 0xFFFF);
        int i1 = in3[i];
        long l1 = in4[i];
        
        /* Complex multi-statement expression with many temporaries */
        int temp1 = c1 * s1 + i1;
        int temp2 = (l1 >> 8) & 0xFF;
        int temp3 = temp1 ^ temp2;
        int temp4 = temp3 * 0x01010101;
        int temp5 = (temp4 << 1) | (temp4 >> 31);
        int temp6 = temp5 + in5[i];
        int temp7 = temp6 - in6[i];
        int temp8 = temp7 * in7[i];
        int temp9 = temp8 / (in8[i] + 1);
        int temp10 = temp9 ^ in9[i];
        
        /* Force complex addressing modes */
        out[i] = temp10 + 
                *(int*)((char*)in1 + i * sizeof(int)) +
                *(int*)((char*)in2 + i * sizeof(int) + 1) +
                *(int*)((char*)in3 + i * sizeof(int) + 2) +
                *(int*)((char*)in4 + i * sizeof(int) + 3);
        
        iter_counter++;
    }
}

/* Main test function */
int main(void) {
    const int N = 1024;
    float* in1 = aligned_alloc(64, N * sizeof(float));
    float* in2 = aligned_alloc(64, N * sizeof(float));
    float* in3 = aligned_alloc(64, N * sizeof(float));
    float* in4 = aligned_alloc(64, N * sizeof(float));
    float* in5 = aligned_alloc(64, N * sizeof(float));
    float* out = aligned_alloc(64, N * sizeof(float));
    
    int* in_int1 = aligned_alloc(64, N * sizeof(int));
    int* in_int2 = aligned_alloc(64, N * sizeof(int));
    int* in_int3 = aligned_alloc(64, N * sizeof(int));
    int* in_int4 = aligned_alloc(64, N * sizeof(int));
    int* in_int5 = aligned_alloc(64, N * sizeof(int));
    int* in_int6 = aligned_alloc(64, N * sizeof(int));
    int* in_int7 = aligned_alloc(64, N * sizeof(int));
    int* in_int8 = aligned_alloc(64, N * sizeof(int));
    int* in_int9 = aligned_alloc(64, N * sizeof(int));
    int* out_int = aligned_alloc(64, N * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        in1[i] = (prng_next() % 1000) / 100.0f;
        in2[i] = (prng_next() % 1000) / 100.0f;
        in3[i] = (prng_next() % 1000) / 100.0f;
        in4[i] = (prng_next() % 1000) / 100.0f;
        in5[i] = (prng_next() % 1000) / 100.0f;
        
        in_int1[i] = prng_next() % 1000;
        in_int2[i] = prng_next() % 1000;
        in_int3[i] = prng_next() % 1000;
        in_int4[i] = prng_next() % 1000;
        in_int5[i] = prng_next() % 1000;
        in_int6[i] = prng_next() % 1000;
        in_int7[i] = prng_next() % 1000;
        in_int8[i] = prng_next() % 1000;
        in_int9[i] = prng_next() % 1000;
    }
    
    /* Test different versions based on available ISA */
#ifdef __AVX512F__
    printf("Testing AVX-512 version...\n");
    test_many_args_avx512(out, in1, in2, in3, in4, in5, N);
#elif defined(__AVX2__)
    printf("Testing AVX2 version...\n");
    test_many_args_avx2(out, in1, in2, in3, N);
#elif defined(__ARM_NEON)
    printf("Testing ARM NEON version...\n");
    test_many_args_neon(out, in1, in2, in3, in4, in5, N);
#else
    printf("Testing generic version...\n");
    test_many_args_generic(out_int, in_int1, in_int2, in_int3,
                          in_int4, in_int5, in_int6, in_int7,
                          in_int8, in_int9, N);
#endif
    
    /* Compute checksum */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += out[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(in1);
    free(in2);
    free(in3);
    free(in4);
    free(in5);
    free(out);
    free(in_int1);
    free(in_int2);
    free(in_int3);
    free(in_int4);
    free(in_int5);
    free(in_int6);
    free(in_int7);
    free(in_int8);
    free(in_int9);
    free(out_int);
    
    return 0;
}
