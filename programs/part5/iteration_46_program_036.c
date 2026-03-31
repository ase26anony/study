#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Simple PRNG for deterministic testing */
static uint32_t seed = 123456789;
static inline uint32_t prng() {
    seed = seed * 1103515245 + 12345;
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
static float output[ARRAY_SIZE] __attribute__((aligned(32)));

/* Complex inline assembly with many operands - targets case 10/11 */
static inline __m256 __attribute__((always_inline))
complex_avx_operation_10(__m256 a, __m256 b, __m256 c, __m256 d, __m256 e,
                         int imm1, int imm2, int imm3, int imm4, int imm5) {
    __m256 result;
    /* Extended asm with 10 input operands */
    asm volatile (
        "vmovaps %[a], %%ymm0\n\t"
        "vmovaps %[b], %%ymm1\n\t"
        "vmovaps %[c], %%ymm2\n\t"
        "vmovaps %[d], %%ymm3\n\t"
        "vmovaps %[e], %%ymm4\n\t"
        /* Complex blending/shuffling with many immediates */
        "vblendps $%[i1], %%ymm0, %%ymm1, %%ymm5\n\t"
        "vblendps $%[i2], %%ymm2, %%ymm3, %%ymm6\n\t"
        "vshufps $%[i3], %%ymm5, %%ymm6, %%ymm7\n\t"
        "vperm2f128 $%[i4], %%ymm7, %%ymm4, %%ymm0\n\t"
        "vaddps %%ymm0, %%ymm4, %[res]\n\t"
        : [res] "=x" (result)
        : [a] "x" (a), [b] "x" (b), [c] "x" (c), [d] "x" (d), [e] "x" (e),
          [i1] "i" (imm1), [i2] "i" (imm2), [i3] "i" (imm3), [i4] "i" (imm4),
          [i5] "i" (imm5)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "memory"
    );
    return result;
}

/* 11-argument version */
static inline __m256 __attribute__((always_inline))
complex_avx_operation_11(__m256 a, __m256 b, __m256 c, __m256 d, __m256 e,
                         __m256 f, int imm1, int imm2, int imm3, 
                         int imm4, int imm5) {
    __m256 result;
    asm volatile (
        "vmovaps %[a], %%ymm0\n\t"
        "vmovaps %[b], %%ymm1\n\t"
        "vmovaps %[c], %%ymm2\n\t"
        "vmovaps %[d], %%ymm3\n\t"
        "vmovaps %[e], %%ymm4\n\t"
        "vmovaps %[f], %%ymm5\n\t"
        "vblendps $%[i1], %%ymm0, %%ymm1, %%ymm6\n\t"
        "vblendps $%[i2], %%ymm2, %%ymm3, %%ymm7\n\t"
        "vshufps $%[i3], %%ymm6, %%ymm7, %%ymm0\n\t"
        "vperm2f128 $%[i4], %%ymm0, %%ymm4, %%ymm1\n\t"
        "vfmadd213ps %%ymm5, %%ymm1, %[res]\n\t"
        : [res] "=x" (result)
        : [a] "x" (a), [b] "x" (b), [c] "x" (c), [d] "x" (d), 
          [e] "x" (e), [f] "x" (f),
          [i1] "i" (imm1), [i2] "i" (imm2), [i3] "i" (imm3),
          [i4] "i" (imm4), [i5] "i" (imm5)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "memory"
    );
    return result;
}

/* Complex expression with many temporaries */
static inline float __attribute__((always_inline))
complex_scalar_expr(float a, float b, float c, float d, float e,
                    float f, float g, float h, float i, float j) {
    /* Create many intermediate values to force expander work */
    volatile float t1 = a + b * c;
    volatile float t2 = d - e / f;
    volatile float t3 = g * h + i;
    volatile float t4 = j - a;
    
    /* Complex chain */
    float r1 = t1 * t2 + t3 / t4;
    float r2 = (t1 + t2) * (t3 - t4);
    float r3 = r1 / r2 + t1 * t3;
    float r4 = r2 - r3 * t2;
    
    /* Final expression with many operands */
    return ((r1 * r2) + (r3 * r4)) / ((t1 * t3) - (t2 * t4)) + 
           ((a * b) + (c * d) - (e * f) + (g * h) - (i * j));
}

/* Target-specific function with AVX2 */
__attribute__((target("avx2"), noinline))
void test_many_args(void) {
    volatile int counter = 0; /* Prevent loop unrolling */
    
    for (int i = 0; i < ARRAY_SIZE - VEC_SIZE; i += VEC_SIZE) {
        /* Load multiple vectors */
        __m256 va = _mm256_load_ps(&array_a[i]);
        __m256 vb = _mm256_load_ps(&array_b[i]);
        __m256 vc = _mm256_load_ps(&array_c[i]);
        __m256 vd = _mm256_load_ps(&array_d[i]);
        __m256 ve = _mm256_load_ps(&array_e[i]);
        
        /* Create fake dependencies to inhibit optimization */
        int imm1, imm2, imm3, imm4, imm5;
        asm volatile("" : "=r"(imm1) : "0"(counter & 0xFF));
        asm volatile("" : "=r"(imm2) : "0"((counter >> 8) & 0xFF));
        asm volatile("" : "=r"(imm3) : "0"((counter >> 16) & 0xFF));
        asm volatile("" : "=r"(imm4) : "0"((counter >> 24) & 0xFF));
        imm5 = (imm1 ^ imm2) & 0xF;
        
        /* Call 10-argument operation */
        __m256 res1 = complex_avx_operation_10(va, vb, vc, vd, ve,
                                              imm1, imm2, imm3, imm4, imm5);
        
        /* Create another vector for 11-argument version */
        __m256 vf = _mm256_add_ps(va, vb);
        __m256 res2 = complex_avx_operation_11(va, vb, vc, vd, ve, vf,
                                              imm1, imm2, imm3, imm4, imm5);
        
        /* Blend results */
        __m256 final = _mm256_add_ps(res1, res2);
        _mm256_store_ps(&output[i], final);
        
        /* Also test scalar path with many arguments */
        for (int j = 0; j < VEC_SIZE; j++) {
            float scalar_result = complex_scalar_expr(
                array_a[i+j], array_b[i+j], array_c[i+j],
                array_d[i+j], array_e[i+j],
                (float)imm1, (float)imm2, (float)imm3,
                (float)imm4, (float)imm5
            );
            output[i+j] += scalar_result * 0.5f;
        }
        
        counter++;
    }
}

/* AVX-512 specific version for more complex optabs */
#ifdef __AVX512F__
__attribute__((target("avx512f"), noinline))
void test_avx512_many_args(void) {
    /* AVX-512 can have mask registers as additional arguments */
    __mmask16 mask1 = 0xAAAA;
    __mmask16 mask2 = 0x5555;
    __mmask16 mask3 = 0x0F0F;
    
    for (int i = 0; i < ARRAY_SIZE - 16; i += 16) {
        __m512 va = _mm512_load_ps(&array_a[i]);
        __m512 vb = _mm512_load_ps(&array_b[i]);
        __m512 vc = _mm512_load_ps(&array_c[i]);
        __m512 vd = _mm512_load_ps(&array_d[i]);
        __m512 ve = _mm512_load_ps(&array_e[i]);
        
        /* Complex blending with multiple masks - potentially many args */
        __m512 blended = _mm512_mask_blend_ps(mask1, va, vb);
        blended = _mm512_mask_blend_ps(mask2, blended, vc);
        blended = _mm512_mask_blend_ps(mask3, blended, vd);
        
        /* Permutation with many lane indices */
        __m512 perm = _mm512_permutexvar_ps(
            _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0),
            ve
        );
        
        __m512 result = _mm512_add_ps(blended, perm);
        _mm512_store_ps(&output[i], result);
    }
}
#endif

int main(void) {
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (float)prng() / (float)UINT32_MAX;
        array_b[i] = (float)prng() / (float)UINT32_MAX;
        array_c[i] = (float)prng() / (float)UINT32_MAX;
        array_d[i] = (float)prng() / (float)UINT32_MAX;
        array_e[i] = (float)prng() / (float)UINT32_MAX;
        output[i] = 0.0f;
    }
    
    /* Call the many-argument functions */
    test_many_args();
    
#ifdef __AVX512F__
    test_avx512_many_args();
#endif
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Array[0] = %f, Array[%d] = %f\n", 
           output[0], ARRAY_SIZE-1, output[ARRAY_SIZE-1]);
    
    return 0;
}
