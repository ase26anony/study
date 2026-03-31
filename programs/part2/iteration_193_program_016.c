/* test_optabs_coverage.c
 * This program is designed to trigger GCC's RTL expansion with 10-11 operands
 * to cover specific switch cases in optabs.cc lines 8254-8263
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Define vector types for portability */
#if defined(__SSE__) || defined(__x86_64__) || defined(__i386__)
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
#define USE_X86_INTRINSICS 1
#elif defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>
typedef float32x4_t v4sf;
typedef int32x4_t v4si;
#define USE_ARM_INTRINSICS 1
#else
/* Fallback definitions */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
#endif

/* Prevent optimization and ensure expansion */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Pattern A: Complex vector blend with many operands */
NOINLINE static v4sf pattern_a(v4sf a, v4sf b, v4sf c, v4sf d, 
                               v4si mask1, v4si mask2, v4si mask3) {
#if defined(USE_X86_INTRINSICS) && defined(__SSE4_1__)
    /* __builtin_ia32_blendps takes 2 vectors + immediate, but we'll create
     * a complex blend pattern that expands to many operands */
    v4sf t1 = __builtin_ia32_blendps(a, b, 0x5);  /* 0101 */
    v4sf t2 = __builtin_ia32_blendps(c, d, 0xA);  /* 1010 */
    v4sf t3 = __builtin_ia32_blendps(t1, t2, 0x3); /* 0011 */
    
    /* Create complex conditional blend using comparisons */
    v4sf cmp1 = __builtin_ia32_cmppslt(a, b, 0x1); /* LT */
    v4sf cmp2 = __builtin_ia32_cmppslt(c, d, 0x1);
    v4sf and_mask = __builtin_ia32_andps(cmp1, cmp2);
    v4sf or_mask = __builtin_ia32_orps(cmp1, cmp2);
    
    /* Final blend with many intermediate values */
    v4sf result = __builtin_ia32_blendvps(t1, t2, and_mask);
    result = __builtin_ia32_blendvps(result, t3, or_mask);
    
    return result;
#else
    /* Portable fallback */
    return a + b + c + d;
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE static float pattern_b(float a, float b, float c, float d,
                                float e, float f, float g, float h) {
#if defined(__FMA__) || defined(__FMA4__)
    /* Chain of FMA operations - expands to many operands during RTL gen */
    float t1 = __builtin_fma(a, b, c);
    float t2 = __builtin_fma(d, e, f);
    float t3 = __builtin_fma(g, h, t1);
    float t4 = __builtin_fma(t1, t2, t3);
    float t5 = __builtin_fma(t2, t3, t4);
    float t6 = __builtin_fma(t3, t4, t5);
    float t7 = __builtin_fma(t4, t5, t6);
    float t8 = __builtin_fma(t5, t6, t7);
    
    return __builtin_fma(t6, t7, t8);
#else
    return a * b + c * d + e * f + g * h;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE static float pattern_c(v4sf v) {
    /* Manually extract and sum all elements - creates many extract operations */
    float sum = 0.0f;
    
#if defined(USE_X86_INTRINSICS)
    /* Each extract creates multiple operands */
    sum += __builtin_ia32_vec_ext_v4sf(v, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v, 1);
    sum += __builtin_ia32_vec_ext_v4sf(v, 2);
    sum += __builtin_ia32_vec_ext_v4sf(v, 3);
    
    /* Additional operations to increase operand count */
    v4sf v2 = v + v;
    sum += __builtin_ia32_vec_ext_v4sf(v2, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v2, 1);
#elif defined(USE_ARM_INTRINSICS)
    sum += vgetq_lane_f32(v, 0);
    sum += vgetq_lane_f32(v, 1);
    sum += vgetq_lane_f32(v, 2);
    sum += vgetq_lane_f32(v, 3);
#else
    /* Portable extraction */
    float* ptr = (float*)&v;
    sum = ptr[0] + ptr[1] + ptr[2] + ptr[3];
#endif
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE static v4sf pattern_d(v4sf a, v4sf b, v4sf c, v4sf d) {
#if defined(USE_X86_INTRINSICS)
    /* Multiple comparisons and blends */
    v4sf cmp_ab = __builtin_ia32_cmpps(a, b, 0x1);  /* LT */
    v4sf cmp_cd = __builtin_ia32_cmpps(c, d, 0x1);
    v4sf cmp_ac = __builtin_ia32_cmpps(a, c, 0x1);
    v4sf cmp_bd = __builtin_ia32_cmpps(b, d, 0x1);
    
    /* Logical operations on comparison results */
    v4sf mask1 = __builtin_ia32_andps(cmp_ab, cmp_cd);
    v4sf mask2 = __builtin_ia32_orps(cmp_ac, cmp_bd);
    v4sf mask3 = __builtin_ia32_xorps(mask1, mask2);
    
    /* Multiple blends based on different masks */
    v4sf t1 = __builtin_ia32_blendvps(a, b, mask1);
    v4sf t2 = __builtin_ia32_blendvps(c, d, mask2);
    v4sf result = __builtin_ia32_blendvps(t1, t2, mask3);
    
    return result;
#else
    return a + b + c + d;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE static int pattern_e(int a, int b, int c, int d, int e,
                              int f, int g, int h, int i, int j) {
    int result;
    
    /* Inline asm with 10 input operands + 1 output = 11 total operands */
    __asm__ volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "imul %[a], %[c]\n\t"
        "imul %[e], %[g]\n\t"
        "add %[a], %[e]\n\t"
        "mov %[out], %[a]"
        : [out] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result;
}

/* Pattern F: Complex shuffle with variable mask */
NOINLINE static v4sf pattern_f(v4sf a, v4sf b, int mask1, int mask2,
                               int mask3, int mask4) {
#if defined(USE_X86_INTRINSICS) && defined(__SSE__)
    /* Multiple shuffles with different masks */
    v4sf s1 = __builtin_ia32_shufps(a, b, mask1);
    v4sf s2 = __builtin_ia32_shufps(b, a, mask2);
    v4sf s3 = __builtin_ia32_shufps(s1, s2, mask3);
    v4sf s4 = __builtin_ia32_shufps(s2, s1, mask4);
    
    /* Blend the results */
    v4sf result = __builtin_ia32_blendps(s3, s4, 0x9); /* 1001 */
    
    return result;
#else
    return a + b;
#endif
}

/* Main test driver */
int main(int argc, char** argv) {
    volatile float checksum = 0.0f;
    
    /* Initialize test data with some variability */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec_c = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec_d = {13.0f, 14.0f, 15.0f, 16.0f};
    v4si mask1 = {0xFFFFFFFF, 0, 0xFFFFFFFF, 0};
    v4si mask2 = {0, 0xFFFFFFFF, 0, 0xFFFFFFFF};
    v4si mask3 = {0xFFFFFFFF, 0xFFFFFFFF, 0, 0};
    
    /* Use argc to select different patterns, ensuring all get compiled */
    switch (argc % 6) {
        case 0:
            checksum += ((float*)&pattern_a(vec_a, vec_b, vec_c, vec_d, 
                                           mask1, mask2, mask3))[0];
            break;
        case 1:
            checksum += pattern_b(1.1f, 2.2f, 3.3f, 4.4f, 
                                 5.5f, 6.6f, 7.7f, 8.8f);
            break;
        case 2:
            checksum += pattern_c(vec_a);
            break;
        case 3:
            checksum += ((float*)&pattern_d(vec_a, vec_b, vec_c, vec_d))[0];
            break;
        case 4:
            checksum += pattern_e(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
            break;
        case 5:
            checksum += ((float*)&pattern_f(vec_a, vec_b, 0x1B, 0x27, 0x4E, 0x8D))[0];
            break;
    }
    
    /* Additional calls to ensure all patterns are used */
    if (argc > 1) {
        checksum += pattern_b(2.1f, 3.2f, 4.3f, 5.4f, 6.5f, 7.6f, 8.7f, 9.8f);
        checksum += pattern_e(11, 12, 13, 14, 15, 16, 17, 18, 19, 20);
    }
    
    printf("Checksum: %f\n", checksum);
    return (int)checksum;
}
