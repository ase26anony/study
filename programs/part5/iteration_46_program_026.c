#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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

/* Prevent optimization of variables */
#define NOOPT __attribute__((optimize("O0")))
#define NOINLINE __attribute__((noinline))
#define TARGET_AVX2 __attribute__((target("avx2")))
#define TARGET_AVX512F __attribute__((target("avx512f")))

/* Complex expression with many temporaries */
NOINLINE TARGET_AVX2
static void test_many_args_avx2(float* restrict out, 
                                 const float* restrict in1,
                                 const float* restrict in2,
                                 const float* restrict in3,
                                 int n) {
    volatile int i; /* Prevent loop unrolling */
    
    for (i = 0; i < n; i += 8) {
        /* Load multiple vectors */
        __m256 v0 = _mm256_loadu_ps(&in1[i]);
        __m256 v1 = _mm256_loadu_ps(&in2[i]);
        __m256 v2 = _mm256_loadu_ps(&in3[i]);
        __m256 v3 = _mm256_set1_ps(1.5f);
        __m256 v4 = _mm256_set1_ps(2.5f);
        __m256 v5 = _mm256_set1_ps(3.5f);
        
        /* Complex chain of operations creating many temporaries */
        __m256 t0 = _mm256_add_ps(v0, v1);
        __m256 t1 = _mm256_mul_ps(t0, v2);
        __m256 t2 = _mm256_sub_ps(v3, v4);
        __m256 t3 = _mm256_fmadd_ps(t1, t2, v5);
        
        /* Extended inline asm with 10-11 operands */
        /* This should trigger the 10-argument optab case */
        __m256 result;
        asm volatile (
            "vmovaps %1, %0\n\t"
            "vaddps %2, %0, %0\n\t"
            "vmulps %3, %0, %0\n\t"
            "vsubps %4, %0, %0\n\t"
            "vfmadd132ps %5, %6, %0\n\t"
            "vblendvps %7, %0, %8, %0\n\t"
            : "=x"(result)
            : "x"(t0), "x"(t1), "x"(t2), "x"(t3),
              "x"(v0), "x"(v1), "x"(v2), "x"(v3),
              "m"(in1[i]), "m"(in2[i])
            : "memory"
        );
        
        _mm256_storeu_ps(&out[i], result);
    }
}

#ifdef __AVX512F__
NOINLINE TARGET_AVX512F
static void test_many_args_avx512(float* restrict out,
                                   const float* restrict in1,
                                   const float* restrict in2,
                                   const float* restrict in3,
                                   const float* restrict in4,
                                   const float* restrict in5,
                                   int n) {
    volatile int i;
    
    for (i = 0; i < n; i += 16) {
        /* Load many vectors - AVX512 has 32 registers */
        __m512 v0 = _mm512_loadu_ps(&in1[i]);
        __m512 v1 = _mm512_loadu_ps(&in2[i]);
        __m512 v2 = _mm512_loadu_ps(&in3[i]);
        __m512 v3 = _mm512_loadu_ps(&in4[i]);
        __m512 v4 = _mm512_loadu_ps(&in5[i]);
        __m512 v5 = _mm512_set1_ps(1.0f);
        __m512 v6 = _mm512_set1_ps(2.0f);
        __m512 v7 = _mm512_set1_ps(3.0f);
        __m512 v8 = _mm512_set1_ps(4.0f);
        
        /* Complex expression with many arguments */
        /* Using GCC vector builtins with many arguments */
        __m512 temp = __builtin_ia32_addps512(v0, v1, 0xFF, _MM_FROUND_CUR_DIRECTION);
        temp = __builtin_ia32_mulps512(temp, v2, 0xFF, _MM_FROUND_CUR_DIRECTION);
        temp = __builtin_ia32_subps512(temp, v3, 0xFF, _MM_FROUND_CUR_DIRECTION);
        
        /* Extended asm with 11 operands - should trigger 11-argument case */
        __m512 result;
        asm volatile (
            "vmovaps %1, %0\n\t"
            "vaddps %2, %0, %0\n\t"
            "vmulps %3, %0, %0\n\t"
            "vsubps %4, %0, %0\n\t"
            "vfmadd132ps %5, %6, %0\n\t"
            "vfmadd231ps %0, %7, %8\n\t"
            "vblendmps %9, %0, %0\n\t"
            : "=v"(result)
            : "v"(temp), "v"(v4), "v"(v5), "v"(v6),
              "v"(v7), "v"(v8), "v"(v0), "v"(v1),
              "v"(v2), "m"(in1[i]), "m"(in2[i])
            : "memory"
        );
        
        _mm512_storeu_ps(&out[i], result);
    }
}
#endif

/* ARM NEON version */
#if defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>

NOINLINE
static void test_many_args_neon(float* restrict out,
                                 const float* restrict in1,
                                 const float* restrict in2,
                                 const float* restrict in3,
                                 int n) {
    volatile int i;
    
    for (i = 0; i < n; i += 4) {
        float32x4_t v0 = vld1q_f32(&in1[i]);
        float32x4_t v1 = vld1q_f32(&in2[i]);
        float32x4_t v2 = vld1q_f32(&in3[i]);
        float32x4_t v3 = vdupq_n_f32(1.5f);
        float32x4_t v4 = vdupq_n_f32(2.5f);
        float32x4_t v5 = vdupq_n_f32(3.5f);
        
        /* Complex vector shuffle/permutation with many arguments */
        /* Using GCC vector builtins */
        float32x4_t temp = vaddq_f32(v0, v1);
        temp = vmulq_f32(temp, v2);
        temp = vsubq_f32(v3, temp);
        temp = vmlaq_f32(v4, temp, v5);
        
        /* Inline asm with many operands */
        float32x4_t result;
        asm volatile (
            "mov v0.16b, %1.16b\n\t"
            "fadd v0.4s, v0.4s, %2.4s\n\t"
            "fmul v0.4s, v0.4s, %3.4s\n\t"
            "fsub v0.4s, v0.4s, %4.4s\n\t"
            "fmla v0.4s, %5.4s, %6.4s\n\t"
            "fmla v0.4s, %7.4s, %8.4s\n\t"
            : "=w"(result)
            : "w"(temp), "w"(v0), "w"(v1), "w"(v2),
              "w"(v3), "w"(v4), "w"(v5), "m"(in1[i]),
              "m"(in2[i]), "m"(in3[i])
            : "v0", "memory"
        );
        
        vst1q_f32(&out[i], result);
    }
}
#endif

/* Generic version using GCC vector extensions */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

NOINLINE
static void test_many_args_generic(float* restrict out,
                                    const float* restrict in1,
                                    const float* restrict in2,
                                    const float* restrict in3,
                                    int n) {
    volatile int i;
    
    for (i = 0; i < n; i += 8) {
        /* Load data into vector types */
        v8sf v0 = *(v8sf*)&in1[i];
        v8sf v1 = *(v8sf*)&in2[i];
        v8sf v2 = *(v8sf*)&in3[i];
        v8sf v3 = (v8sf){1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
        v8sf v4 = (v8sf){2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
        v8sf v5 = (v8sf){3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
        
        /* Complex expression with many operations */
        v8sf t0 = v0 + v1;
        v8sf t1 = t0 * v2;
        v8sf t2 = v3 - v4;
        v8sf t3 = t1 * t2 + v5;
        
        /* Use __builtin_shuffle with many arguments */
        /* This can generate optab calls with many operands */
        v8sf shuffled = __builtin_shuffle(t0, t1, 
            (v8si){0, 9, 2, 11, 4, 13, 6, 15});
        shuffled = __builtin_shuffle(shuffled, t2,
            (v8si){8, 1, 10, 3, 12, 5, 14, 7});
        shuffled = __builtin_shuffle(shuffled, t3,
            (v8si){0, 9, 2, 11, 4, 13, 6, 15, 8, 17, 10, 19, 12, 21, 14, 23});
        
        /* Extended asm with many operands */
        v8sf result;
        asm volatile (
            "vmovaps %1, %0\n\t"
            "vaddps %2, %0, %0\n\t"
            "vmulps %3, %0, %0\n\t"
            "vsubps %4, %0, %0\n\t"
            "vfmadd132ps %5, %6, %0\n\t"
            "vfmadd231ps %0, %7, %8\n\t"
            "vblendvps %9, %0, %10, %0\n\t"
            : "=x"(result)
            : "x"(shuffled), "x"(t0), "x"(t1), "x"(t2),
              "x"(t3), "x"(v0), "x"(v1), "x"(v2),
              "x"(v3), "x"(v4), "m"(in1[i])
            : "memory"
        );
        
        *(v8sf*)&out[i] = result;
    }
}

/* Multi-statement expression with many temporaries */
NOINLINE
static float complex_expression_10_args(float a, float b, float c, float d,
                                         float e, float f, float g, float h,
                                         float i, float j) {
    /* Force many temporaries */
    volatile float t1 = a + b;
    volatile float t2 = c * d;
    volatile float t3 = e - f;
    volatile float t4 = g / h;
    volatile float t5 = i * j;
    
    float r1 = t1 * t2;
    float r2 = t3 + t4;
    float r3 = t5 - a;
    float r4 = r1 / r2;
    float r5 = r3 * r4;
    
    /* Use inline asm to prevent optimization */
    asm volatile("" : "+r"(r1), "+r"(r2), "+r"(r3));
    
    return r5 + b + c + d + e + f + g + h + i + j;
}

/* Array-based complex computation */
NOINLINE
static void array_based_many_args(char* out, const char* in1, const short* in2,
                                   const int* in3, const long* in4, int n) {
    volatile int i;
    
    for (i = 0; i < n; i++) {
        /* Complex pointer arithmetic and indexing */
        char* p1 = (char*)(in1 + i);
        short* p2 = (short*)(in2 + i);
        int* p3 = (int*)(in3 + i);
        long* p4 = (long*)(in4 + i);
        
        /* Multi-statement expression with many operations */
        char val1 = *p1 + i;
        short val2 = *p2 - i;
        int val3 = *p3 * i;
        long val4 = *p4 / (i + 1);
        
        /* Bitwise operations mixing types */
        int temp = (val1 & 0xFF) | ((val2 & 0xFFFF) << 8);
        temp ^= (val3 & 0xFFFFFF);
        temp += (val4 & 0xFFFFFFFF);
        
        /* Extended asm with memory operands */
        asm volatile (
            "add %1, %0\n\t"
            "sub %2, %0\n\t"
            "xor %3, %0\n\t"
            "or %4, %0\n\t"
            "and %5, %0\n\t"
            "shl $3, %0\n\t"
            "shr $1, %0\n\t"
            "imul %6, %0\n\t"
            "idiv %7\n\t"
            : "+r"(temp)
            : "r"(val1), "r"(val2), "r"(val3), "r"((int)val4),
              "i"(0xFFF), "m"(*p1), "m"(*p2), "m"(*p3), "m"(*p4)
            : "memory"
        );
        
        out[i] = (char)(temp & 0xFF);
    }
}

int main(void) {
    const int N = 1024;
    float* in1 = aligned_alloc(64, N * sizeof(float));
    float* in2 = aligned_alloc(64, N * sizeof(float));
    float* in3 = aligned_alloc(64, N * sizeof(float));
    float* in4 = aligned_alloc(64, N * sizeof(float));
    float* in5 = aligned_alloc(64, N * sizeof(float));
    float* out = aligned_alloc(64, N * sizeof(float));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        in1[i] = (prng_next() % 1000) / 100.0f;
        in2[i] = (prng_next() % 1000) / 100.0f;
        in3[i] = (prng_next() % 1000) / 100.0f;
        in4[i] = (prng_next() % 1000) / 100.0f;
        in5[i] = (prng_next() % 1000) / 100.0f;
        out[i] = 0.0f;
    }
    
    printf("Testing many-argument optab expansion...\n");
    
    /* Test AVX2 path */
    test_many_args_avx2(out, in1, in2, in3, N);
    
#ifdef __AVX512F__
    /* Test AVX512 path */
    test_many_args_avx512(out, in1, in2, in3, in4, in5, N);
#endif
    
#if defined(__ARM_NEON) || defined(__aarch64__)
    /* Test NEON path */
    test_many_args_neon(out, in1, in2, in3, N);
#endif
    
    /* Test generic vector path */
    test_many_args_generic(out, in1, in2, in3, N);
    
    /* Test scalar path with 10 arguments */
    float sum = 0.0f;
    for (int i = 0; i < 10; i++) {
        sum += complex_expression_10_args(
            in1[i], in2[i], in3[i], in4[i], in5[i],
            in1[i+1], in2[i+1], in3[i+1], in4[i+1], in5[i+1]
        );
    }
    
    /* Test array-based many-argument computation */
    char* char_out = (char*)aligned_alloc(64, N);
    char* char_in = (char*)aligned_alloc(64, N);
    short* short_in = (short*)aligned_alloc(64, N * sizeof(short));
    int* int_in = (int*)aligned_alloc(64, N * sizeof(int));
    long* long_in = (long*)aligned_alloc(64, N * sizeof(long));
    
    for (int i = 0; i < N; i++) {
        char_in[i] = prng_next() % 256;
        short_in[i] = prng_next() % 65536;
        int_in[i] = prng_next();
        long_in[i] = (long)prng_next() * prng_next();
    }
    
    array_based_many_args(char_out, char_in, short_in, int_in, long_in, N);
    
    /* Compute checksum */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += out[i];
        checksum += char_out[i];
    }
    checksum += sum;
    
    printf("Checksum: %f\n", checksum);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(in1);
    free(in2);
    free(in3);
    free(in4);
    free(in5);
    free(out);
    free(char_out);
    free(char_in);
    free(short_in);
    free(int_in);
    free(long_in);
    
    return 0;
}
