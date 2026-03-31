/* test_optabs.c - Test program to cover 10/11 operand switch cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>

/* Define vector types for portability */
#if defined(__SSE__) || defined(__x86_64__) || defined(__i386__)
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
#define USE_X86 1
#endif

#if defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>
typedef float32x4_t v4sf;
typedef int32x4_t v4si;
#define USE_ARM 1
#endif

/* Fallback dummy implementations */
#ifndef USE_X86
#ifndef USE_ARM
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
#endif
#endif

/* Prevent optimization and ensure expansion */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Pattern A: Complex vector blend with many operands */
NOINLINE static v4sf pattern_a(v4sf a, v4sf b, v4sf c, v4sf d, 
                               int m1, int m2, int m3, int m4) {
#if defined(USE_X86) && defined(__SSE4_1__)
    /* __builtin_ia32_blendps takes 3 operands but expands to many RTL ops */
    v4sf t1 = __builtin_ia32_blendps(a, b, m1);
    v4sf t2 = __builtin_ia32_blendps(c, d, m2);
    v4sf t3 = __builtin_ia32_blendps(t1, t2, m3);
    return __builtin_ia32_blendps(t3, a, m4);
#elif defined(USE_ARM)
    /* Use NEON selection with mask computed from multiple operands */
    uint32x4_t mask1 = vdupq_n_u32(m1);
    uint32x4_t mask2 = vdupq_n_u32(m2);
    v4sf t1 = vbslq_f32(mask1, a, b);
    v4sf t2 = vbslq_f32(mask2, c, d);
    uint32x4_t mask3 = vdupq_n_u32(m3);
    v4sf t3 = vbslq_f32(mask3, t1, t2);
    uint32x4_t mask4 = vdupq_n_u32(m4);
    return vbslq_f32(mask4, t3, a);
#else
    /* Fallback: manual blend */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        float* r = (float*)&result;
        float* pa = (float*)&a;
        float* pb = (float*)&b;
        float* pc = (float*)&c;
        float* pd = (float*)&d;
        r[i] = (m1 & (1 << i)) ? pa[i] : pb[i];
        float temp = (m2 & (1 << i)) ? pc[i] : pd[i];
        r[i] = (m3 & (1 << i)) ? r[i] : temp;
        r[i] = (m4 & (1 << i)) ? r[i] : pa[i];
    }
    return result;
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE static float pattern_b(float a, float b, float c, float d,
                                float e, float f, float g, float h,
                                float i, float j) {
#if defined(__FMA__) || defined(__FMA4__)
    /* Chain of FMA operations - expands to many operands */
    float t1 = __builtin_fma(a, b, c);
    float t2 = __builtin_fma(d, e, f);
    float t3 = __builtin_fma(g, h, i);
    float t4 = __builtin_fma(t1, t2, t3);
    return __builtin_fma(t4, j, a + b + c + d);
#else
    /* Manual FMA simulation */
    return ((a * b) + c) * ((d * e) + f) + 
           ((g * h) + i) * j + a + b + c + d;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE static float pattern_c(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    float sum = 0.0f;
    
    /* Extract and sum all elements - creates many extract operations */
#if defined(USE_X86)
    sum += __builtin_ia32_vec_ext_v4sf(v1, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v1, 1);
    sum += __builtin_ia32_vec_ext_v4sf(v1, 2);
    sum += __builtin_ia32_vec_ext_v4sf(v1, 3);
    
    sum += __builtin_ia32_vec_ext_v4sf(v2, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v2, 1);
    sum += __builtin_ia32_vec_ext_v4sf(v2, 2);
    sum += __builtin_ia32_vec_ext_v4sf(v2, 3);
    
    sum += __builtin_ia32_vec_ext_v4sf(v3, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v3, 1);
    sum += __builtin_ia32_vec_ext_v4sf(v3, 2);
    sum += __builtin_ia32_vec_ext_v4sf(v3, 3);
    
    sum += __builtin_ia32_vec_ext_v4sf(v4, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v4, 1);
    sum += __builtin_ia32_vec_ext_v4sf(v4, 2);
    sum += __builtin_ia32_vec_ext_v4sf(v4, 3);
#elif defined(USE_ARM)
    sum += vgetq_lane_f32(v1, 0);
    sum += vgetq_lane_f32(v1, 1);
    sum += vgetq_lane_f32(v1, 2);
    sum += vgetq_lane_f32(v1, 3);
    
    sum += vgetq_lane_f32(v2, 0);
    sum += vgetq_lane_f32(v2, 1);
    sum += vgetq_lane_f32(v2, 2);
    sum += vgetq_lane_f32(v2, 3);
    
    sum += vgetq_lane_f32(v3, 0);
    sum += vgetq_lane_f32(v3, 1);
    sum += vgetq_lane_f32(v3, 2);
    sum += vgetq_lane_f32(v3, 3);
    
    sum += vgetq_lane_f32(v4, 0);
    sum += vgetq_lane_f32(v4, 1);
    sum += vgetq_lane_f32(v4, 2);
    sum += vgetq_lane_f32(v4, 3);
#else
    /* Manual extraction */
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
NOINLINE static v4sf pattern_d(v4sf a, v4sf b, v4sf c, v4sf d,
                               v4sf e, v4sf f) {
#if defined(USE_X86)
    /* Multiple comparisons and blends */
    v4sf cmp1 = __builtin_ia32_cmpps(a, b, 1);  /* LT */
    v4sf cmp2 = __builtin_ia32_cmpps(c, d, 1);  /* LT */
    v4sf cmp3 = __builtin_ia32_cmpps(e, f, 1);  /* LT */
    
    v4sf t1 = __builtin_ia32_andps(cmp1, cmp2);
    v4sf t2 = __builtin_ia32_orps(cmp3, a);
    v4sf mask = __builtin_ia32_andnps(t1, t2);
    
    /* Complex blend based on mask */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        int* m = (int*)&mask;
        float* ra = (float*)&a;
        float* rb = (float*)&b;
        float* rc = (float*)&c;
        float* res = (float*)&result;
        res[i] = (m[i] & 1) ? ra[i] : 
                ((m[i] & 2) ? rb[i] : rc[i]);
    }
    return result;
#elif defined(USE_ARM)
    uint32x4_t cmp1 = vcltq_f32(a, b);
    uint32x4_t cmp2 = vcltq_f32(c, d);
    uint32x4_t cmp3 = vcltq_f32(e, f);
    
    uint32x4_t t1 = vandq_u32(cmp1, cmp2);
    uint32x4_t t2 = vorrq_u32(cmp3, vreinterpretq_u32_f32(a));
    uint32x4_t mask = vbicq_u32(t2, t1);
    
    return vbslq_f32(mask, a, vbslq_f32(cmp2, b, c));
#else
    /* Manual implementation */
    v4sf result = a;
    float* ra = (float*)&a;
    float* rb = (float*)&b;
    float* rc = (float*)&c;
    float* rd = (float*)&d;
    float* re = (float*)&e;
    float* rf = (float*)&f;
    float* res = (float*)&result;
    
    for (int i = 0; i < 4; i++) {
        int cond1 = (ra[i] < rb[i]) ? 1 : 0;
        int cond2 = (rc[i] < rd[i]) ? 1 : 0;
        int cond3 = (re[i] < rf[i]) ? 1 : 0;
        
        if (cond1 && cond2) {
            res[i] = ra[i];
        } else if (cond3) {
            res[i] = rb[i];
        } else {
            res[i] = rc[i];
        }
    }
    return result;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE static int pattern_e(int a, int b, int c, int d, int e,
                              int f, int g, int h, int i, int j) {
    int result1, result2;
    
    /* Inline asm with many operands - directly creates RTL with many operands */
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
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2;
}

/* Main test function with runtime variability */
int main(int argc, char** argv) {
    volatile int seed = argc;
    float checksum = 0.0f;
    
    /* Initialize test vectors with volatile to prevent constant folding */
    volatile float f1 = 1.0f + seed;
    volatile float f2 = 2.0f + seed;
    volatile float f3 = 3.0f + seed;
    volatile float f4 = 4.0f + seed;
    volatile float f5 = 5.0f + seed;
    volatile float f6 = 6.0f + seed;
    volatile float f7 = 7.0f + seed;
    volatile float f8 = 8.0f + seed;
    volatile float f9 = 9.0f + seed;
    volatile float f10 = 10.0f + seed;
    
    /* Create vector values */
    v4sf v1 = {f1, f2, f3, f4};
    v4sf v2 = {f5, f6, f7, f8};
    v4sf v3 = {f9, f10, f1, f2};
    v4sf v4 = {f3, f4, f5, f6};
    v4sf v5 = {f7, f8, f9, f10};
    v4sf v6 = {f1, f3, f5, f7};
    
    /* Execute all patterns based on argc to ensure all are compiled */
    if (argc > 1) {
        /* Pattern A: Complex blend */
        v4sf r1 = pattern_a(v1, v2, v3, v4, 0xF, 0xA, 0x5, 0x3);
        checksum += ((float*)&r1)[0] + ((float*)&r1)[1];
        
        /* Pattern B: FMA chain */
        float r2 = pattern_b(f1, f2, f3, f4, f5, f6, f7, f8, f9, f10);
        checksum += r2;
        
        /* Pattern C: Vector reduction */
        float r3 = pattern_c(v1, v2, v3, v4);
        checksum += r3;
        
        /* Pattern D: Conditional vector ops */
        v4sf r4 = pattern_d(v1, v2, v3, v4, v5, v6);
        checksum += ((float*)&r4)[2] + ((float*)&r4)[3];
        
        /* Pattern E: Inline asm with many operands */
        int r5 = pattern_e(seed, seed+1, seed+2, seed+3, seed+4,
                          seed+5, seed+6, seed+7, seed+8, seed+9);
        checksum += r5;
    } else {
        /* Simpler path but still exercises some patterns */
        checksum = pattern_b(f1, f2, f3, f4, f5, f6, f7, f8, f9, f10);
    }
    
    /* Use checksum to prevent dead code elimination */
    printf("Result: %f\n", checksum);
    return (checksum > 0) ? 0 : 1;
}
