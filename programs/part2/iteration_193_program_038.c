/* test_optabs_coverage.c - Test program to cover 10/11 operand switch cases in optabs.cc */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of critical functions */
#define NOOPT __attribute__((noinline, noipa, used))

/* Generic vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Volatile sink to prevent dead code elimination */
static volatile int sink;

/* Pattern A: Complex vector shuffle with many operands */
NOOPT v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d, 
                             int imm1, int imm2, int imm3, int imm4) {
#ifdef __SSE__
    /* Use SSE shuffle intrinsics - these expand to many RTL operands */
    v4sf t1 = __builtin_ia32_shufps(a, b, imm1);
    v4sf t2 = __builtin_ia32_shufps(c, d, imm2);
    v4sf t3 = __builtin_ia32_shufps(t1, t2, imm3);
    return __builtin_ia32_shufps(t3, t1, imm4);
#else
    /* Fallback for non-SSE targets */
    return a + b + c + d;
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOOPT float pattern_b_fma_chain(float a, float b, float c, float d,
                                float e, float f, float g, float h,
                                float i, float j, float k) {
#ifdef __FMA__
    /* Chain of FMA operations - expands to many operands during RTL generation */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(t1, t2, t3);
    return __builtin_fmaf(t4, j, k);
#else
    /* Manual FMA emulation */
    return a * b + c + d * e + f + g * h + i + j * k;
#endif
}

/* Pattern C: Vector reduction with explicit lane extraction */
NOOPT float pattern_c_vector_reduce(v4sf v) {
    float sum = 0.0f;
    
    /* Extract each lane individually - each extract is an operand */
#ifdef __SSE__
    sum += __builtin_ia32_vec_ext_v4sf(v, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v, 1);
    sum += __builtin_ia32_vec_ext_v4sf(v, 2);
    sum += __builtin_ia32_vec_ext_v4sf(v, 3);
#else
    /* Portable extraction */
    float* p = (float*)&v;
    sum = p[0] + p[1] + p[2] + p[3];
#endif
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOOPT v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c, v4sf d,
                                        v4sf mask1, v4sf mask2) {
#ifdef __SSE__
    /* Multiple comparisons and blends - each creates many RTL operands */
    v4sf cmp1 = __builtin_ia32_cmpps(a, b, 1);  /* LT comparison */
    v4sf cmp2 = __builtin_ia32_cmpps(c, d, 1);  /* LT comparison */
    v4sf and1 = __builtin_ia32_andps(cmp1, mask1);
    v4sf and2 = __builtin_ia32_andps(cmp2, mask2);
    v4sf or1 = __builtin_ia32_orps(and1, and2);
    
    /* Blend based on complex mask */
    v4sf t1 = __builtin_ia32_blendps(a, b, 0x5);
    v4sf t2 = __builtin_ia32_blendps(c, d, 0xA);
    return __builtin_ia32_blendps(t1, t2, 0x3);
#else
    return a + b + c + d;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOOPT int pattern_e_multi_operand_asm(int a, int b, int c, int d, int e,
                                      int f, int g, int h, int i, int j) {
    int result;
    
    /* Inline asm with 11 total operands (1 output + 10 inputs) */
    asm volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "mov %[b], %[out]"
        : [out] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result;
}

/* Pattern F: AVX2 gather operation (if available) - many operands */
NOOPT v8sf pattern_f_gather_style(float* base, v8si indices, v8sf src,
                                  v8sf mask, int scale) {
#ifdef __AVX2__
    /* Simulate gather-like operation with many parameters */
    v8sf result = src;
    
    /* Complex operation mixing many operands */
    for (int i = 0; i < 8; i++) {
        int idx = ((int*)&indices)[i];
        if (((int*)&mask)[i] != 0) {
            ((float*)&result)[i] = base[idx * scale];
        }
    }
    
    /* Additional operations to increase operand count */
    result = result + src * mask;
    return result;
#else
    return src;
#endif
}

/* Main test driver that exercises all patterns */
int main(int argc, char** argv) {
    /* Use argc for runtime variability to prevent constant folding */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Initialize test data */
    v4sf vec1 = {1.0f + seed, 2.0f + seed, 3.0f + seed, 4.0f + seed};
    v4sf vec2 = {5.0f + seed, 6.0f + seed, 7.0f + seed, 8.0f + seed};
    v4sf vec3 = {9.0f + seed, 10.0f + seed, 11.0f + seed, 12.0f + seed};
    v4sf vec4 = {13.0f + seed, 14.0f + seed, 15.0f + seed, 16.0f + seed};
    
    v4si ivec1 = {seed + 1, seed + 2, seed + 3, seed + 4};
    v4sf mask1 = {0.0f, 1.0f, 0.0f, 1.0f};
    v4sf mask2 = {1.0f, 0.0f, 1.0f, 0.0f};
    
    /* Exercise Pattern A - Complex shuffle */
    v4sf res_a = pattern_a_shuffle(vec1, vec2, vec3, vec4, 
                                   seed & 0xFF, (seed >> 8) & 0xFF,
                                   (seed >> 16) & 0xFF, (seed >> 24) & 0xFF);
    sink = ((int*)&res_a)[0];
    
    /* Exercise Pattern B - FMA chain */
    float res_b = pattern_b_fma_chain(1.1f + seed, 2.2f + seed, 3.3f + seed,
                                      4.4f + seed, 5.5f + seed, 6.6f + seed,
                                      7.7f + seed, 8.8f + seed, 9.9f + seed,
                                      10.1f + seed, 11.1f + seed);
    sink = (int)res_b;
    
    /* Exercise Pattern C - Vector reduction */
    float res_c = pattern_c_vector_reduce(vec1);
    sink = (int)res_c;
    
    /* Exercise Pattern D - Conditional select */
    v4sf res_d = pattern_d_conditional_select(vec1, vec2, vec3, vec4, mask1, mask2);
    sink = ((int*)&res_d)[1];
    
    /* Exercise Pattern E - Multi-operand inline asm */
    int res_e = pattern_e_multi_operand_asm(seed, seed+1, seed+2, seed+3,
                                            seed+4, seed+5, seed+6, seed+7,
                                            seed+8, seed+9);
    sink = res_e;
    
    /* Exercise Pattern F - Gather-style operation */
    float base_array[100];
    for (int i = 0; i < 100; i++) base_array[i] = (float)(i + seed);
    
    v8si indices8 = {1, 3, 5, 7, 9, 11, 13, 15};
    v8sf src8 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf mask8 = {1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f};
    
#ifdef __AVX2__
    v8sf res_f = pattern_f_gather_style(base_array, indices8, src8, mask8, 2);
    sink = ((int*)&res_f)[0];
#endif
    
    /* Return checksum to ensure all computations are used */
    return sink & 0xFF;
}
