#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Simple PRNG for reproducible results */
static uint32_t seed = 123456789;
static inline uint32_t prng() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Inhibit optimization helpers */
static inline void clobber_memory(void *p, size_t n) {
    asm volatile("" : "+m"(*(volatile char(*)[n])p));
}

static inline void use_value(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Vector types for different architectures */
#ifdef __AVX512F__
typedef __m512i v16si;
typedef __m512  v16sf;
typedef __m512d v8df;
typedef __mmask16 k16;
#elif defined(__AVX2__)
typedef __m256i v8si;
typedef __m256  v8sf;
typedef __m256d v4df;
#else
typedef __m128i v4si;
typedef __m128  v4sf;
#endif

/* Complex expression builder that forces many temporaries */
__attribute__((noinline))
static int complex_multi_arg_expr(int a, int b, int c, int d, int e,
                                  int f, int g, int h, int i, int j) {
    /* Create many intermediate values to force expander to handle many args */
    int t1 = a * b + c;
    int t2 = d ^ e | f;
    int t3 = g << 2;
    int t4 = h >> 1;
    int t5 = i & j;
    int t6 = t1 - t2;
    int t7 = t3 * t4;
    int t8 = t5 + t6;
    int t9 = t7 ^ t8;
    int t10 = t9 * a;
    
    /* Force dependency chain */
    asm volatile("" : "+r"(t1), "+r"(t2), "+r"(t3), "+r"(t4), "+r"(t5),
                       "+r"(t6), "+r"(t7), "+r"(t8), "+r"(t9), "+r"(t10));
    
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
}

/* AVX2 target function with many-argument operations */
#ifdef __AVX2__
__attribute__((target("avx2"), noinline))
static void test_many_args_avx2(float *restrict out, 
                                const float *restrict in1,
                                const float *restrict in2,
                                const float *restrict in3,
                                int n) {
    volatile int prevent_unroll = 1; /* Prevent loop unrolling */
    
    for (int i = 0; i < n; i += 8) {
        /* Load multiple vectors */
        __m256 v1 = _mm256_loadu_ps(&in1[i]);
        __m256 v2 = _mm256_loadu_ps(&in2[i]);
        __m256 v3 = _mm256_loadu_ps(&in3[i]);
        __m256 v4 = _mm256_set1_ps(2.0f);
        __m256 v5 = _mm256_set1_ps(3.0f);
        __m256 v6 = _mm256_set1_ps(4.0f);
        __m256 v7 = _mm256_set1_ps(5.0f);
        __m256 v8 = _mm256_set1_ps(6.0f);
        
        /* Complex chain of operations creating many temporaries */
        __m256 t1 = _mm256_add_ps(v1, v2);
        __m256 t2 = _mm256_mul_ps(v3, v4);
        __m256 t3 = _mm256_sub_ps(t1, t2);
        __m256 t4 = _mm256_fmadd_ps(v5, v6, v7);
        __m256 t5 = _mm256_div_ps(t3, t4);
        __m256 t6 = _mm256_sqrt_ps(t5);
        __m256 t7 = _mm256_max_ps(t6, v8);
        
        /* Inline asm with 10 operands - may trigger optab expansion */
        asm volatile (
            "vmovaps %1, %0\n\t"
            "vaddps %2, %0, %0\n\t"
            "vmulps %3, %0, %0\n\t"
            "vsubps %4, %0, %0\n\t"
            "vfmadd132ps %5, %6, %0\n\t"
            "vdivps %7, %0, %0\n\t"
            "vsqrtps %0, %0\n\t"
            "vmaxps %8, %0, %0\n\t"
            : "=x"(t7)
            : "x"(v1), "x"(v2), "x"(v3), "x"(v4),
              "x"(v5), "x"(v6), "x"(v7), "x"(v8),
              "m"(*in1)  /* Memory operand */
            : "memory"
        );
        
        /* Another complex expression with many arguments */
        float imm1 = 1.5f, imm2 = 2.5f, imm3 = 3.5f, imm4 = 4.5f;
        float imm5 = 5.5f, imm6 = 6.5f, imm7 = 7.5f, imm8 = 8.5f;
        float imm9 = 9.5f, imm10 = 10.5f;
        
        /* Force the compiler to handle many arguments in expression */
        __m256 result = _mm256_set_ps(
            complex_multi_arg_expr(i, i+1, i+2, i+3, i+4, 
                                   i+5, i+6, i+7, i+8, i+9) * 0.01f,
            imm1 * imm2 + imm3 - imm4,
            imm5 / imm6 * imm7,
            imm8 + imm9 - imm10,
            (float)(i & 0xFF) * 0.1f,
            (float)((i >> 8) & 0xFF) * 0.01f,
            (float)((i >> 16) & 0xFF) * 0.001f,
            (float)((i >> 24) & 0xFF) * 0.0001f
        );
        
        /* Blend with previous result using many arguments */
        result = _mm256_blend_ps(t7, result, 0xAA); /* 10101010 pattern */
        
        _mm256_storeu_ps(&out[i], result);
        
        /* Prevent optimization */
        if (prevent_unroll) {
            asm volatile("" : : "r"(&out[i]) : "memory");
        }
    }
}
#endif

/* Generic version for non-AVX targets */
__attribute__((noinline))
static void test_many_args_generic(float *restrict out,
                                   const float *restrict in1,
                                   const float *restrict in2,
                                   const float *restrict in3,
                                   int n) {
    volatile int prevent_unroll = 1;
    
    for (int i = 0; i < n; i++) {
        /* Create many intermediate values */
        float a = in1[i];
        float b = in2[i];
        float c = in3[i];
        float d = (float)(i % 256);
        float e = (float)((i >> 2) % 256);
        float f = (float)((i >> 4) % 256);
        float g = (float)((i >> 6) % 256);
        float h = (float)((i >> 8) % 256);
        float j = (float)((i >> 10) % 256);
        float k = (float)((i >> 12) % 256);
        
        /* Complex expression with many arguments */
        float t1 = a + b * c;
        float t2 = d - e / f;
        float t3 = g * h + j;
        float t4 = k - a * b;
        float t5 = c + d * e;
        float t6 = f - g / h;
        float t7 = j * k + a;
        float t8 = b - c * d;
        float t9 = e + f * g;
        float t10 = h - j / k;
        
        /* Inline asm with 11 operands - targets the 11-argument case */
        asm volatile (
            "fadds %1, %2, %0\n\t"
            "fmuls %3, %0, %0\n\t"
            "fsubs %4, %0, %0\n\t"
            "fdivs %5, %0, %0\n\t"
            "fmadds %6, %7, %0, %0\n\t"
            : "=w"(t10)
            : "w"(t1), "w"(t2), "w"(t3), "w"(t4),
              "w"(t5), "w"(t6), "w"(t7), "w"(t8),
              "w"(t9), "m"(*in1), "m"(*in2)  /* Two memory operands */
            : "memory"
        );
        
        out[i] = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
        
        /* Inhibit CSE and constant propagation */
        asm volatile("" : "+r"(i) : : "memory");
    }
}

/* Test with vector builtins that take many arguments */
#ifdef __ARM_NEON
#include <arm_neon.h>

__attribute__((noinline))
static void test_many_args_neon(float *restrict out,
                                const float *restrict in1,
                                const float *restrict in2,
                                const float *restrict in3,
                                int n) {
    volatile int prevent_unroll = 1;
    
    for (int i = 0; i < n; i += 4) {
        float32x4_t v1 = vld1q_f32(&in1[i]);
        float32x4_t v2 = vld1q_f32(&in2[i]);
        float32x4_t v3 = vld1q_f32(&in3[i]);
        
        /* Create many vector constants */
        float32x4_t c1 = vdupq_n_f32(1.0f);
        float32x4_t c2 = vdupq_n_f32(2.0f);
        float32x4_t c3 = vdupq_n_f32(3.0f);
        float32x4_t c4 = vdupq_n_f32(4.0f);
        float32x4_t c5 = vdupq_n_f32(5.0f);
        float32x4_t c6 = vdupq_n_f32(6.0f);
        float32x4_t c7 = vdupq_n_f32(7.0f);
        float32x4_t c8 = vdupq_n_f32(8.0f);
        
        /* Complex sequence that might be folded into multi-arg optab */
        float32x4_t r1 = vaddq_f32(v1, v2);
        float32x4_t r2 = vmulq_f32(v3, c1);
        float32x4_t r3 = vmlaq_f32(c2, r1, r2);
        float32x4_t r4 = vsubq_f32(r3, c3);
        float32x4_t r5 = vmulq_f32(r4, c4);
        float32x4_t r6 = vaddq_f32(r5, c5);
        float32x4_t r7 = vmlaq_f32(c6, r6, c7);
        float32x4_t r8 = vsubq_f32(r7, c8);
        
        /* Shuffle with computed indices - potentially many arguments */
        uint32_t idx[4] = {1, 3, 0, 2};
        uint32x4_t mask = vld1q_u32(idx);
        float32x4_t shuffled = vqtbl1q_f32(v1, mask);
        
        /* Blend all results */
        float32x4_t result = vbslq_f32(vdupq_n_u32(0xF0F0F0F0), r8, shuffled);
        
        vst1q_f32(&out[i], result);
        
        if (prevent_unroll) {
            asm volatile("" : : "r"(&out[i]) : "memory");
        }
    }
}
#endif

int main() {
    const int N = 1024;
    float *in1 = aligned_alloc(32, N * sizeof(float));
    float *in2 = aligned_alloc(32, N * sizeof(float));
    float *in3 = aligned_alloc(32, N * sizeof(float));
    float *out = aligned_alloc(32, N * sizeof(float));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        in1[i] = (float)prng() / (float)UINT32_MAX;
        in2[i] = (float)prng() / (float)UINT32_MAX;
        in3[i] = (float)prng() / (float)UINT32_MAX;
        out[i] = 0.0f;
    }
    
    /* Clobber to prevent constant propagation */
    clobber_memory(in1, N * sizeof(float));
    clobber_memory(in2, N * sizeof(float));
    clobber_memory(in3, N * sizeof(float));
    
    printf("Testing many-argument optab expansion...\n");
    
#ifdef __AVX2__
    printf("Using AVX2 path\n");
    test_many_args_avx2(out, in1, in2, in3, N);
#elif defined(__ARM_NEON)
    printf("Using NEON path\n");
    test_many_args_neon(out, in1, in2, in3, N);
#else
    printf("Using generic path\n");
    test_many_args_generic(out, in1, in2, in3, N);
#endif
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        checksum += (double)out[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("First 10 results: ");
    for (int i = 0; i < 10 && i < N; i++) {
        printf("%f ", out[i]);
    }
    printf("\n");
    
    free(in1);
    free(in2);
    free(in3);
    free(out);
    
    return 0;
}
