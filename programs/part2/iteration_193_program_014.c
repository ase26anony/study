/* test_optabs_high_operand_count.c
 * 
 * This test program is designed to trigger GCC's RTL expansion for operations
 * with 10 or 11 operands, specifically targeting uncovered switch cases
 * in optabs.cc (lines 8254-8263).
 *
 * Compilation flags recommended:
 *   gcc -O3 -march=native -fno-tree-vectorize -fprofile-arcs -ftest-coverage \
 *       -fdump-rtl-expand test_optabs_high_operand_count.c -o test_optabs
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent aggressive optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define USED __attribute__((used))

/* Generic vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* SSE/AVX intrinsics */
#ifdef __SSE__
#include <xmmintrin.h>
#endif

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#ifdef __SSE3__
#include <pmmintrin.h>
#endif

#ifdef __SSSE3__
#include <tmmintrin.h>
#endif

#ifdef __SSE4_1__
#include <smmintrin.h>
#endif

#ifdef __AVX__
#include <immintrin.h>
#endif

/* ARM NEON intrinsics */
#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* Pattern A: Vector blend/select with many conditions */
NOINLINE USED v4sf pattern_a_vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d,
                                                  v4sf e, v4sf f, v4sf g, v4sf h,
                                                  int mask1, int mask2, int mask3) {
    /* Complex blending operation that may expand to many operands */
    v4sf temp1 = a + b;
    v4sf temp2 = c - d;
    v4sf temp3 = e * f;
    v4sf temp4 = g / h;
    
    /* Create complex mask from multiple inputs */
    v4si mask_vec = {mask1, mask2, mask3, mask1 ^ mask2 ^ mask3};
    
    /* Manual blend operation - each element conditionally selected */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        float* res_ptr = (float*)&result;
        float* t1_ptr = (float*)&temp1;
        float* t2_ptr = (float*)&temp2;
        float* t3_ptr = (float*)&temp3;
        float* t4_ptr = (float*)&temp4;
        int* mask_ptr = (int*)&mask_vec;
        
        /* Complex conditional selection - may expand to many RTL operands */
        if (mask_ptr[i] & 0x1) {
            res_ptr[i] = t1_ptr[i] + t2_ptr[i];
        } else if (mask_ptr[i] & 0x2) {
            res_ptr[i] = t3_ptr[i] - t4_ptr[i];
        } else if (mask_ptr[i] & 0x4) {
            res_ptr[i] = t1_ptr[i] * t3_ptr[i];
        } else {
            res_ptr[i] = t2_ptr[i] / t4_ptr[i];
        }
        
        /* Additional operations to increase operand count */
        res_ptr[i] += (mask_ptr[i] & 0x8) ? t1_ptr[(i+1)%4] : t2_ptr[(i+2)%4];
        res_ptr[i] *= (mask_ptr[i] & 0x10) ? 1.5f : 0.75f;
    }
    
    return result;
}

/* Pattern B: Fused multiply-add chain with many accumulators */
NOINLINE USED float pattern_b_fma_chain(float a, float b, float c, float d, float e,
                                       float f, float g, float h, float i, float j,
                                       float k, float l, float m, float n) {
    /* Deep FMA chain that may flatten to many operands */
    float result = a;
    
    /* Chain of operations that may be expanded with many operands */
    result = result * b + c;
    result = result * d + e;
    result = result * f + g;
    result = result * h + i;
    result = result * j + k;
    result = result * l + m;
    result = result * 2.0f + n;
    
    /* Additional complex expression */
    result = (result * a + b) * (c + d) - (e * f + g) / (h - i) + (j * k - l) * (m + n);
    
    return result;
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE USED float pattern_c_vector_reduction_unrolled(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Manually unrolled horizontal addition with many extract operations */
    float sum = 0.0f;
    
    /* Extract each element individually - each extract may be an operand */
    float v1_0 = ((float*)&v1)[0];
    float v1_1 = ((float*)&v1)[1];
    float v1_2 = ((float*)&v1)[2];
    float v1_3 = ((float*)&v1)[3];
    
    float v2_0 = ((float*)&v2)[0];
    float v2_1 = ((float*)&v2)[1];
    float v2_2 = ((float*)&v2)[2];
    float v2_3 = ((float*)&v2)[3];
    
    float v3_0 = ((float*)&v3)[0];
    float v3_1 = ((float*)&v3)[1];
    float v3_2 = ((float*)&v3)[2];
    float v3_3 = ((float*)&v3)[3];
    
    float v4_0 = ((float*)&v4)[0];
    float v4_1 = ((float*)&v4)[1];
    float v4_2 = ((float*)&v4)[2];
    float v4_3 = ((float*)&v4)[3];
    
    /* Complex reduction with many operands */
    sum = v1_0 + v1_1 + v1_2 + v1_3 +
          v2_0 + v2_1 + v2_2 + v2_3 +
          v3_0 + v3_1 + v3_2 + v3_3 +
          v4_0 + v4_1 + v4_2 + v4_3;
    
    /* Additional operations to increase complexity */
    sum = sum * (v1_0 - v2_0) + (v3_1 * v4_1) - (v2_2 / v3_2) + (v4_3 * v1_3);
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE USED v4sf pattern_d_conditional_vector_ops(v4sf a, v4sf b, v4sf c, v4sf d,
                                                    v4sf e, v4sf f, v4sf g, v4sf h) {
    /* Complex conditional vector operations */
    v4sf cmp1 = a > b;
    v4sf cmp2 = c < d;
    v4sf cmp3 = e == f;
    v4sf cmp4 = g != h;
    
    /* Combine comparisons - each may add operands */
    v4sf mask1 = cmp1 & cmp2;
    v4sf mask2 = cmp3 | cmp4;
    v4sf mask3 = mask1 ^ mask2;
    
    /* Conditional operations based on masks */
    v4sf temp1 = a * b;
    v4sf temp2 = c + d;
    v4sf temp3 = e - f;
    v4sf temp4 = g / h;
    
    /* Manual conditional select with many operands */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        float* res_ptr = (float*)&result;
        float* t1_ptr = (float*)&temp1;
        float* t2_ptr = (float*)&temp2;
        float* t3_ptr = (float*)&temp3;
        float* t4_ptr = (float*)&temp4;
        int* m1_ptr = (int*)&mask1;
        int* m2_ptr = (int*)&mask2;
        int* m3_ptr = (int*)&mask3;
        
        /* Complex conditional expression */
        if (m1_ptr[i]) {
            res_ptr[i] = t1_ptr[i] + t2_ptr[i];
        } else if (m2_ptr[i]) {
            res_ptr[i] = t3_ptr[i] - t4_ptr[i];
        } else if (m3_ptr[i]) {
            res_ptr[i] = t1_ptr[i] * t3_ptr[i];
        } else {
            res_ptr[i] = t2_ptr[i] / t4_ptr[i];
        }
        
        /* Additional conditional operations */
        res_ptr[i] += m1_ptr[i] ? t1_ptr[(i+1)%4] : t2_ptr[(i+2)%4];
        res_ptr[i] *= m2_ptr[i] ? 2.0f : 0.5f;
        res_ptr[i] -= m3_ptr[i] ? t3_ptr[(i+3)%4] : t4_ptr[i];
    }
    
    return result;
}

/* Pattern E: Inline assembly with many operands */
NOINLINE USED void pattern_e_inline_asm_many_operands(float* a, float* b, float* c,
                                                     float* d, float* e, float* f,
                                                     float* g, float* h, float* i,
                                                     float* j, float* k) {
    /* Inline assembly with exactly 11 operands */
    asm volatile (
        /* Complex multi-operand operation */
        "mov %[a], %[b] \n\t"
        "add %[c], %[d] \n\t"
        "sub %[e], %[f] \n\t"
        "mul %[g], %[h] \n\t"
        "div %[i], %[j] \n\t"
        "faddp %[k], %[a]"
        : [a] "+r" (*a), [b] "+r" (*b), [c] "+r" (*c),
          [d] "+r" (*d), [e] "+r" (*e), [f] "+r" (*f),
          [g] "+r" (*g), [h] "+r" (*h), [i] "+r" (*i),
          [j] "+r" (*j), [k] "+r" (*k)
        : 
        : "memory", "cc"
    );
}

/* Pattern F: Complex expression with many temporaries */
NOINLINE USED float pattern_f_complex_expression(float a, float b, float c, float d,
                                                float e, float f, float g, float h,
                                                float i, float j, float k, float l,
                                                float m, float n, float o, float p) {
    /* Expression designed to create many RTL operands during expansion */
    float t1 = a + b;
    float t2 = c - d;
    float t3 = e * f;
    float t4 = g / h;
    float t5 = i + j;
    float t6 = k - l;
    float t7 = m * n;
    float t8 = o / p;
    
    float t9 = t1 * t2 + t3 - t4;
    float t10 = t5 / t6 * t7 + t8;
    float t11 = t9 - t10 + t1 * t3;
    float t12 = t2 / t4 * t5 - t6;
    float t13 = t7 + t8 * t9 / t10;
    float t14 = t11 - t12 + t13 * t1;
    
    return t14 * t2 / t3 + t4 - t5 * t6 / t7 + t8;
}

/* Pattern G: Vector shuffle with complex index computation */
NOINLINE USED v4sf pattern_g_complex_shuffle(v4sf a, v4sf b, v4sf c, v4sf d,
                                            int idx1, int idx2, int idx3, int idx4,
                                            int idx5, int idx6, int idx7, int idx8) {
    /* Complex shuffle operation that may expand to many operands */
    v4sf temp[4] = {a, b, c, d};
    
    /* Compute shuffle indices from multiple inputs */
    int indices[4];
    indices[0] = (idx1 + idx2) % 4;
    indices[1] = (idx3 ^ idx4) % 4;
    indices[2] = (idx5 * idx6) % 4;
    indices[3] = (idx7 - idx8) % 4;
    
    /* Manual shuffle with many operands */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        int src_vec = indices[i] / 4;
        int src_idx = indices[i] % 4;
        ((float*)&result)[i] = ((float*)&temp[src_vec])[src_idx];
    }
    
    /* Additional operations */
    result = result + a * b - c / d;
    
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    volatile float checksum = 0.0f;
    
    /* Initialize test data with some variability based on argc */
    v4sf vec1 = {1.0f + argc, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f + argc, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f + argc, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f + argc};
    
    float scalars[16];
    for (int i = 0; i < 16; i++) {
        scalars[i] = (float)(i + argc);
    }
    
    /* Execute all patterns to ensure compilation and coverage */
    
    /* Pattern A */
    v4sf result_a = pattern_a_vector_blend_complex(vec1, vec2, vec3, vec4,
                                                  vec2, vec1, vec4, vec3,
                                                  argc, argc+1, argc+2);
    checksum += ((float*)&result_a)[0] + ((float*)&result_a)[1] +
                ((float*)&result_a)[2] + ((float*)&result_a)[3];
    
    /* Pattern B */
    float result_b = pattern_b_fma_chain(scalars[0], scalars[1], scalars[2],
                                        scalars[3], scalars[4], scalars[5],
                                        scalars[6], scalars[7], scalars[8],
                                        scalars[9], scalars[10], scalars[11],
                                        scalars[12], scalars[13]);
    checksum += result_b;
    
    /* Pattern C */
    float result_c = pattern_c_vector_reduction_unrolled(vec1, vec2, vec3, vec4);
    checksum += result_c;
    
    /* Pattern D */
    v4sf result_d = pattern_d_conditional_vector_ops(vec1, vec2, vec3, vec4,
                                                    vec2, vec1, vec4, vec3);
    checksum += ((float*)&result_d)[0] + ((float*)&result_d)[1] +
                ((float*)&result_d)[2] + ((float*)&result_d)[3];
    
    /* Pattern E */
    float asm_operands[11];
    for (int i = 0; i < 11; i++) {
        asm_operands[i] = (float)(i + argc);
    }
    pattern_e_inline_asm_many_operands(&asm_operands[0], &asm_operands[1],
                                      &asm_operands[2], &asm_operands[3],
                                      &asm_operands[4], &asm_operands[5],
                                      &asm_operands[6], &asm_operands[7],
                                      &asm_operands[8], &asm_operands[9],
                                      &asm_operands[10]);
    for (int i = 0; i < 11; i++) {
        checksum += asm_operands[i];
    }
    
    /* Pattern F */
    float result_f = pattern_f_complex_expression(scalars[0], scalars[1], scalars[2],
                                                 scalars[3], scalars[4], scalars[5],
                                                 scalars[6], scalars[7], scalars[8],
                                                 scalars[9], scalars[10], scalars[11],
                                                 scalars[12], scalars[13], scalars[14],
                                                 scalars[15]);
    checksum += result_f;
    
    /* Pattern G */
    v4sf result_g = pattern_g_complex_shuffle(vec1, vec2, vec3, vec4,
                                             argc, argc+1, argc+2, argc+3,
                                             argc+4, argc+5, argc+6, argc+7);
    checksum += ((float*)&result_g)[0] + ((float*)&result_g)[1] +
                ((float*)&result_g)[2] + ((float*)&result_g)[3];
    
    /* Use checksum to prevent dead code elimination */
    printf("Checksum: %f\n", checksum);
    
    return (checksum > 100.0f) ? 0 : 1;
}
