/* test_optabs.c - Test program to cover 10/11 operand switch cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of critical functions */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Vector type definitions for various architectures */
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
typedef float32x4_t v4sf_neon;
#endif

/* Global volatile to prevent dead code elimination */
volatile int g_result = 0;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE
int test_pattern_a(int argc) {
#ifdef __SSE__
    v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf b = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf c = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf d = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Complex shuffle pattern that may expand to many operands */
    v4sf result;
    if (argc > 1) {
        /* __builtin_ia32_shufps takes 3 operands but expands to many RTL ops */
        result = __builtin_ia32_shufps(a, b, (argc % 4) | ((argc % 3) << 2) | 
                                       ((argc % 2) << 4) | ((argc % 5) << 6));
    } else {
        /* Alternative shuffle with different mask */
        result = __builtin_ia32_shufps(c, d, 0x1B); /* 0b00011011 */
    }
    
    /* Chain operations to increase operand count */
    v4sf temp = __builtin_ia32_shufps(result, a, 0xE4);
    v4sf final = __builtin_ia32_shufps(temp, b, 0x1B);
    
    /* Use result to prevent elimination */
    return (int)final[0] + (int)final[1] + (int)final[2] + (int)final[3];
#else
    return argc;
#endif
}

/* Pattern B: Fused multiply-add chain */
NOINLINE
float test_pattern_b(float a, float b, float c, float d, float e, 
                     float f, float g, float h, float i, float j) {
#ifdef __FP_FAST_FMA
    /* Chain of FMA operations creating deep expression tree */
    float t1 = __builtin_fma(a, b, c);
    float t2 = __builtin_fma(d, e, f);
    float t3 = __builtin_fma(g, h, i);
    float t4 = __builtin_fma(t1, t2, t3);
    float t5 = __builtin_fma(t4, j, a);
    float t6 = __builtin_fma(b, t5, c);
    float t7 = __builtin_fma(d, t6, e);
    float t8 = __builtin_fma(f, t7, g);
    float t9 = __builtin_fma(h, t8, i);
    return __builtin_fma(j, t9, t1);
#else
    /* Fallback for targets without FMA */
    return a * b + c * d + e * f + g * h + i * j;
#endif
}

/* Pattern C: Vector extraction and manual reduction */
NOINLINE
int test_pattern_c(int argc) {
#ifdef __SSE__
    v4si vec1 = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec2 = {argc + 4, argc + 5, argc + 6, argc + 7};
    v4si vec3 = {argc + 8, argc + 9, argc + 10, argc + 11};
    v4si vec4 = {argc + 12, argc + 13, argc + 14, argc + 15};
    
    /* Manual horizontal addition with many extract operations */
    int sum = 0;
    
    /* Extract each element - each extract may become multiple operands */
    sum += ((int*)&vec1)[0];  /* Simulating extract */
    sum += ((int*)&vec1)[1];
    sum += ((int*)&vec1)[2];
    sum += ((int*)&vec1)[3];
    
    sum += ((int*)&vec2)[0];
    sum += ((int*)&vec2)[1];
    sum += ((int*)&vec2)[2];
    sum += ((int*)&vec2)[3];
    
    sum += ((int*)&vec3)[0];
    sum += ((int*)&vec3)[1];
    sum += ((int*)&vec3)[2];
    sum += ((int*)&vec3)[3];
    
    sum += ((int*)&vec4)[0];
    sum += ((int*)&vec4)[1];
    sum += ((int*)&vec4)[2];
    sum += ((int*)&vec4)[3];
    
    return sum;
#else
    return argc * 16 + 120; /* 0+1+2+...+15 = 120 */
#endif
}

/* Pattern D: Vector conditional select with complex mask */
NOINLINE
v4sf test_pattern_d(v4sf a, v4sf b, v4sf c, v4sf d, int mask) {
#ifdef __SSE__
    /* Create comparison mask from multiple operations */
    v4sf cmp1 = __builtin_ia32_cmpeqps(a, b);
    v4sf cmp2 = __builtin_ia32_cmpgtps(c, d);
    v4sf cmp3 = __builtin_ia32_cmpltps(a, c);
    
    /* Combine masks with logical operations */
    v4sf mask1 = __builtin_ia32_andps(cmp1, cmp2);
    v4sf mask2 = __builtin_ia32_orps(cmp3, cmp1);
    v4sf final_mask = __builtin_ia32_andnps(mask1, mask2);
    
    /* Conditional blend based on mask */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        if (((int*)&final_mask)[i]) {
            ((float*)&result)[i] = ((float*)&a)[i];
        } else {
            ((float*)&result)[i] = ((float*)&b)[i];
        }
    }
    
    return result;
#else
    return a;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE
int test_pattern_asm(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j, int k) {
    int result1, result2, result3;
    
    /* Inline asm with 11 operands (6 inputs, 3 outputs, 2 clobbers) */
    asm volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "imul %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "mov %[k], %[res1]\n\t"
        "lea (%[res1], %[a], 2), %[res2]\n\t"
        "sub %[res2], %[res3]"
        : [res1] "=r" (result1),
          [res2] "=r" (result2),
          [res3] "=r" (result3)
        : [a] "r" (a),
          [b] "r" (b),
          [c] "r" (c),
          [d] "r" (d),
          [e] "r" (e),
          [f] "r" (f),
          [g] "r" (g),
          [h] "r" (h),
          [i] "r" (i),
          [j] "r" (j),
          [k] "r" (k)
        : "cc", "memory"
    );
    
    return result1 + result2 + result3;
}

/* Pattern F: AVX blend with dynamic mask (potential for many operands) */
NOINLINE
v8sf test_pattern_f(v8sf a, v8sf b, v8sf c, v8sf d, int mask) {
#ifdef __AVX__
    /* Complex blend operation that may expand to many RTL operands */
    v8sf ab_blend = __builtin_ia32_blendps256(a, b, mask & 0xFF);
    v8sf cd_blend = __builtin_ia32_blendps256(c, d, (mask >> 8) & 0xFF);
    
    /* Further operations to increase complexity */
    v8sf result = __builtin_ia32_addps256(ab_blend, cd_blend);
    result = __builtin_ia32_mulps256(result, a);
    result = __builtin_ia32_subps256(result, b);
    
    return result;
#else
    return a;
#endif
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Test Pattern A - Vector shuffle */
    result += test_pattern_a(argc);
    
    /* Test Pattern B - FMA chain */
    float fma_result = test_pattern_b(1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                                      6.0f, 7.0f, 8.0f, 9.0f, 10.0f);
    result += (int)fma_result;
    
    /* Test Pattern C - Vector extraction */
    result += test_pattern_c(argc);
    
    /* Test Pattern D - Vector conditional select */
#ifdef __SSE__
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf v4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf vec_result = test_pattern_d(v1, v2, v3, v4, argc);
    result += (int)vec_result[0];
#endif
    
    /* Test Pattern E - Inline assembly with 11 operands */
    result += test_pattern_asm(argc, argc+1, argc+2, argc+3, argc+4,
                               argc+5, argc+6, argc+7, argc+8, argc+9, argc+10);
    
    /* Store to volatile global to ensure all computations are used */
    g_result = result;
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
