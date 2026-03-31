#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Simple PRNG for deterministic testing */
static uint32_t seed = 123456789;
static inline uint32_t fast_rand(void) {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

/* Initialize arrays with pseudo-random data */
#define ARRAY_SIZE 1024
#define VEC_SIZE 8  /* For AVX2 256-bit vectors (8 floats) */

static float array_a[ARRAY_SIZE] __attribute__((aligned(32)));
static float array_b[ARRAY_SIZE] __attribute__((aligned(32)));
static float array_c[ARRAY_SIZE] __attribute__((aligned(32)));
static float array_d[ARRAY_SIZE] __attribute__((aligned(32)));
static float array_e[ARRAY_SIZE] __attribute__((aligned(32)));
static float array_f[ARRAY_SIZE] __attribute__((aligned(32)));
static float array_g[ARRAY_SIZE] __attribute__((aligned(32)));
static float array_h[ARRAY_SIZE] __attribute__((aligned(32)));
static float output[ARRAY_SIZE] __attribute__((aligned(32)));

/* Complex expression with many temporaries */
static inline __m256 complex_vector_expr(
    __m256 a, __m256 b, __m256 c, __m256 d,
    __m256 e, __m256 f, __m256 g, __m256 h,
    int idx1, int idx2, int idx3) __attribute__((always_inline)) {
    
    /* Create many intermediate values to force expander complexity */
    __m256 t1 = _mm256_add_ps(a, b);
    __m256 t2 = _mm256_sub_ps(c, d);
    __m256 t3 = _mm256_mul_ps(e, f);
    __m256 t4 = _mm256_div_ps(g, h);
    
    /* Complex blend chain - this may expand to many operations */
    __m256 blend1 = _mm256_blend_ps(t1, t2, 0xAA);  /* 10101010 */
    __m256 blend2 = _mm256_blend_ps(t3, t4, 0x55);  /* 01010101 */
    
    /* Shuffle with multiple indices - potentially triggering many-arg optab */
    __m256 shuffled = _mm256_shuffle_ps(blend1, blend2, _MM_SHUFFLE(idx1, idx2, idx3, 0));
    
    /* Permute across lanes - another candidate for complex expansion */
    __m256 permuted = _mm256_permutevar8x32_ps(shuffled, 
        _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0));
    
    return permuted;
}

/* Inline assembly with many operands - designed to trigger optab expansion */
static inline __m256 many_operand_asm(
    __m256 v0, __m256 v1, __m256 v2, __m256 v3,
    __m256 v4, __m256 v5, __m256 v6, __m256 v7,
    int imm1, int imm2, int imm3) {
    
    __m256 result;
    
    /* Extended asm with 11 operands (10 inputs + 1 output) */
    asm volatile (
        /* Complex operation with many dependencies */
        "vmovaps %[v0], %%ymm0\n\t"
        "vmovaps %[v1], %%ymm1\n\t"
        "vmovaps %[v2], %%ymm2\n\t"
        "vmovaps %[v3], %%ymm3\n\t"
        "vmovaps %[v4], %%ymm4\n\t"
        "vmovaps %[v5], %%ymm5\n\t"
        "vmovaps %[v6], %%ymm6\n\t"
        "vmovaps %[v7], %%ymm7\n\t"
        
        /* Multiple operations that might be combined */
        "vaddps %%ymm0, %%ymm1, %%ymm8\n\t"
        "vsubps %%ymm2, %%ymm3, %%ymm9\n\t"
        "vmulps %%ymm4, %%ymm5, %%ymm10\n\t"
        "vdivps %%ymm6, %%ymm7, %%ymm11\n\t"
        
        /* Blend with immediate - could be expanded via optab */
        "vblendps $0x%c[imm1], %%ymm8, %%ymm9, %%ymm12\n\t"
        "vblendps $0x%c[imm2], %%ymm10, %%ymm11, %%ymm13\n\t"
        
        /* Shuffle with immediate - another candidate */
        "vshufps $0x%c[imm3], %%ymm12, %%ymm13, %%ymm0\n\t"
        
        "vmovaps %%ymm0, %[result]\n\t"
        : [result] "=x" (result)
        : [v0] "x" (v0), [v1] "x" (v1), [v2] "x" (v2), [v3] "x" (v3),
          [v4] "x" (v4), [v5] "x" (v5), [v6] "x" (v6), [v7] "x" (v7),
          [imm1] "i" (imm1), [imm2] "i" (imm2), [imm3] "i" (imm3)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",
          "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "memory"
    );
    
    return result;
}

/* Target-specific function with AVX2 */
__attribute__((target("avx2"), noinline))
static void test_many_args(void) {
    volatile int i;  /* Prevent loop unrolling */
    
    for (i = 0; i < ARRAY_SIZE - VEC_SIZE; i += VEC_SIZE) {
        /* Load 8 vectors (8 arguments each) */
        __m256 va = _mm256_load_ps(&array_a[i]);
        __m256 vb = _mm256_load_ps(&array_b[i]);
        __m256 vc = _mm256_load_ps(&array_c[i]);
        __m256 vd = _mm256_load_ps(&array_d[i]);
        __m256 ve = _mm256_load_ps(&array_e[i]);
        __m256 vf = _mm256_load_ps(&array_f[i]);
        __m256 vg = _mm256_load_ps(&array_g[i]);
        __m256 vh = _mm256_load_ps(&array_h[i]);
        
        /* Complex expression with many temporaries */
        __m256 result1 = complex_vector_expr(va, vb, vc, vd, ve, vf, vg, vh,
                                            i & 3, (i >> 2) & 3, (i >> 4) & 3);
        
        /* Inline asm with 11 operands */
        __m256 result2 = many_operand_asm(va, vb, vc, vd, ve, vf, vg, vh,
                                         (i & 7), ((i + 1) & 7), ((i + 2) & 7));
        
        /* Blend the two results */
        __m256 final = _mm256_add_ps(result1, result2);
        
        /* Store result */
        _mm256_store_ps(&output[i], final);
    }
}

/* Alternative approach using GCC vector builtins directly */
#ifdef __GNUC__
typedef float v8sf __attribute__((vector_size(32)));

/* Function using __builtin_shuffle with many arguments */
static inline v8sf complex_shuffle(
    v8sf a, v8sf b, v8sf c, v8sf d,
    v8sf e, v8sf f, v8sf g, v8sf h,
    int idx0, int idx1, int idx2) {
    
    /* Create a complex mask - this might expand to many operations */
    v8sf mask = a + b - c * d / e;
    
    /* Multiple shuffles that could be combined */
    v8sf s1 = __builtin_shuffle(a, b, (v8sf){0, 1, 2, 3, 4, 5, 6, 7});
    v8sf s2 = __builtin_shuffle(c, d, (v8sf){7, 6, 5, 4, 3, 2, 1, 0});
    v8sf s3 = __builtin_shuffle(e, f, (v8sf){idx0, idx1, idx2, 3, 4, 5, 6, 7});
    v8sf s4 = __builtin_shuffle(g, h, (v8sf){7, 6, 5, idx0, idx1, idx2, 1, 0});
    
    /* Complex blend operation */
    v8sf result = __builtin_shuffle(s1, s2, mask > 0 ? 
        (v8sf){0, 1, 2, 3, 12, 13, 14, 15} : 
        (v8sf){8, 9, 10, 11, 4, 5, 6, 7});
    
    return result;
}
#endif

/* Multi-statement expression with many temporaries */
static inline float complex_scalar_expr(
    float a, float b, float c, float d, float e,
    float f, float g, float h, float i, float j) {
    
    /* Many intermediate calculations */
    float t1 = a + b;
    float t2 = c - d;
    float t3 = e * f;
    float t4 = g / h;
    float t5 = i * j;
    float t6 = t1 * t2;
    float t7 = t3 + t4;
    float t8 = t5 - t6;
    float t9 = t7 * t8;
    float t10 = t9 / (t1 + 1.0f);
    
    /* Complex conditional expression */
    float result = (t10 > 0) ? 
        (t1 * t2 + t3 * t4 - t5 * t6 / t7) :
        (t8 * t9 - t10 * t1 / t2 + t3 - t4);
    
    return result;
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (float)fast_rand() / 1000.0f;
        array_b[i] = (float)fast_rand() / 1000.0f;
        array_c[i] = (float)fast_rand() / 1000.0f;
        array_d[i] = (float)fast_rand() / 1000.0f;
        array_e[i] = (float)fast_rand() / 1000.0f;
        array_f[i] = (float)fast_rand() / 1000.0f;
        array_g[i] = (float)fast_rand() / 1000.0f;
        array_h[i] = (float)fast_rand() / 1000.0f;
        output[i] = 0.0f;
    }
    
    /* Call the target function */
    test_many_args();
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Also test scalar path with many arguments */
    float scalar_result = 0.0f;
    for (int i = 0; i < 100; i++) {
        scalar_result += complex_scalar_expr(
            array_a[i], array_b[i], array_c[i], array_d[i], array_e[i],
            array_f[i], array_g[i], array_h[i], 
            (float)i, (float)(i * 2));
    }
    printf("Scalar result: %f\n", scalar_result);
    
    return 0;
}
