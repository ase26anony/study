/* test_optabs.c - Coverage test for optabs.cc switch cases 10 and 11 */
#include <stdint.h>
#include <stdlib.h>

/* Define vector types for SSE/AVX and NEON */
#ifdef __SSE__
#include <xmmintrin.h>
typedef __m128 v4sf;
typedef __m128i v4si;
#else
/* Fallback definitions if SSE not available */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
typedef float32x4_t v4sf_neon;
typedef int32x4_t v4si_neon;
#endif

/* Prevent inlining to ensure expansion happens in this function */
__attribute__((noinline, noipa, used))
static v4sf pattern_a(v4sf a, v4sf b, v4sf c, v4sf d, 
                      v4sf e, v4sf f, v4sf g, v4sf h,
                      int imm1, int imm2, int imm3) {
    /* Pattern A: Complex vector shuffle/blend with many operands */
    /* This should expand to an RTL pattern with many operands */
#ifdef __SSE__
    v4sf t1 = _mm_shuffle_ps(a, b, imm1);
    v4sf t2 = _mm_shuffle_ps(c, d, imm2);
    v4sf t3 = _mm_shuffle_ps(e, f, imm3);
    v4sf t4 = _mm_blend_ps(t1, t2, 0x5);
    v4sf t5 = _mm_blend_ps(t3, g, 0x3);
    v4sf t6 = _mm_blend_ps(t4, t5, 0x9);
    v4sf res = _mm_add_ps(t6, h);
    return res;
#else
    /* Generic fallback */
    v4sf t1 = __builtin_shuffle(a, b, (v4si){imm1 & 3, (imm1 >> 2) & 3, 
                                             (imm1 >> 4) & 3, (imm1 >> 6) & 3});
    v4sf t2 = __builtin_shuffle(c, d, (v4si){imm2 & 3, (imm2 >> 2) & 3, 
                                             (imm2 >> 4) & 3, (imm2 >> 6) & 3});
    v4sf t3 = __builtin_shuffle(e, f, (v4si){imm3 & 3, (imm3 >> 2) & 3, 
                                             (imm3 >> 4) & 3, (imm3 >> 6) & 3});
    v4sf mask1 = (v4sf){1.0f, 0.0f, 1.0f, 0.0f}; /* 0x5 */
    v4sf mask2 = (v4sf){1.0f, 1.0f, 0.0f, 0.0f}; /* 0x3 */
    v4sf mask3 = (v4sf){1.0f, 0.0f, 0.0f, 1.0f}; /* 0x9 */
    v4sf t4 = __builtin_shufflevector(t1, t2, 0, 5, 2, 7);
    v4sf t5 = __builtin_shufflevector(t3, g, 0, 1, 6, 7);
    v4sf t6 = __builtin_shufflevector(t4, t5, 0, 5, 6, 3);
    return t6 + h;
#endif
}

__attribute__((noinline, noipa, used))
static v4sf pattern_b(v4sf a, v4sf b, v4sf c, v4sf d,
                      v4sf e, v4sf f, v4sf g, v4sf h,
                      v4sf i, v4sf j) {
    /* Pattern B: Fused multiply-add chain with many accumulators */
    /* Each FMA expands to multiple operands */
#ifdef __FMA__
    v4sf t1 = __builtin_fma(a, b, c);
    v4sf t2 = __builtin_fma(d, e, f);
    v4sf t3 = __builtin_fma(g, h, i);
    v4sf t4 = __builtin_fma(t1, t2, t3);
    return __builtin_fma(t4, j, t1);
#else
    /* Manual FMA emulation - still creates many operands */
    v4sf t1 = a * b + c;
    v4sf t2 = d * e + f;
    v4sf t3 = g * h + i;
    v4sf t4 = t1 * t2 + t3;
    return t4 * j + t1;
#endif
}

__attribute__((noinline, noipa, used))
static float pattern_c(v4sf a, v4sf b, v4sf c, v4sf d,
                       v4sf e, v4sf f, v4sf g, v4sf h) {
    /* Pattern C: Vector reduction with explicit scalarization */
    /* Each extract operation adds operands */
    float sum = 0.0f;
    
    /* Extract and sum 32 elements (8 vectors * 4 lanes) */
    sum += ((float*)&a)[0] + ((float*)&a)[1] + ((float*)&a)[2] + ((float*)&a)[3];
    sum += ((float*)&b)[0] + ((float*)&b)[1] + ((float*)&b)[2] + ((float*)&b)[3];
    sum += ((float*)&c)[0] + ((float*)&c)[1] + ((float*)&c)[2] + ((float*)&c)[3];
    sum += ((float*)&d)[0] + ((float*)&d)[1] + ((float*)&d)[2] + ((float*)&d)[3];
    sum += ((float*)&e)[0] + ((float*)&e)[1] + ((float*)&e)[2] + ((float*)&e)[3];
    sum += ((float*)&f)[0] + ((float*)&f)[1] + ((float*)&f)[2] + ((float*)&f)[3];
    sum += ((float*)&g)[0] + ((float*)&g)[1] + ((float*)&g)[2] + ((float*)&g)[3];
    sum += ((float*)&h)[0] + ((float*)&h)[1] + ((float*)&h)[2] + ((float*)&h)[3];
    
    return sum;
}

__attribute__((noinline, noipa, used))
static v4sf pattern_d(v4sf a, v4sf b, v4sf c, v4sf d,
                      v4sf e, v4sf f, v4sf g, v4sf h,
                      v4sf i, v4sf j, v4sf k) {
    /* Pattern D: Conditional vector move/merge with many inputs */
    /* Complex comparison tree creates many operands */
#ifdef __SSE__
    v4sf cmp1 = _mm_cmplt_ps(a, b);
    v4sf cmp2 = _mm_cmpgt_ps(c, d);
    v4sf cmp3 = _mm_cmpeq_ps(e, f);
    v4sf cmp4 = _mm_cmpneq_ps(g, h);
    
    v4sf mask1 = _mm_and_ps(cmp1, cmp2);
    v4sf mask2 = _mm_or_ps(cmp3, cmp4);
    v4sf final_mask = _mm_xor_ps(mask1, mask2);
    
    v4sf t1 = _mm_blendv_ps(i, j, final_mask);
    return _mm_blendv_ps(t1, k, cmp1);
#else
    /* Generic implementation */
    v4sf cmp1 = a < b;
    v4sf cmp2 = c > d;
    v4sf cmp3 = e == f;
    v4sf cmp4 = g != h;
    
    v4sf mask1 = cmp1 & cmp2;
    v4sf mask2 = cmp3 | cmp4;
    v4sf final_mask = mask1 ^ mask2;
    
    v4sf t1 = __builtin_shufflevector(i, j, 
        final_mask[0] ? 4 : 0, final_mask[1] ? 5 : 1,
        final_mask[2] ? 6 : 2, final_mask[3] ? 7 : 3);
    
    return __builtin_shufflevector(t1, k,
        cmp1[0] ? 8 : 0, cmp1[1] ? 9 : 1,
        cmp1[2] ? 10 : 2, cmp1[3] ? 11 : 3);
#endif
}

__attribute__((noinline, noipa, used))
static void pattern_e(int *out, int a, int b, int c, int d, int e,
                      int f, int g, int h, int i, int j, int k) {
    /* Pattern E: Inline assembly with exactly 11 operands */
    /* This directly creates an RTL insn with many operands */
    asm volatile (
        "/* Multi-operand assembly pattern */\n\t"
        "add %[a], %[b], %[c]\n\t"
        "add %[d], %[e], %[f]\n\t"
        "add %[g], %[h], %[i]\n\t"
        "mul %[j], %[k], %[out0]\n\t"
        : [out0] "=r" (out[0]), [out1] "=r" (out[1]),
          [out2] "=r" (out[2]), [out3] "=r" (out[3])
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Use argc for runtime variability to ensure all paths are compiled */
    int mode = argc > 1 ? atoi(argv[1]) % 5 : 0;
    
    /* Initialize test vectors */
    v4sf va = (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vb = (v4sf){5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vc = (v4sf){9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vd = (v4sf){13.0f, 14.0f, 15.0f, 16.0f};
    v4sf ve = (v4sf){17.0f, 18.0f, 19.0f, 20.0f};
    v4sf vf = (v4sf){21.0f, 22.0f, 23.0f, 24.0f};
    v4sf vg = (v4sf){25.0f, 26.0f, 27.0f, 28.0f};
    v4sf vh = (v4sf){29.0f, 30.0f, 31.0f, 32.0f};
    v4sf vi = (v4sf){33.0f, 34.0f, 35.0f, 36.0f};
    v4sf vj = (v4sf){37.0f, 38.0f, 39.0f, 40.0f};
    v4sf vk = (v4sf){41.0f, 42.0f, 43.0f, 44.0f};
    
    volatile float checksum = 0.0f;
    volatile int int_checksum = 0;
    int out_arr[4];
    
    switch (mode) {
        case 0:
            /* Trigger pattern A (shuffle/blend with 11 args) */
            checksum += ((float*)&pattern_a(va, vb, vc, vd, ve, vf, vg, vh, 
                                           0xE4, 0x1B, 0x39))[0];
            break;
        case 1:
            /* Trigger pattern B (FMA chain with 10 args) */
            checksum += ((float*)&pattern_b(va, vb, vc, vd, ve, vf, vg, vh, 
                                           vi, vj))[0];
            break;
        case 2:
            /* Trigger pattern C (reduction with 8 vector args + many extracts) */
            checksum += pattern_c(va, vb, vc, vd, ve, vf, vg, vh);
            break;
        case 3:
            /* Trigger pattern D (conditional merge with 11 args) */
            checksum += ((float*)&pattern_d(va, vb, vc, vd, ve, vf, vg, vh,
                                           vi, vj, vk))[0];
            break;
        case 4:
            /* Trigger pattern E (inline asm with 11 input operands) */
            pattern_e(out_arr, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
            int_checksum = out_arr[0] + out_arr[1] + out_arr[2] + out_arr[3];
            break;
    }
    
    /* Use results to prevent optimization */
    return (int)checksum + int_checksum;
}
