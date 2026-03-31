/* test_optabs_high_operand_count.c
 * 
 * This program is designed to trigger the 10 and 11 operand switch cases
 * in GCC's optabs.cc during RTL expansion. It uses various patterns that
 * generate complex RTL with many operands.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent aggressive optimization that might simplify our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Define vector types for different architectures */
#ifdef __SSE__
#include <xmmintrin.h>
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
#endif

#ifdef __AVX__
#include <immintrin.h>
typedef float v8sf __attribute__((vector_size(32)));
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
typedef float32x4_t v4sf;
typedef int32x4_t v4si;
#endif

/* Volatile variable to prevent dead code elimination */
static volatile int g_volatile_sink = 0;

/* Pattern A: Complex vector blend with many operands */
NOOPT v4sf pattern_a_blend_many_operands(v4sf a, v4sf b, v4sf c, v4sf d, 
                                         v4sf e, v4sf f, v4sf g, v4sf h,
                                         int mask1, int mask2, int mask3, int mask4)
{
    /* This pattern should generate many operands during expansion */
#ifdef __SSE4_1__
    /* Use blendps with dynamic mask - may expand to many operations */
    v4sf result = a;
    
    /* Chain multiple blend operations with different masks */
    for (int i = 0; i < 4; i++) {
        /* Each blend operation with dynamic mask may expand to multiple RTL insns */
        int mask = (mask1 >> i) & 1 ? 0xFF : 0x00;
        result = __builtin_ia32_blendps(result, b, mask);
        mask = (mask2 >> i) & 1 ? 0xFF : 0x00;
        result = __builtin_ia32_blendps(result, c, mask);
        mask = (mask3 >> i) & 1 ? 0xFF : 0x00;
        result = __builtin_ia32_blendps(result, d, mask);
        mask = (mask4 >> i) & 1 ? 0xFF : 0x00;
        result = __builtin_ia32_blendps(result, e, mask);
    }
    
    /* Additional arithmetic to create data dependencies */
    result = result + f * g - h;
    return result;
#elif defined(__ARM_NEON)
    /* ARM NEON equivalent with bitselect */
    v4sf result = a;
    uint32x4_t mask_vec;
    
    for (int i = 0; i < 4; i++) {
        uint32_t mask_val = (mask1 >> i) & 1 ? 0xFFFFFFFF : 0;
        mask_vec = vdupq_n_u32(mask_val);
        result = vbslq_f32(mask_vec, b, result);
        
        mask_val = (mask2 >> i) & 1 ? 0xFFFFFFFF : 0;
        mask_vec = vdupq_n_u32(mask_val);
        result = vbslq_f32(mask_vec, c, result);
        
        mask_val = (mask3 >> i) & 1 ? 0xFFFFFFFF : 0;
        mask_vec = vdupq_n_u32(mask_val);
        result = vbslq_f32(mask_vec, d, result);
        
        mask_val = (mask4 >> i) & 1 ? 0xFFFFFFFF : 0;
        mask_vec = vdupq_n_u32(mask_val);
        result = vbslq_f32(mask_vec, e, result);
    }
    
    result = vaddq_f32(result, vmulq_f32(f, g));
    result = vsubq_f32(result, h);
    return result;
#else
    /* Fallback for architectures without SIMD */
    v4sf result = a + b + c + d + e + f + g + h;
    return result;
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOOPT float pattern_b_fma_chain(float a, float b, float c, float d, float e,
                                float f, float g, float h, float i, float j,
                                float k, float l, float m, float n, float o)
{
    /* Chain of FMA operations - may be expanded to many operands */
#ifdef __FMA__
    /* Use builtin_fma to create complex expression tree */
    float result = __builtin_fma(a, b, 
                     __builtin_fma(c, d,
                       __builtin_fma(e, f,
                         __builtin_fma(g, h,
                           __builtin_fma(i, j,
                             __builtin_fma(k, l,
                               __builtin_fma(m, n, o)))))));
    
    /* Additional operations to increase operand count */
    result = result * 2.0f - 1.0f;
    return result;
#else
    /* Manual FMA emulation that creates many operations */
    float t1 = a * b + c;
    float t2 = d * e + f;
    float t3 = g * h + i;
    float t4 = j * k + l;
    float t5 = m * n + o;
    
    float result = t1 * t2 + t3 * t4 + t5;
    result = result * 2.0f - 1.0f;
    return result;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOOPT float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4)
{
    /* Manually extract and sum all vector elements - creates many extract operations */
    float sum = 0.0f;
    
#ifdef __SSE__
    /* Extract each element individually */
    sum += ((float*)&v1)[0] + ((float*)&v1)[1] + ((float*)&v1)[2] + ((float*)&v1)[3];
    sum += ((float*)&v2)[0] + ((float*)&v2)[1] + ((float*)&v2)[2] + ((float*)&v2)[3];
    sum += ((float*)&v3)[0] + ((float*)&v3)[1] + ((float*)&v3)[2] + ((float*)&v3)[3];
    sum += ((float*)&v4)[0] + ((float*)&v4)[1] + ((float*)&v4)[2] + ((float*)&v4)[3];
    
    /* Use builtin_extract if available for more complex patterns */
    #ifdef __SSE4_1__
    sum += __builtin_ia32_vec_ext_v4sf(v1, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v1, 1);
    sum += __builtin_ia32_vec_ext_v4sf(v1, 2);
    sum += __builtin_ia32_vec_ext_v4sf(v1, 3);
    #endif
#elif defined(__ARM_NEON)
    /* ARM NEON extraction */
    sum += vgetq_lane_f32(v1, 0) + vgetq_lane_f32(v1, 1) + 
           vgetq_lane_f32(v1, 2) + vgetq_lane_f32(v1, 3);
    sum += vgetq_lane_f32(v2, 0) + vgetq_lane_f32(v2, 1) + 
           vgetq_lane_f32(v2, 2) + vgetq_lane_f32(v2, 3);
    sum += vgetq_lane_f32(v3, 0) + vgetq_lane_f32(v3, 1) + 
           vgetq_lane_f32(v3, 2) + vgetq_lane_f32(v3, 3);
    sum += vgetq_lane_f32(v4, 0) + vgetq_lane_f32(v4, 1) + 
           vgetq_lane_f32(v4, 2) + vgetq_lane_f32(v4, 3);
#else
    /* Generic fallback */
    float* f1 = (float*)&v1;
    float* f2 = (float*)&v2;
    float* f3 = (float*)&v3;
    float* f4 = (float*)&v4;
    
    for (int i = 0; i < 4; i++) {
        sum += f1[i] + f2[i] + f3[i] + f4[i];
    }
#endif
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOOPT v4sf pattern_d_conditional_vector(v4sf a, v4sf b, v4sf c, v4sf d,
                                        v4sf e, v4sf f, v4sf g, v4sf h)
{
    /* Complex conditional vector operations that may expand to many RTL operands */
#ifdef __SSE__
    /* Multiple comparisons and blends */
    v4sf cmp1 = a > b;
    v4sf cmp2 = c < d;
    v4sf cmp3 = e == f;
    v4sf cmp4 = g != h;
    
    /* Combine comparisons with logical operations */
    v4sf mask1 = cmp1 & cmp2;
    v4sf mask2 = cmp3 | cmp4;
    v4sf final_mask = mask1 ^ mask2;
    
    /* Use the mask for conditional selection */
    v4sf result = (a * final_mask) + (b * (~final_mask));
    result = result + (c * mask1) - (d * mask2);
    
    return result;
#elif defined(__ARM_NEON)
    /* ARM NEON comparisons */
    uint32x4_t cmp1 = vcgtq_f32(a, b);
    uint32x4_t cmp2 = vcltq_f32(c, d);
    uint32x4_t cmp3 = vceqq_f32(e, f);
    uint32x4_t cmp4 = vmvnq_u32(vceqq_f32(g, h));  // not equal
    
    uint32x4_t mask1 = vandq_u32(cmp1, cmp2);
    uint32x4_t mask2 = vorrq_u32(cmp3, cmp4);
    uint32x4_t final_mask = veorq_u32(mask1, mask2);
    
    v4sf result = vbslq_f32(final_mask, a, b);
    v4sf temp1 = vbslq_f32(mask1, c, vdupq_n_f32(0.0f));
    v4sf temp2 = vbslq_f32(mask2, d, vdupq_n_f32(0.0f));
    
    result = vaddq_f32(result, temp1);
    result = vsubq_f32(result, temp2);
    
    return result;
#else
    return a + b + c + d + e + f + g + h;
#endif
}

/* Pattern E: Inline assembly with exactly 10-11 operands */
NOOPT int pattern_e_inline_asm_many_operands(int a, int b, int c, int d, int e,
                                             int f, int g, int h, int i, int j)
{
    int result1, result2;
    
    /* Inline assembly with exactly 10 input/output operands */
    asm volatile (
        /* Complex operation using all 10 input registers */
        "add %[res1], %[a], %[b]\n\t"
        "add %[res1], %[res1], %[c]\n\t"
        "add %[res1], %[res1], %[d]\n\t"
        "add %[res1], %[res1], %[e]\n\t"
        "mul %[res2], %[f], %[g]\n\t"
        "add %[res2], %[res2], %[h]\n\t"
        "add %[res2], %[res2], %[i]\n\t"
        "add %[res2], %[res2], %[j]\n\t"
        "add %[res1], %[res1], %[res2]"
        
        : [res1] "=r" (result1), [res2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2;
}

/* Pattern F: Complex shuffle with immediate and multiple vector inputs */
NOOPT v4sf pattern_f_complex_shuffle(v4sf a, v4sf b, v4sf c, v4sf d,
                                     int imm1, int imm2, int imm3, int imm4)
{
#ifdef __SSE__
    /* Chain multiple shuffle operations - each shuffle may expand to many RTL operands */
    v4sf result = a;
    
    /* Use different shuffle patterns */
    result = __builtin_ia32_shufps(result, b, imm1);
    result = __builtin_ia32_shufps(result, c, imm2);
    result = __builtin_ia32_shufps(result, d, imm3);
    
    /* Additional arithmetic mixing */
    result = result * 2.0f - 1.0f;
    
    /* Another shuffle with the result */
    result = __builtin_ia32_shufps(result, a, imm4);
    
    return result;
#elif defined(__ARM_NEON)
    /* ARM NEON equivalent using vrev, vext, vtbl, etc. */
    v4sf result = a;
    
    /* Use vext for byte extraction */
    result = vextq_f32(result, b, 1);
    result = vextq_f32(result, c, 2);
    result = vextq_f32(result, d, 3);
    
    /* Additional operations */
    result = vmulq_n_f32(result, 2.0f);
    result = vsubq_f32(result, vdupq_n_f32(1.0f));
    
    return result;
#else
    return a + b + c + d;
#endif
}

/* Main test driver that exercises all patterns */
int main(int argc, char** argv)
{
    /* Use argc to add runtime variability and prevent constant folding */
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Initialize test data with some variability */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf vec5 = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf vec6 = {21.0f, 22.0f, 23.0f, 24.0f};
    v4sf vec7 = {25.0f, 26.0f, 27.0f, 28.0f};
    v4sf vec8 = {29.0f, 30.0f, 31.0f, 32.0f};
    
    float scalar1 = 1.0f + (rand() % 100) * 0.01f;
    float scalar2 = 2.0f + (rand() % 100) * 0.01f;
    float scalar3 = 3.0f + (rand() % 100) * 0.01f;
    float scalar4 = 4.0f + (rand() % 100) * 0.01f;
    float scalar5 = 5.0f + (rand() % 100) * 0.01f;
    float scalar6 = 6.0f + (rand() % 100) * 0.01f;
    float scalar7 = 7.0f + (rand() % 100) * 0.01f;
    float scalar8 = 8.0f + (rand() % 100) * 0.01f;
    float scalar9 = 9.0f + (rand() % 100) * 0.01f;
    float scalar10 = 10.0f + (rand() % 100) * 0.01f;
    float scalar11 = 11.0f + (rand() % 100) * 0.01f;
    float scalar12 = 12.0f + (rand() % 100) * 0.01f;
    float scalar13 = 13.0f + (rand() % 100) * 0.01f;
    float scalar14 = 14.0f + (rand() % 100) * 0.01f;
    float scalar15 = 15.0f + (rand() % 100) * 0.01f;
    
    int int1 = rand() % 100;
    int int2 = rand() % 100;
    int int3 = rand() % 100;
    int int4 = rand() % 100;
    int int5 = rand() % 100;
    int int6 = rand() % 100;
    int int7 = rand() % 100;
    int int8 = rand() % 100;
    int int9 = rand() % 100;
    int int10 = rand() % 100;
    
    /* Execute all patterns to ensure they're compiled */
    v4sf result_a = pattern_a_blend_many_operands(vec1, vec2, vec3, vec4,
                                                  vec5, vec6, vec7, vec8,
                                                  int1, int2, int3, int4);
    
    float result_b = pattern_b_fma_chain(scalar1, scalar2, scalar3, scalar4,
                                         scalar5, scalar6, scalar7, scalar8,
                                         scalar9, scalar10, scalar11, scalar12,
                                         scalar13, scalar14, scalar15);
    
    float result_c = pattern_c_vector_reduction(vec1, vec2, vec3, vec4);
    
    v4sf result_d = pattern_d_conditional_vector(vec1, vec2, vec3, vec4,
                                                 vec5, vec6, vec7, vec8);
    
    int result_e = pattern_e_inline_asm_many_operands(int1, int2, int3, int4,
                                                      int5, int6, int7, int8,
                                                      int9, int10);
    
    v4sf result_f = pattern_f_complex_shuffle(vec1, vec2, vec3, vec4,
                                              int1 & 0xFF, int2 & 0xFF,
                                              int3 & 0xFF, int4 & 0xFF);
    
    /* Use results to compute a checksum to prevent dead code elimination */
    float checksum = 0.0f;
    
    /* Extract values from vector results */
#ifdef __SSE__
    checksum += ((float*)&result_a)[0] + ((float*)&result_a)[1] + 
                ((float*)&result_a)[2] + ((float*)&result_a)[3];
    checksum += ((float*)&result_d)[0] + ((float*)&result_d)[1] + 
                ((float*)&result_d)[2] + ((float*)&result_d)[3];
    checksum += ((float*)&result_f)[0] + ((float*)&result_f)[1] + 
                ((float*)&result_f)[2] + ((float*)&result_f)[3];
#elif defined(__ARM_NEON)
    checksum += vgetq_lane_f32(result_a, 0) + vgetq_lane_f32(result_a, 1) +
                vgetq_lane_f32(result_a, 2) + vgetq_lane_f32(result_a, 3);
    checksum += vgetq_lane_f32(result_d, 0) + vgetq_lane_f32(result_d, 1) +
                vgetq_lane_f32(result_d, 2) + vgetq_lane_f32(result_d, 3);
    checksum += vgetq_lane_f32(result_f, 0) + vgetq_lane_f32(result_f, 1) +
                vgetq_lane_f32(result_f, 2) + vgetq_lane_f32(result_f, 3);
#endif
    
    checksum += result_b + result_c + (float)result_e;
    
    /* Store to volatile sink to ensure all computations are kept */
    g_volatile_sink = (int)checksum;
    
    /* Return checksum as program output */
    printf("Checksum: %f\n", checksum);
    return (int)checksum;
}
