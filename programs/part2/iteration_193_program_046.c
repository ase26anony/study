/* test_optabs.c - Test program to cover 10/11 operand switch cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types for portability */
#ifdef __SSE__
#include <xmmintrin.h>
typedef __m128 v4sf;
typedef __m128i v4si;
#else
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
typedef float32x4_t neon_v4sf;
#endif

/* Prevent optimization and ensure expansion */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Volatile sink to prevent elimination */
static volatile int sink;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d, 
                                int imm1, int imm2, int imm3, int imm4) {
    /* Create a complex shuffle pattern that may expand to many operands */
    v4sf t1, t2, t3, t4;
    
#ifdef __SSE__
    t1 = _mm_shuffle_ps(a, b, imm1);
    t2 = _mm_shuffle_ps(c, d, imm2);
    t3 = _mm_shuffle_ps(t1, t2, imm3);
    t4 = _mm_shuffle_ps(t3, a, imm4);
#else
    /* Portable fallback using GCC vector extensions */
    t1 = __builtin_shuffle(a, b, (v4si){0,1,2,3});
    t2 = __builtin_shuffle(c, d, (v4si){0,1,2,3});
    t3 = __builtin_shuffle(t1, t2, (v4si){0,1,2,3});
    t4 = __builtin_shuffle(t3, a, (v4si){0,1,2,3});
#endif
    
    return t4;
}

/* Pattern B: Fused multiply-add chain */
NOINLINE v4sf pattern_b_fma_chain(v4sf a, v4sf b, v4sf c, 
                                  v4sf d, v4sf e, v4sf f) {
    v4sf res;
    
#ifdef __FMA__
    /* Chain of FMA operations - may expand to many operands */
    res = __builtin_fma(a, b, c);
    res = __builtin_fma(d, e, res);
    res = __builtin_fma(f, a, res);
    res = __builtin_fma(b, c, res);
    res = __builtin_fma(d, e, res);
#else
    /* Manual FMA emulation */
    res = a * b + c;
    res = d * e + res;
    res = f * a + res;
    res = b * c + res;
    res = d * e + res;
#endif
    
    return res;
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE float pattern_c_vector_reduce(v4sf v0, v4sf v1, v4sf v2, v4sf v3) {
    /* Extract each lane manually - creates many extract operations */
    float sum = 0.0f;
    
    /* Extract 16 lanes total */
    for (int i = 0; i < 4; i++) {
#ifdef __SSE__
        sum += ((float*)&v0)[i];
        sum += ((float*)&v1)[i];
        sum += ((float*)&v2)[i];
        sum += ((float*)&v3)[i];
#else
        sum += v0[i];
        sum += v1[i];
        sum += v2[i];
        sum += v3[i];
#endif
    }
    
    return sum;
}

/* Pattern D: Conditional vector operations */
NOINLINE v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c, 
                                           v4sf d, v4sf mask1, v4sf mask2) {
    v4sf res;
    
#ifdef __SSE__
    /* Complex conditional logic with multiple masks */
    v4sf cmp1 = _mm_cmplt_ps(a, b);
    v4sf cmp2 = _mm_cmpgt_ps(c, d);
    v4sf mask = _mm_and_ps(cmp1, cmp2);
    mask = _mm_or_ps(mask, mask1);
    mask = _mm_andnot_ps(mask2, mask);
    
    /* Blend based on complex mask */
    res = _mm_blendv_ps(a, b, mask);
    res = _mm_blendv_ps(res, c, cmp1);
    res = _mm_blendv_ps(res, d, cmp2);
#else
    /* Portable version */
    v4sf cmp1 = a < b;
    v4sf cmp2 = c > d;
    v4sf mask = cmp1 & cmp2;
    mask = mask | mask1;
    mask = ~mask2 & mask;
    
    res = mask ? a : b;
    v4sf temp = cmp1 ? res : c;
    res = cmp2 ? temp : d;
#endif
    
    return res;
}

/* Pattern E: Inline assembly with many operands */
NOINLINE void pattern_e_multi_operand_asm(v4sf *out, v4sf in1, v4sf in2,
                                          v4sf in3, v4sf in4, v4sf in5,
                                          v4sf in6, v4sf in7, v4sf in8) {
    /* Inline assembly with 10 operands (output + 8 inputs + clobber) */
    __asm__ volatile (
        "# Complex multi-operand assembly pattern\n"
        : "=m" (*out)
        : "x" (in1), "x" (in2), "x" (in3), "x" (in4),
          "x" (in5), "x" (in6), "x" (in7), "x" (in8)
        : "memory"
    );
}

/* Pattern F: ARM NEON specific - multiple lane operations */
#ifdef __ARM_NEON
NOINLINE neon_v4sf pattern_f_neon_complex(neon_v4sf a, neon_v4sf b,
                                          neon_v4sf c, neon_v4sf d,
                                          int lane0, int lane1,
                                          int lane2, int lane3) {
    /* Complex NEON operations that may expand to many operands */
    neon_v4sf t1 = vmulq_f32(a, b);
    neon_v4sf t2 = vaddq_f32(c, d);
    
    /* Multiple lane extracts and inserts */
    float32_t l0 = vgetq_lane_f32(t1, lane0);
    float32_t l1 = vgetq_lane_f32(t2, lane1);
    float32_t l2 = vgetq_lane_f32(a, lane2);
    float32_t l3 = vgetq_lane_f32(b, lane3);
    
    neon_v4sf res = vsetq_lane_f32(l0 + l1, t1, 0);
    res = vsetq_lane_f32(l1 + l2, res, 1);
    res = vsetq_lane_f32(l2 + l3, res, 2);
    res = vsetq_lane_f32(l3 + l0, res, 3);
    
    /* Fused multiply accumulate */
    res = vmlaq_f32(res, c, d);
    res = vmlaq_f32(res, a, b);
    
    return res;
}
#endif

/* Main test driver */
int main(int argc, char **argv) {
    float checksum = 0.0f;
    
    /* Initialize test vectors with some data */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf v4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf v5 = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf v6 = {21.0f, 22.0f, 23.0f, 24.0f};
    v4sf mask1 = {0.0f, 1.0f, 0.0f, 1.0f};
    v4sf mask2 = {1.0f, 0.0f, 1.0f, 0.0f};
    
    /* Use argc to add runtime variability */
    int selector = argc > 1 ? atoi(argv[1]) % 5 : 0;
    
    /* Execute different patterns based on selector */
    switch (selector) {
        case 0: {
            /* Pattern A - Complex shuffle */
            v4sf res = pattern_a_shuffle(v1, v2, v3, v4, 0x1B, 0x27, 0x39, 0x4E);
            checksum += ((float*)&res)[0] + ((float*)&res)[1];
            break;
        }
        case 1: {
            /* Pattern B - FMA chain */
            v4sf res = pattern_b_fma_chain(v1, v2, v3, v4, v5, v6);
            checksum += ((float*)&res)[2] + ((float*)&res)[3];
            break;
        }
        case 2: {
            /* Pattern C - Vector reduction */
            float res = pattern_c_vector_reduce(v1, v2, v3, v4);
            checksum += res;
            break;
        }
        case 3: {
            /* Pattern D - Conditional select */
            v4sf res = pattern_d_conditional_select(v1, v2, v3, v4, mask1, mask2);
            checksum += ((float*)&res)[0] + ((float*)&res)[3];
            break;
        }
        case 4: {
            /* Pattern E - Multi-operand assembly */
            v4sf out;
            pattern_e_multi_operand_asm(&out, v1, v2, v3, v4, v5, v6, v1, v2);
            checksum += ((float*)&out)[0];
            break;
        }
#ifdef __ARM_NEON
        case 5: {
            /* Pattern F - NEON complex operations */
            neon_v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
            neon_v4sf b = {5.0f, 6.0f, 7.0f, 8.0f};
            neon_v4sf c = {9.0f, 10.0f, 11.0f, 12.0f};
            neon_v4sf d = {13.0f, 14.0f, 15.0f, 16.0f};
            neon_v4sf res = pattern_f_neon_complex(a, b, c, d, 0, 1, 2, 3);
            checksum += vgetq_lane_f32(res, 0) + vgetq_lane_f32(res, 1);
            break;
        }
#endif
    }
    
    /* Use checksum to prevent optimization */
    sink = (int)checksum;
    printf("Result: %f\n", checksum);
    
    return 0;
}
