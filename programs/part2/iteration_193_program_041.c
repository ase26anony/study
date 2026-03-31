/* test_optabs_coverage.c - Test program to cover 10/11 operand switch cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent unwanted optimizations */
#define NOINLINE __attribute__((noinline, noipa))
#define USED __attribute__((used))

/* Generic vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Volatile sink to prevent optimization */
static volatile int sink = 0;

/* Pattern A: Complex vector blend with many operands */
NOINLINE USED v4sf pattern_a_blend_many_operands(v4sf a, v4sf b, v4sf c, v4sf d, 
                                                 v4sf e, v4sf f, v4sf g, v4sf h,
                                                 int mask1, int mask2, int mask3) {
#ifdef __SSE__
    /* Use SSE intrinsics if available */
    v4sf temp1 = __builtin_ia32_shufps(a, b, mask1);
    v4sf temp2 = __builtin_ia32_shufps(c, d, mask2);
    v4sf temp3 = __builtin_ia32_shufps(e, f, mask3);
    
    /* Complex blend operation that may expand to many operands */
    v4sf result = temp1 + temp2 * temp3 - g + h;
    
    /* Force dependency chain */
    result = result * result + a - b + c - d + e - f;
    
    return result;
#else
    /* Fallback for non-SSE targets */
    return a + b + c + d + e + f + g + h;
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE USED float pattern_b_fma_chain(float a, float b, float c, float d, float e,
                                        float f, float g, float h, float i, float j) {
#ifdef __FMA__
    /* Chain of FMA operations that may flatten to many operands */
    float res1 = __builtin_fma(a, b, c);
    float res2 = __builtin_fma(d, e, f);
    float res3 = __builtin_fma(g, h, i);
    float final = __builtin_fma(res1, res2, res3);
    return __builtin_fma(final, j, a + b + c + d);
#else
    /* Manual FMA simulation */
    return a * b + c + d * e + f + g * h + i + j;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE USED float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Extract each element manually - creates many extract operations */
    float sum = 0.0f;
    
    /* This unrolled extraction creates many operands */
    sum += v1[0] + v1[1] + v1[2] + v1[3];
    sum += v2[0] + v2[1] + v2[2] + v2[3];
    sum += v3[0] + v3[1] + v3[2] + v3[3];
    sum += v4[0] + v4[1] + v4[2] + v4[3];
    
    /* Additional arithmetic to prevent simplification */
    sum = sum * sum - sum / 2.0f + sum * 1.5f;
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE USED v4sf pattern_d_conditional_blend(v4sf a, v4sf b, v4sf c, v4sf d,
                                               v4sf e, v4sf f, v4sf g, v4sf h) {
#ifdef __SSE__
    /* Multiple comparisons and blends */
    v4sf cmp1 = a > b;
    v4sf cmp2 = c < d;
    v4sf cmp3 = e == f;
    v4sf cmp4 = g >= h;
    
    /* Complex blend logic */
    v4sf temp1 = cmp1 ? a : b;
    v4sf temp2 = cmp2 ? c : d;
    v4sf temp3 = cmp3 ? e : f;
    v4sf temp4 = cmp4 ? g : h;
    
    /* Final combination with many operands */
    return temp1 + temp2 - temp3 * temp4 + a - b + c - d;
#else
    return a + b + c + d + e + f + g + h;
#endif
}

/* Pattern E: Inline assembly with exactly 10/11 operands */
NOINLINE USED int pattern_e_multi_operand_asm(int a, int b, int c, int d, int e,
                                              int f, int g, int h, int i, int j) {
    int result1, result2;
    
    /* Assembly with 10 explicit operands */
    asm volatile (
        "add %[r1], %[a], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "add %[r1], %[r1], %[d]\n\t"
        "add %[r2], %[e], %[f]\n\t"
        "add %[r2], %[r2], %[g]\n\t"
        "add %[r2], %[r2], %[h]\n\t"
        "mul %[r1], %[r1], %[r2]\n\t"
        "add %[r1], %[r1], %[i]\n\t"
        "add %[r1], %[r1], %[j]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1;
}

/* Pattern F: Vector shuffle with immediate and many vector arguments */
NOINLINE USED v4sf pattern_f_complex_shuffle(v4sf a, v4sf b, v4sf c, v4sf d,
                                             v4sf e, v4sf f, int imm1, int imm2,
                                             int imm3, int imm4) {
#ifdef __SSE4_1__
    /* Complex shuffle pattern that may require many operands */
    v4sf shuffled1 = __builtin_ia32_shufps(a, b, imm1);
    v4sf shuffled2 = __builtin_ia32_shufps(c, d, imm2);
    v4sf shuffled3 = __builtin_ia32_shufps(e, f, imm3);
    
    /* Blend them together */
    v4sf temp = __builtin_ia32_blendps(shuffled1, shuffled2, imm4);
    
    /* More operations to increase operand count */
    return temp + shuffled3 - a + b - c + d - e + f;
#elif defined(__SSE__)
    /* SSE2/SSE3 fallback */
    v4sf temp1 = __builtin_ia32_shufps(a, b, imm1);
    v4sf temp2 = __builtin_ia32_shufps(c, d, imm2);
    return temp1 + temp2 + e + f;
#else
    return a + b + c + d + e + f;
#endif
}

/* Pattern G: ARM NEON specific - many vector lane operations */
#ifdef __ARM_NEON
#include <arm_neon.h>
NOINLINE USED float32x4_t pattern_g_neon_multi_lane(float32x4_t a, float32x4_t b,
                                                    float32x4_t c, float32x4_t d,
                                                    float32x4_t e, float32x4_t f) {
    /* NEON operations that may expand to many RTL operands */
    float32x4_t vadd1 = vaddq_f32(a, b);
    float32x4_t vadd2 = vaddq_f32(c, d);
    float32x4_t vadd3 = vaddq_f32(e, f);
    
    float32x4_t vmul1 = vmulq_f32(vadd1, vadd2);
    float32x4_t vmul2 = vmulq_f32(vadd3, a);
    
    /* Lane extractions and inserts */
    float32_t lane0 = vgetq_lane_f32(vmul1, 0);
    float32_t lane1 = vgetq_lane_f32(vmul1, 1);
    float32_t lane2 = vgetq_lane_f32(vmul1, 2);
    float32_t lane3 = vgetq_lane_f32(vmul1, 3);
    
    float32x4_t result = vsetq_lane_f32(lane0 + lane1, vmul2, 0);
    result = vsetq_lane_f32(lane2 + lane3, result, 1);
    
    return vaddq_f32(result, vmul1);
}
#endif

/* Main test driver */
int main(int argc, char *argv[]) {
    int checksum = 0;
    
    /* Initialize test data with some variability */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf vec5 = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf vec6 = {21.0f, 22.0f, 23.0f, 24.0f};
    
    /* Use argc to select different patterns, ensuring all get compiled */
    if (argc > 1) {
        /* Pattern A - Complex blend */
        v4sf result_a = pattern_a_blend_many_operands(vec1, vec2, vec3, vec4, 
                                                     vec5, vec6, vec1, vec2,
                                                     0x1B, 0x2D, 0x3E);
        checksum += (int)result_a[0];
        
        /* Pattern B - FMA chain */
        float result_b = pattern_b_fma_chain(1.5f, 2.5f, 3.5f, 4.5f, 5.5f,
                                            6.5f, 7.5f, 8.5f, 9.5f, 10.5f);
        checksum += (int)result_b;
    }
    
    if (argc > 2) {
        /* Pattern C - Vector reduction */
        float result_c = pattern_c_vector_reduction(vec1, vec2, vec3, vec4);
        checksum += (int)result_c;
        
        /* Pattern D - Conditional blend */
        v4sf result_d = pattern_d_conditional_blend(vec1, vec2, vec3, vec4,
                                                   vec5, vec6, vec1, vec2);
        checksum += (int)result_d[1];
    }
    
    if (argc > 3) {
        /* Pattern E - Multi-operand assembly */
        int result_e = pattern_e_multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        checksum += result_e;
        
        /* Pattern F - Complex shuffle */
        v4sf result_f = pattern_f_complex_shuffle(vec1, vec2, vec3, vec4,
                                                 vec5, vec6, 0x1B, 0x2D, 0x3E, 0xF);
        checksum += (int)result_f[2];
    }
    
#ifdef __ARM_NEON
    if (argc > 4) {
        /* Pattern G - ARM NEON operations */
        float32x4_t neon1 = {1.0f, 2.0f, 3.0f, 4.0f};
        float32x4_t neon2 = {5.0f, 6.0f, 7.0f, 8.0f};
        float32x4_t neon3 = {9.0f, 10.0f, 11.0f, 12.0f};
        float32x4_t neon4 = {13.0f, 14.0f, 15.0f, 16.0f};
        float32x4_t neon5 = {17.0f, 18.0f, 19.0f, 20.0f};
        float32x4_t neon6 = {21.0f, 22.0f, 23.0f, 24.0f};
        
        float32x4_t result_g = pattern_g_neon_multi_lane(neon1, neon2, neon3, 
                                                        neon4, neon5, neon6);
        checksum += (int)vgetq_lane_f32(result_g, 0);
    }
#endif
    
    /* Use the checksum to prevent optimization */
    sink = checksum;
    
    printf("Checksum: %d\n", checksum);
    return checksum == 0 ? 0 : 1;
}
