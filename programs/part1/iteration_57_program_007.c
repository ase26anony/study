/* Test program to trigger 10/11-operand instruction expansion in GCC optabs */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Architecture-specific headers */
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#endif

#ifdef __ARM_ARCH
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#endif

#ifdef __PPC64__
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression_10_operands(int a, int b, int c, int d, 
                                                 int e, int f, int g, int h,
                                                 int i, int j) {
    /* This expression might be combined into a single instruction 
       with 10 operands during optimization */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Another complex expression with 11 operands */
static inline int complex_expression_11_operands(int a, int b, int c, int d,
                                                 int e, int f, int g, int h,
                                                 int i, int j, int k) {
    return (a * b) + (c * d) + (e * f) + (g * h) + (i * j) + k;
}

int main() {
    int result = 0;
    
    /* Test complex expressions with many operands */
    int vars[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    
    /* Expression with 10 operands */
    result += complex_expression_10_operands(vars[0], vars[1], vars[2], vars[3],
                                            vars[4], vars[5], vars[6], vars[7],
                                            vars[8], vars[9]);
    
    /* Expression with 11 operands */
    result += complex_expression_11_operands(vars[0], vars[1], vars[2], vars[3],
                                            vars[4], vars[5], vars[6], vars[7],
                                            vars[8], vars[9], vars[10]);
    
    /* Inline assembly with exactly 11 operands */
    /* This forces the compiler to handle 11 operands in RTL expansion */
    {
        int64_t op0 = 1, op1 = 2, op2 = 3, op3 = 4, op4 = 5;
        int64_t op5 = 6, op6 = 7, op7 = 8, op8 = 9, op9 = 10, op10 = 11;
        int64_t output;
        
        /* 11-operand inline asm - the template doesn't matter much,
           it's the operand count that triggers the expansion */
        asm volatile (
            "/* 11-operand asm block */\n\t"
            "mov %0, %1\n\t"
            : "=r" (output)
            : "r" (op0), "r" (op1), "r" (op2), "r" (op3), "r" (op4),
              "r" (op5), "r" (op6), "r" (op7), "r" (op8), "r" (op9),
              "r" (op10)
            : "memory"
        );
        result += output;
    }
    
    /* Another inline asm with 10 operands */
    {
        int32_t in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5;
        int32_t in5 = 6, in6 = 7, in7 = 8, in8 = 9, in9 = 10;
        int32_t out;
        
        asm volatile (
            "/* 10-operand asm block */\n\t"
            "add %0, %1, %2\n\t"
            : "=r" (out)
            : "r" (in0), "r" (in1), "r" (in2), "r" (in3), "r" (in4),
              "r" (in5), "r" (in6), "r" (in7), "r" (in8), "r" (in9)
            : "cc"
        );
        result += out;
    }
    
    /* Use atomic built-in with many parameters (6 params + return = 7 total) */
    {
        int64_t atomic_var = 100;
        int64_t expected = 100;
        int64_t desired = 200;
        int success;
        
        /* __atomic_compare_exchange has 6 parameters plus return value */
        success = __atomic_compare_exchange(&atomic_var, &expected, &desired,
                                           0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        result += success + atomic_var;
    }
    
    /* Architecture-specific vector intrinsics */
#ifdef __x86_64__
    {
        /* AVX-512 masked operation with multiple operands */
        __m512i vec1 = _mm512_set1_epi32(1);
        __m512i vec2 = _mm512_set1_epi32(2);
        __m512i vec3 = _mm512_set1_epi32(3);
        __mmask16 mask = 0xAAAA;
        
        /* Fused multiply-add with mask - potentially expands to multi-operand instruction */
        __m512i res = _mm512_mask_mullo_epi32(vec1, mask, vec2, vec3);
        
        /* Extract and sum results */
        int32_t temp[16];
        _mm512_storeu_si512(temp, res);
        for (int i = 0; i < 16; i++) {
            result += temp[i];
        }
    }
#endif
    
#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
    {
        /* SVE2 intrinsics that use many operands */
        svint32_t sve_vec1 = svdup_s32(1);
        svint32_t sve_vec2 = svdup_s32(2);
        svint32_t sve_vec3 = svdup_s32(3);
        svbool_t pg = svptrue_b32();
        
        /* Complex SVE operation - the actual intrinsic would depend on 
           specific SVE2 instruction availability */
        svint32_t sve_res = svmla_s32_z(pg, sve_vec1, sve_vec2, sve_vec3);
        
        /* Store and accumulate */
        int32_t sve_temp[16] = {0};
        svst1_s32(pg, sve_temp, sve_res);
        for (int i = 0; i < 16; i++) {
            result += sve_temp[i];
        }
    }
#endif
    
    /* ARM NEON with lane operations */
    {
        int32x4_t neon1 = vdupq_n_s32(1);
        int32x4_t neon2 = vdupq_n_s32(2);
        int32x4_t neon3 = vdupq_n_s32(3);
        int32x4_t neon4 = vdupq_n_s32(4);
        
        /* Multi-lane operation */
        int32x4_t neon_res = vmlaq_laneq_s32(neon1, neon2, neon3, 1);
        neon_res = vmlaq_laneq_s32(neon_res, neon4, neon1, 2);
        
        /* Extract and sum */
        result += vgetq_lane_s32(neon_res, 0) +
                 vgetq_lane_s32(neon_res, 1) +
                 vgetq_lane_s32(neon_res, 2) +
                 vgetq_lane_s32(neon_res, 3);
    }
#endif
    
#ifdef __PPC64__
    {
        /* PowerPC VSX/Altivec operations */
        vector int v1 = {1, 2, 3, 4};
        vector int v2 = {5, 6, 7, 8};
        vector int v3 = {9, 10, 11, 12};
        vector int v4 = {13, 14, 15, 16};
        
        /* Complex vector operation */
        vector int vres = vec_madd(v1, v2, v3);
        vres = vec_add(vres, v4);
        
        /* Extract and sum */
        int* pres = (int*)&vres;
        for (int i = 0; i < 4; i++) {
            result += pres[i];
        }
    }
#endif
    
    /* Final complex expression mixing all results */
    {
        int a = result, b = 2, c = 3, d = 4, e = 5;
        int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
        
        /* This might get optimized into a complex multi-operand instruction */
        int final = a + b * c + d * e + f * g + h * i + j * k;
        result = final;
    }
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
