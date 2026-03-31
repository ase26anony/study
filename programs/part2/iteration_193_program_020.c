/* test_optabs_coverage.c
 * 
 * This program is designed to trigger the 10 and 11 operand switch cases
 * in GCC's optabs.cc expansion routines. It uses various patterns that
 * require many operands during RTL expansion.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types for portability */
#ifdef __SSE__
#include <xmmintrin.h>
#else
/* Fallback definitions if SSE not available */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
#endif

/* Prevent optimization and ensure expansion */
#define NOINLINE __attribute__((noinline, noipa))
#define USED __attribute__((used))

/* Volatile sink to prevent dead code elimination */
static volatile int sink;

/* Pattern A: Vector blend with complex mask computation (10+ operands) */
NOINLINE USED
v4sf pattern_a_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d, 
                             v4sf e, v4sf f, v4sf g, v4sf h) {
#ifdef __SSE4_1__
    /* Use blendps with immediate mask - expansion may need many operands */
    v4sf temp1 = _mm_blend_ps(a, b, 0x5);  /* 0101 mask */
    v4sf temp2 = _mm_blend_ps(c, d, 0xA);  /* 1010 mask */
    v4sf temp3 = _mm_blend_ps(e, f, 0x3);  /* 0011 mask */
    v4sf temp4 = _mm_blend_ps(g, h, 0xC);  /* 1100 mask */
    
    /* Chain blends - creates deep expression tree */
    v4sf result = _mm_blend_ps(temp1, temp2, 0x6);
    result = _mm_blend_ps(result, temp3, 0x9);
    result = _mm_blend_ps(result, temp4, 0xC);
    
    return result;
#else
    /* Manual blend implementation that expands to many operations */
    v4sf mask1 = a > b;
    v4sf mask2 = c < d;
    v4sf mask3 = e != f;
    v4sf mask4 = g == h;
    
    /* Complex conditional selection - each element selection is separate */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        float* r = (float*)&result;
        float* a_ptr = (float*)&a;
        float* b_ptr = (float*)&b;
        float* c_ptr = (float*)&c;
        float* d_ptr = (float*)&d;
        float* e_ptr = (float*)&e;
        float* f_ptr = (float*)&f;
        float* g_ptr = (float*)&g;
        float* h_ptr = (float*)&h;
        
        /* Each element selection involves multiple operands */
        r[i] = (a_ptr[i] > b_ptr[i]) ? 
               ((c_ptr[i] < d_ptr[i]) ? e_ptr[i] : f_ptr[i]) :
               ((g_ptr[i] == h_ptr[i]) ? h_ptr[i] : a_ptr[i]);
    }
    return result;
#endif
}

/* Pattern B: Fused multiply-add chain (11 operands in expression tree) */
NOINLINE USED
float pattern_b_fma_chain(float a, float b, float c, float d, float e,
                          float f, float g, float h, float i, float j) {
#ifdef __FMA__
    /* Chain of FMA operations creates deep expression tree */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    
    /* Nested FMA calls - expands to complex RTL with many operands */
    float result = __builtin_fmaf(t1, t2, __builtin_fmaf(t3, j, a));
    
    return result;
#else
    /* Manual FMA simulation - expands to many arithmetic operations */
    float t1 = a * b + c;
    float t2 = d * e + f;
    float t3 = g * h + i;
    float t4 = t3 * j + a;
    float result = t1 * t2 + t4;
    
    return result;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization (10+ extract ops) */
NOINLINE USED
float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Manual horizontal addition - each extract is a separate operation */
    float sum = 0.0f;
    
    /* Extract each element - each extract adds operands */
    for (int i = 0; i < 4; i++) {
        float* v1p = (float*)&v1;
        float* v2p = (float*)&v2;
        float* v3p = (float*)&v3;
        float* v4p = (float*)&v4;
        
        sum += v1p[i] + v2p[i] + v3p[i] + v4p[i];
    }
    
    /* Additional arithmetic to create more operands */
    sum = sum * 2.0f - sum / 2.0f + sum * 1.5f;
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE USED
v4sf pattern_d_conditional_vector(v4sf a, v4sf b, v4sf c, v4sf d,
                                  v4sf e, v4sf f, v4sf g, v4sf h) {
#ifdef __SSE__
    /* Multiple comparisons and blends */
    v4sf cmp1 = _mm_cmplt_ps(a, b);
    v4sf cmp2 = _mm_cmpgt_ps(c, d);
    v4sf cmp3 = _mm_cmpeq_ps(e, f);
    v4sf cmp4 = _mm_cmpneq_ps(g, h);
    
    /* Combine masks - each operation adds operands */
    v4sf mask1 = _mm_and_ps(cmp1, cmp2);
    v4sf mask2 = _mm_or_ps(cmp3, cmp4);
    v4sf final_mask = _mm_xor_ps(mask1, mask2);
    
    /* Conditional selection based on complex mask */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        float* r = (float*)&result;
        float* a_ptr = (float*)&a;
        float* b_ptr = (float*)&b;
        float* m_ptr = (float*)&final_mask;
        
        /* Each element selection is conditional */
        r[i] = (m_ptr[i] > 0) ? a_ptr[i] : b_ptr[i];
    }
    
    return result;
#else
    /* Manual implementation */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        float* r = (float*)&result;
        float* a_ptr = (float*)&a;
        float* b_ptr = (float*)&b;
        float* c_ptr = (float*)&c;
        float* d_ptr = (float*)&d;
        float* e_ptr = (float*)&e;
        float* f_ptr = (float*)&f;
        float* g_ptr = (float*)&g;
        float* h_ptr = (float*)&h;
        
        /* Complex conditional with many operands */
        int cond1 = a_ptr[i] < b_ptr[i];
        int cond2 = c_ptr[i] > d_ptr[i];
        int cond3 = e_ptr[i] == f_ptr[i];
        int cond4 = g_ptr[i] != h_ptr[i];
        
        int mask1 = cond1 && cond2;
        int mask2 = cond3 || cond4;
        int final_mask = mask1 ^ mask2;
        
        r[i] = final_mask ? a_ptr[i] : b_ptr[i];
    }
    return result;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE USED
int64_t pattern_e_multi_operand_asm(int64_t a, int64_t b, int64_t c, 
                                    int64_t d, int64_t e, int64_t f,
                                    int64_t g, int64_t h, int64_t i,
                                    int64_t j, int64_t k) {
    int64_t result1, result2, result3;
    
    /* Inline assembly with 11 explicit operands (10 inputs, 1 output) */
    asm volatile (
        "add %[r1], %[a], %[b]\n\t"
        "add %[r2], %[c], %[d]\n\t"
        "add %[r3], %[e], %[f]\n\t"
        "add %[r1], %[r1], %[g]\n\t"
        "add %[r2], %[r2], %[h]\n\t"
        "add %[r3], %[r3], %[i]\n\t"
        "mul %[r1], %[r1], %[j]\n\t"
        "mul %[r2], %[r2], %[k]\n\t"
        "add %0, %[r1], %[r2]\n\t"
        "add %0, %0, %[r3]"
        : "=r" (result1)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k),
          [r1] "r" (result2), [r2] "r" (result3)
        : "cc"
    );
    
    return result1;
}

/* Pattern F: Shuffle with dynamic mask (10+ operands) */
NOINLINE USED
v4sf pattern_f_shuffle_dynamic(v4sf a, v4sf b, v4sf c, v4sf d,
                               int mask1, int mask2, int mask3, int mask4) {
#ifdef __SSE__
    /* Multiple shuffles with different masks */
    v4sf s1 = _mm_shuffle_ps(a, b, mask1);
    v4sf s2 = _mm_shuffle_ps(c, d, mask2);
    v4sf s3 = _mm_shuffle_ps(s1, s2, mask3);
    v4sf result = _mm_shuffle_ps(s3, a, mask4);
    
    return result;
#else
    /* Manual shuffle */
    v4sf result;
    float* r = (float*)&result;
    float* a_ptr = (float*)&a;
    float* b_ptr = (float*)&b;
    float* c_ptr = (float*)&c;
    float* d_ptr = (float*)&d;
    
    /* Complex indexing with many operands */
    int indices[4] = {mask1 & 0x3, (mask1 >> 2) & 0x3,
                      (mask2 & 0x3) + 4, ((mask2 >> 2) & 0x3) + 4};
    
    for (int i = 0; i < 4; i++) {
        int idx = indices[i];
        if (idx < 4) r[i] = a_ptr[idx];
        else if (idx < 8) r[i] = b_ptr[idx - 4];
        else if (idx < 12) r[i] = c_ptr[idx - 8];
        else r[i] = d_ptr[idx - 12];
    }
    
    return result;
#endif
}

/* Main test driver with runtime variability */
int main(int argc, char** argv) {
    /* Initialize with some values */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf vec5 = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf vec6 = {21.0f, 22.0f, 23.0f, 24.0f};
    v4sf vec7 = {25.0f, 26.0f, 27.0f, 28.0f};
    v4sf vec8 = {29.0f, 30.0f, 31.0f, 32.0f};
    
    float scalar_result = 0.0f;
    v4sf vector_result;
    int64_t int_result = 0;
    
    /* Use argc to select different patterns, ensuring all get compiled */
    switch (argc % 6) {
        case 0:
            /* Pattern A - Complex blend */
            vector_result = pattern_a_blend_complex(vec1, vec2, vec3, vec4,
                                                   vec5, vec6, vec7, vec8);
            sink = *(int*)&vector_result;
            break;
            
        case 1:
            /* Pattern B - FMA chain (11 scalar operands) */
            scalar_result = pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f,
                                               6.6f, 7.7f, 8.8f, 9.9f, 10.10f);
            sink = (int)scalar_result;
            break;
            
        case 2:
            /* Pattern C - Vector reduction */
            scalar_result = pattern_c_vector_reduction(vec1, vec2, vec3, vec4);
            sink = (int)scalar_result;
            break;
            
        case 3:
            /* Pattern D - Conditional vector */
            vector_result = pattern_d_conditional_vector(vec1, vec2, vec3, vec4,
                                                        vec5, vec6, vec7, vec8);
            sink = *(int*)&vector_result;
            break;
            
        case 4:
            /* Pattern E - Multi-operand assembly (11 operands) */
            int_result = pattern_e_multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
            sink = (int)int_result;
            break;
            
        case 5:
            /* Pattern F - Dynamic shuffle */
            vector_result = pattern_f_shuffle_dynamic(vec1, vec2, vec3, vec4,
                                                    0x1B, 0x27, 0x39, 0x4E);
            sink = *(int*)&vector_result;
            break;
    }
    
    /* Compute checksum to use all results */
    float checksum = scalar_result + 
                     ((float*)&vector_result)[0] +
                     ((float*)&vector_result)[1] +
                     ((float*)&vector_result)[2] +
                     ((float*)&vector_result)[3] +
                     (float)int_result;
    
    printf("Checksum: %f\n", checksum);
    return (int)checksum;
}
