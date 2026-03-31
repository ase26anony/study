/* test_many_args.c - Program to trigger 10-11 argument optab expansion */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

/* Simple PRNG for reproducible results */
static uint32_t prng_state = 123456789;
static inline uint32_t simple_prng() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(float* arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr[i] = (float)(simple_prng() % 1000) / 100.0f;
    }
}

/* Complex inline assembly with many operands - targeting case 11 */
__attribute__((noinline, target("avx2,avx512f")))
static void many_operand_asm(
    __m256i* out, 
    const __m256i* a, 
    const __m256i* b,
    const __m256i* c,
    const __m256i* d,
    const __m256i* e,
    const __m256i* f,
    const __m256i* g,
    const __m256i* h,
    const __m256i* i,
    const __m256i* j
) {
    /* Extended asm with 11 input operands */
    asm volatile (
        "vpaddq %[tmp1], %[va], %[va]\n\t"
        "vpaddq %[tmp2], %[vb], %[vb]\n\t"
        "vpaddq %[tmp3], %[vc], %[vc]\n\t"
        "vpaddq %[tmp4], %[vd], %[vd]\n\t"
        "vpaddq %[tmp5], %[ve], %[ve]\n\t"
        "vpaddq %[tmp6], %[vf], %[vf]\n\t"
        "vpaddq %[tmp7], %[vg], %[vg]\n\t"
        "vpaddq %[tmp8], %[vh], %[vh]\n\t"
        "vpaddq %[tmp9], %[vi], %[vi]\n\t"
        "vpaddq %[tmp10], %[vj], %[vj]\n\t"
        "vpaddq %[va], %[vb], %[out0]\n\t"
        "vpaddq %[vc], %[vd], %[out1]\n\t"
        "vpaddq %[ve], %[vf], %[out2]\n\t"
        "vpaddq %[vg], %[vh], %[out3]\n\t"
        "vpaddq %[vi], %[vj], %[out4]\n\t"
        : [out0] "=x" (out[0]),
          [out1] "=x" (out[1]),
          [out2] "=x" (out[2]),
          [out3] "=x" (out[3]),
          [out4] "=x" (out[4]),
          [va] "+x" (a[0]),
          [vb] "+x" (b[0]),
          [vc] "+x" (c[0]),
          [vd] "+x" (d[0]),
          [ve] "+x" (e[0]),
          [vf] "+x" (f[0]),
          [vg] "+x" (g[0]),
          [vh] "+x" (h[0]),
          [vi] "+x" (i[0]),
          [vj] "+x" (j[0])
        : [tmp1] "i" (1),
          [tmp2] "i" (2),
          [tmp3] "i" (3),
          [tmp4] "i" (4),
          [tmp5] "i" (5),
          [tmp6] "i" (6),
          [tmp7] "i" (7),
          [tmp8] "i" (8),
          [tmp9] "i" (9),
          [tmp10] "i" (10)
        : "memory"
    );
}

/* Complex vector shuffle with many arguments - targeting case 10 */
__attribute__((noinline, target("avx2")))
static __m256i complex_shuffle_10_args(
    __m256i a, __m256i b, __m256i c, __m256i d,
    __m256i e, __m256i f, __m256i g, __m256i h,
    __m256i i, __m256i j
) {
    /* Create a complex expression with many temporaries */
    __m256i t1 = _mm256_add_epi32(a, b);
    __m256i t2 = _mm256_add_epi32(c, d);
    __m256i t3 = _mm256_add_epi32(e, f);
    __m256i t4 = _mm256_add_epi32(g, h);
    __m256i t5 = _mm256_add_epi32(i, j);
    
    /* Complex shuffle/permute chain */
    __m256i s1 = _mm256_shuffle_epi32(t1, _MM_SHUFFLE(3, 2, 1, 0));
    __m256i s2 = _mm256_shuffle_epi32(t2, _MM_SHUFFLE(0, 1, 2, 3));
    __m256i s3 = _mm256_shuffle_epi32(t3, _MM_SHUFFLE(1, 0, 3, 2));
    __m256i s4 = _mm256_shuffle_epi32(t4, _MM_SHUFFLE(2, 3, 0, 1));
    __m256i s5 = _mm256_shuffle_epi32(t5, _MM_SHUFFLE(3, 0, 1, 2));
    
    /* Blend operations creating complex dependency chain */
    __m256i b1 = _mm256_blend_epi32(s1, s2, 0x0F);
    __m256i b2 = _mm256_blend_epi32(s3, s4, 0xF0);
    __m256i b3 = _mm256_blend_epi32(b1, b2, 0x33);
    __m256i b4 = _mm256_blend_epi32(b3, s5, 0xCC);
    
    /* Final permute with immediate arguments */
    return _mm256_permutevar8x32_epi32(b4, 
        _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0));
}

/* Multi-statement expression with many temporaries */
__attribute__((noinline, target("avx512f")))
static __m512i complex_expression_11_args(
    __m512i a, __m512i b, __m512i c, __m512i d,
    __m512i e, __m512i f, __m512i g, __m512i h,
    __m512i i, __m512i j, __m512i k
) {
    /* Use volatile to inhibit optimization */
    volatile __m512i va = a;
    volatile __m512i vb = b;
    volatile __m512i vc = c;
    volatile __m512i vd = d;
    volatile __m512i ve = e;
    volatile __m512i vf = f;
    volatile __m512i vg = g;
    volatile __m512i vh = h;
    volatile __m512i vi = i;
    volatile __m512i vj = j;
    volatile __m512i vk = k;
    
    /* Complex chain of operations */
    __m512i t1 = _mm512_add_epi64(va, vb);
    __m512i t2 = _mm512_add_epi64(vc, vd);
    __m512i t3 = _mm512_add_epi64(ve, vf);
    __m512i t4 = _mm512_add_epi64(vg, vh);
    __m512i t5 = _mm512_add_epi64(vi, vj);
    
    /* Blend with mask - many arguments */
    __mmask8 mask = 0xAA;
    __m512i b1 = _mm512_mask_blend_epi64(mask, t1, t2);
    __m512i b2 = _mm512_mask_blend_epi64(mask ^ 0xFF, t3, t4);
    __m512i b3 = _mm512_mask_blend_epi64(0x55, b1, b2);
    __m512i b4 = _mm512_mask_blend_epi64(0x33, b3, t5);
    
    /* Shuffle with many lane indices */
    return _mm512_permutexvar_epi64(
        _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0),
        _mm512_add_epi64(b4, vk));
}

/* Main test function with hot loop */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args(float* output, const float* input, size_t size) {
    /* Force many vector loads and operations */
    for (volatile size_t idx = 0; idx < size - 64; idx += 8) {
        /* Load 11 vectors - enough for 11-argument operations */
        __m256i v0 = _mm256_loadu_si256((const __m256i*)(input + idx + 0));
        __m256i v1 = _mm256_loadu_si256((const __m256i*)(input + idx + 8));
        __m256i v2 = _mm256_loadu_si256((const __m256i*)(input + idx + 16));
        __m256i v3 = _mm256_loadu_si256((const __m256i*)(input + idx + 24));
        __m256i v4 = _mm256_loadu_si256((const __m256i*)(input + idx + 32));
        __m256i v5 = _mm256_loadu_si256((const __m256i*)(input + idx + 40));
        __m256i v6 = _mm256_loadu_si256((const __m256i*)(input + idx + 48));
        __m256i v7 = _mm256_loadu_si256((const __m256i*)(input + idx + 56));
        __m256i v8 = _mm256_loadu_si256((const __m256i*)(input + idx + 64));
        __m256i v9 = _mm256_loadu_si256((const __m256i*)(input + idx + 72));
        __m256i v10 = _mm256_loadu_si256((const __m256i*)(input + idx + 80));
        
        /* Call 10-argument function */
        __m256i result10 = complex_shuffle_10_args(v0, v1, v2, v3, v4, 
                                                   v5, v6, v7, v8, v9);
        
        /* Prepare for 11-argument asm */
        __m256i asm_results[5];
        many_operand_asm(asm_results, &v0, &v1, &v2, &v3, &v4, 
                         &v5, &v6, &v7, &v8, &v9, &v10);
        
        /* Combine results */
        __m256i final = _mm256_add_epi32(result10, asm_results[0]);
        _mm256_storeu_si256((__m256i*)(output + idx), final);
        
        /* Also test AVX-512 11-argument path if available */
#ifdef __AVX512F__
        __m512i z0 = _mm512_loadu_si512((const __m512i*)(input + idx));
        __m512i z1 = _mm512_loadu_si512((const __m512i*)(input + idx + 16));
        __m512i z2 = _mm512_loadu_si512((const __m512i*)(input + idx + 32));
        __m512i z3 = _mm512_loadu_si512((const __m512i*)(input + idx + 48));
        __m512i z4 = _mm512_loadu_si512((const __m512i*)(input + idx + 64));
        __m512i z5 = _mm512_loadu_si512((const __m512i*)(input + idx + 80));
        __m512i z6 = _mm512_loadu_si512((const __m512i*)(input + idx + 96));
        __m512i z7 = _mm512_loadu_si512((const __m512i*)(input + idx + 112));
        __m512i z8 = _mm512_loadu_si512((const __m512i*)(input + idx + 128));
        __m512i z9 = _mm512_loadu_si512((const __m512i*)(input + idx + 144));
        __m512i z10 = _mm512_loadu_si512((const __m512i*)(input + idx + 160));
        
        __m512i zresult = complex_expression_11_args(z0, z1, z2, z3, z4,
                                                     z5, z6, z7, z8, z9, z10);
        _mm512_storeu_si512((__m512i*)(output + idx + 32), zresult);
#endif
    }
}

/* Alternative ARM NEON version */
#ifdef __ARM_NEON
#include <arm_neon.h>

__attribute__((noinline))
static int32x4_t neon_many_args(
    int32x4_t a, int32x4_t b, int32x4_t c, int32x4_t d,
    int32x4_t e, int32x4_t f, int32x4_t g, int32x4_t h,
    int32x4_t i, int32x4_t j
) {
    /* Complex NEON operations with many arguments */
    int32x4_t t1 = vaddq_s32(a, b);
    int32x4_t t2 = vaddq_s32(c, d);
    int32x4_t t3 = vaddq_s32(e, f);
    int32x4_t t4 = vaddq_s32(g, h);
    int32x4_t t5 = vaddq_s32(i, j);
    
    /* Multiple shuffles/permutes */
    int32x4_t s1 = vrev64q_s32(t1);
    int32x4_t s2 = vrev64q_s32(t2);
    int32x4_t s3 = vcombine_s32(vget_high_s32(t3), vget_low_s32(t3));
    int32x4_t s4 = vcombine_s32(vget_low_s32(t4), vget_high_s32(t4));
    
    /* Complex blend-like operations */
    int32x4_t b1 = vbslq_s32(vdupq_n_u32(0xFFFFFFFF), s1, s2);
    int32x4_t b2 = vbslq_s32(vdupq_n_u32(0x00000000), s3, s4);
    int32x4_t b3 = vbslq_s32(vdupq_n_u32(0xF0F0F0F0), b1, b2);
    
    return vaddq_s32(b3, t5);
}
#endif

int main() {
    const size_t ARRAY_SIZE = 1024;
    float* input = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* output = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_arrays(input, ARRAY_SIZE);
    memset(output, 0, ARRAY_SIZE * sizeof(float));
    
    printf("Testing many-argument optab expansion...\n");
    
    /* Run the test */
    test_many_args(output, input, ARRAY_SIZE);
    
    /* Compute checksum */
    double checksum = 0.0;
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
    }
    
    printf("Output checksum: %f\n", checksum);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(input);
    free(output);
    
    return 0;
}
