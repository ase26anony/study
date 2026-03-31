/* test_optabs_high_operand_count.c
 * 
 * This test targets GCC's optabs.cc expansion routines that handle
 * operations with 10-11 operands. The goal is to trigger the specific
 * switch cases at lines 8254-8263 in optabs.cc.
 */

#include <stdio.h>
#include <stdint.h>
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
static volatile int volatile_sink;

/* Pattern A: Complex vector blend with many operands */
NOINLINE USED
v4sf pattern_a_blend_many_operands(v4sf a, v4sf b, v4sf c, v4sf d,
                                   v4sf e, v4sf f, v4sf g, v4sf h,
                                   int mask1, int mask2, int mask3) {
    /* This should expand to many operands during blend/shuffle expansion */
#ifdef __SSE__
    /* Use SSE intrinsics directly */
    v4sf temp1 = __builtin_ia32_shufps(a, b, mask1);
    v4sf temp2 = __builtin_ia32_shufps(c, d, mask2);
    v4sf temp3 = __builtin_ia32_shufps(e, f, mask3);
    v4sf temp4 = __builtin_ia32_shufps(g, h, (mask1 ^ mask2) & 0xFF);
    
    /* Complex blend chain - each blend adds multiple operands */
    v4sf result = __builtin_ia32_blendps(temp1, temp2, (mask1 >> 4) & 0xF);
    result = __builtin_ia32_blendps(result, temp3, (mask2 >> 4) & 0xF);
    result = __builtin_ia32_blendps(result, temp4, (mask3 >> 4) & 0xF);
    
    /* Additional arithmetic to ensure expansion */
    result = result + temp1 * temp2 - temp3 / (temp4 + v4sf){1.0f, 1.0f, 1.0f, 1.0f};
    return result;
#else
    /* Fallback for non-SSE targets */
    return a + b + c + d + e + f + g + h;
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE USED
float pattern_b_fma_chain(float a, float b, float c, float d, float e,
                         float f, float g, float h, float i, float j) {
    /* Chain of FMAs creates expression tree that may flatten to many operands */
#ifdef __FMA__
    /* Use __builtin_fmaf if available */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(t1, t2, t3);
    float result = __builtin_fmaf(t4, j, a + b + c + d);
    
    /* Additional complex expression */
    result = __builtin_fmaf(result, a, __builtin_fmaf(b, c, 
                     __builtin_fmaf(d, e, __builtin_fmaf(f, g, 
                     __builtin_fmaf(h, i, j)))));
    return result;
#else
    /* Manual FMA emulation */
    float t1 = a * b + c;
    float t2 = d * e + f;
    float t3 = g * h + i;
    float t4 = t1 * t2 + t3;
    float result = t4 * j + a + b + c + d;
    result = result * a + (b * c + (d * e + (f * g + (h * i + j))));
    return result;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE USED
float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Manual unrolled reduction extracts each lane separately */
    float sum = 0.0f;
    
    /* Extract and sum each lane - each extract adds operands */
#ifdef __SSE__
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
#else
    /* Fallback using union for extraction */
    union { v4sf v; float a[4]; } u1, u2, u3, u4;
    u1.v = v1; u2.v = v2; u3.v = v3; u4.v = v4;
    for (int i = 0; i < 4; i++) {
        sum += u1.a[i] + u2.a[i] + u3.a[i] + u4.a[i];
    }
#endif
    
    /* Additional complex operations */
    sum = sum * 2.0f - sum / 3.0f + sum * sum - sum / sum;
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE USED
v4sf pattern_d_conditional_vector(v4sf a, v4sf b, v4sf c, v4sf d,
                                  v4sf e, v4sf f, v4sf g, v4sf h) {
#ifdef __SSE__
    /* Multiple comparisons create many operands */
    v4sf cmp1 = __builtin_ia32_cmpps(a, b, 0);  /* EQ */
    v4sf cmp2 = __builtin_ia32_cmpps(c, d, 1);  /* LT */
    v4sf cmp3 = __builtin_ia32_cmpps(e, f, 2);  /* LE */
    v4sf cmp4 = __builtin_ia32_cmpps(g, h, 4);  /* GT */
    
    /* Complex blend chain based on comparisons */
    v4sf temp1 = __builtin_ia32_andps(cmp1, cmp2);
    v4sf temp2 = __builtin_ia32_orps(cmp3, cmp4);
    v4sf mask = __builtin_ia32_xorps(temp1, temp2);
    
    /* Blend using the complex mask - may expand to many operands */
    v4sf result = __builtin_ia32_blendvps(a, b, mask);
    result = __builtin_ia32_blendvps(result, c, __builtin_ia32_andps(mask, cmp1));
    result = __builtin_ia32_blendvps(result, d, __builtin_ia32_orps(mask, cmp2));
    result = __builtin_ia32_blendvps(result, e, __builtin_ia32_xorps(mask, cmp3));
    
    return result;
#else
    /* Fallback */
    return a + b - c * d + e / f - g + h;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE USED
int pattern_e_inline_asm_11_operands(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    int result1, result2, result3;
    
    /* Inline assembly with 11 total operands (3 outputs, 8 inputs) */
    asm volatile (
        /* Complex operation with many operands */
        "mov %0, %3\n\t"
        "add %0, %4\n\t"
        "imul %0, %5\n\t"
        "sub %0, %6\n\t"
        "mov %1, %7\n\t"
        "xor %1, %8\n\t"
        "or %1, %9\n\t"
        "and %2, %10\n\t"
        "lea (%0, %1, 2), %2"
        : "=r"(result1), "=r"(result2), "=r"(result3)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h)
        : "cc"
    );
    
    /* Use results to prevent optimization */
    return result1 + result2 + result3 + i + j;
}

/* Pattern F: AVX2 gather operation simulation (many address operands) */
NOINLINE USED
v8sf pattern_f_gather_simulation(const float* base, v8si indices,
                                 v8sf mask, v8sf defaults,
                                 int scale1, int scale2, int scale3) {
#ifdef __AVX2__
    /* Simulate gather with many address calculations */
    v8sf result = defaults;
    
    /* Each gather element requires address calculation with multiple operands */
    for (int i = 0; i < 8; i++) {
        int idx = ((int*)&indices)[i];
        float val = ((mask[i] > 0) ? base[idx * scale1] : defaults[i]);
        ((float*)&result)[i] = val * scale2 + scale3;
    }
    
    /* Additional complex operations */
    result = result + mask * defaults - result / (defaults + 1.0f);
    return result;
#else
    /* Fallback */
    v8sf result = {0};
    for (int i = 0; i < 8; i++) {
        ((float*)&result)[i] = base[i] + i;
    }
    return result;
#endif
}

/* Main test driver with runtime variability */
int main(int argc, char** argv) {
    float checksum = 0.0f;
    
    /* Initialize test data with some variability */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf v4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf v5 = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf v6 = {21.0f, 22.0f, 23.0f, 24.0f};
    v4sf v7 = {25.0f, 26.0f, 27.0f, 28.0f};
    v4sf v8 = {29.0f, 30.0f, 31.0f, 32.0f};
    
    float scalars[20];
    for (int i = 0; i < 20; i++) {
        scalars[i] = (float)(i + argc); /* Use argc for variability */
    }
    
    /* Execute different patterns based on argc to ensure all are compiled */
    if (argc > 1) {
        /* Pattern A: Complex blend */
        v4sf res_a = pattern_a_blend_many_operands(v1, v2, v3, v4, v5, v6, v7, v8,
                                                   argc, argc*2, argc*3);
        checksum += ((float*)&res_a)[0] + ((float*)&res_a)[1];
        
        /* Pattern B: FMA chain */
        float res_b = pattern_b_fma_chain(scalars[0], scalars[1], scalars[2],
                                         scalars[3], scalars[4], scalars[5],
                                         scalars[6], scalars[7], scalars[8],
                                         scalars[9]);
        checksum += res_b;
    }
    
    if (argc > 2) {
        /* Pattern C: Vector reduction */
        float res_c = pattern_c_vector_reduction(v1, v2, v3, v4);
        checksum += res_c;
        
        /* Pattern D: Conditional vector */
        v4sf res_d = pattern_d_conditional_vector(v1, v2, v3, v4, v5, v6, v7, v8);
        checksum += ((float*)&res_d)[2] + ((float*)&res_d)[3];
    }
    
    if (argc > 3) {
        /* Pattern E: Inline assembly with 11 operands */
        int res_e = pattern_e_inline_asm_11_operands(argc, argc+1, argc+2,
                                                    argc+3, argc+4, argc+5,
                                                    argc+6, argc+7, argc+8,
                                                    argc+9);
        checksum += (float)res_e;
    }
    
    /* Always execute some pattern to ensure coverage */
    v4sf res_always = pattern_a_blend_many_operands(v1, v2, v3, v4, v5, v6, v7, v8,
                                                   1, 2, 3);
    checksum += ((float*)&res_always)[0] + ((float*)&res_always)[3];
    
    /* Store to volatile to prevent optimization */
    volatile_sink = (int)checksum;
    
    printf("Checksum: %f\n", checksum);
    return (checksum > 0) ? 0 : 1;
}
