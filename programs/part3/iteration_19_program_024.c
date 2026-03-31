/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's RTL expansion for instructions
 * with 10 or 11 operands, covering the uncovered lines in optabs.cc.
 * 
 * Compile with: gcc -O2 -fdump-rtl-expand -c test_optabs_10_11_operands.c
 * 
 * For detailed RTL analysis: gcc -O3 -fdump-rtl-all -dP -c test_optabs_10_11_operands.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure local expansion */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOINLINE static uint64_t multi_operand_arithmetic(uint64_t a, uint64_t b, 
                                                  uint64_t c, uint64_t d,
                                                  uint64_t e, uint64_t f,
                                                  uint64_t g, uint64_t h,
                                                  uint64_t i, uint64_t j)
{
    /* Complex expression that might require many temporary registers */
    uint64_t result = 0;
    
    /* Multiple operations that could be combined into a complex RTL pattern */
    result = ((a * b) >> 32) + ((c * d) >> 32);
    result += ((e * f) >> 32) + ((g * h) >> 32);
    result += ((i * j) >> 32);
    
    /* Additional operations to increase register pressure */
    result = (result * a) / (b + 1);
    result = (result + c) * (d + 1) / (e + 1);
    
    return result;
}

/* Vector operations that might expand to multi-operand instructions */
NOINLINE static v4si vector_multi_ops(v4si a, v4si b, v4si c, v4si d,
                                      v4si e, v4si f, v4si g, v4si h)
{
    /* Complex vector expression - might expand to many operands */
    v4si t1 = a * b + c;
    v4si t2 = d * e + f;
    v4si t3 = g * h + a;
    v4si t4 = b * c + d;
    
    v4si result = t1 + t2 + t3 + t4;
    result = result * a - b;
    result = result / (c + 1);
    
    return result;
}

#ifdef __x86_64__
/* x86-specific inline assembly with many operands */
NOINLINE static uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b,
                                               uint64_t c, uint64_t d,
                                               uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h,
                                               uint64_t i, uint64_t j)
{
    uint64_t result1, result2, result3;
    
    /* Inline assembly with 10 operands (9 inputs + 1 output) */
    asm volatile (
        "imulq %[b], %[a]\n\t"
        "imulq %[d], %[c]\n\t"
        "imulq %[f], %[e]\n\t"
        "imulq %[h], %[g]\n\t"
        "addq %[c], %[a]\n\t"
        "addq %[g], %[e]\n\t"
        "addq %[a], %[e]\n\t"
        "addq %[i], %[e]\n\t"
        "addq %[j], %[e]\n\t"
        : "=r" (result1), "=r" (result2), "=r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2 + result3;
}

#include <x86intrin.h>
/* AVX2/AVX-512 operations that might use many operands */
NOINLINE static __m256i avx_multi_operand(__m256i a, __m256i b, __m256i c,
                                          __m256i d, __m256i e, __m256i f)
{
    /* Complex AVX expression */
    __m256i t1 = _mm256_add_epi32(a, b);
    __m256i t2 = _mm256_mullo_epi32(c, d);
    __m256i t3 = _mm256_sub_epi32(e, f);
    __m256i t4 = _mm256_slli_epi32(a, 2);
    
    __m256i result = _mm256_add_epi32(t1, t2);
    result = _mm256_add_epi32(result, t3);
    result = _mm256_add_epi32(result, t4);
    
    /* Additional operations */
    result = _mm256_mullo_epi32(result, b);
    result = _mm256_srai_epi32(result, 1);
    
    return result;
}
#endif

#ifdef __aarch64__
#include <arm_neon.h>
/* ARM NEON operations with many operands */
NOINLINE static int32x4_t neon_multi_operand(int32x4_t a, int32x4_t b,
                                             int32x4_t c, int32x4_t d,
                                             int32x4_t e, int32x4_t f,
                                             int32x4_t g, int32x4_t h)
{
    /* Complex NEON expression */
    int32x4_t t1 = vaddq_s32(a, b);
    int32x4_t t2 = vmulq_s32(c, d);
    int32x4_t t3 = vsubq_s32(e, f);
    int32x4_t t4 = vshlq_s32(g, vdupq_n_s32(2));
    
    int32x4_t result = vaddq_s32(t1, t2);
    result = vaddq_s32(result, t3);
    result = vaddq_s32(result, t4);
    result = vaddq_s32(result, h);
    
    /* More operations */
    result = vmulq_s32(result, a);
    result = vrshrq_n_s32(result, 1);
    
    return result;
}

/* ARM-specific inline assembly with many operands */
NOINLINE static uint64_t arm_multi_operand_asm(uint64_t a, uint64_t b,
                                               uint64_t c, uint64_t d,
                                               uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h)
{
    uint64_t result1, result2, result3, result4;
    
    asm volatile (
        "mul %[r1], %[a], %[b]\n\t"
        "mul %[r2], %[c], %[d]\n\t"
        "add %[r1], %[r1], %[r2]\n\t"
        "mul %[r3], %[e], %[f]\n\t"
        "mul %[r4], %[g], %[h]\n\t"
        "add %[r3], %[r3], %[r4]\n\t"
        "add %[r1], %[r1], %[r3]\n\t"
        : [r1] "=r" (result1), [r2] "=r" (result2),
          [r3] "=r" (result3), [r4] "=r" (result4)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h)
        : "cc"
    );
    
    return result1 + result2 + result3 + result4;
}
#endif

/* Function that tries multiple expansion paths */
NOINLINE static uint64_t trigger_multi_operand_expansion(int variant, 
                                                         uint64_t *inputs)
{
    uint64_t result = 0;
    
    switch (variant % 4) {
        case 0:
            /* Complex arithmetic expression */
            result = multi_operand_arithmetic(inputs[0], inputs[1], inputs[2],
                                             inputs[3], inputs[4], inputs[5],
                                             inputs[6], inputs[7], inputs[8],
                                             inputs[9]);
            break;
            
        case 1:
            /* Vector operations */
            {
                v4si a = {inputs[0], inputs[1], inputs[2], inputs[3]};
                v4si b = {inputs[4], inputs[5], inputs[6], inputs[7]};
                v4si c = {inputs[8], inputs[9], inputs[0], inputs[1]};
                v4si d = {inputs[2], inputs[3], inputs[4], inputs[5]};
                v4si e = {inputs[6], inputs[7], inputs[8], inputs[9]};
                v4si f = {inputs[0], inputs[2], inputs[4], inputs[6]};
                v4si g = {inputs[1], inputs[3], inputs[5], inputs[7]};
                v4si h = {inputs[8], inputs[9], inputs[0], inputs[1]};
                
                v4si vec_result = vector_multi_ops(a, b, c, d, e, f, g, h);
                result = vec_result[0] + vec_result[1] + 
                         vec_result[2] + vec_result[3];
            }
            break;
            
#ifdef __x86_64__
        case 2:
            /* x86-specific paths */
            result = x86_multi_operand_asm(inputs[0], inputs[1], inputs[2],
                                          inputs[3], inputs[4], inputs[5],
                                          inputs[6], inputs[7], inputs[8],
                                          inputs[9]);
            break;
#endif
            
#ifdef __aarch64__
        case 3:
            /* ARM-specific paths */
            result = arm_multi_operand_asm(inputs[0], inputs[1], inputs[2],
                                          inputs[3], inputs[4], inputs[5],
                                          inputs[6], inputs[7]);
            break;
#endif
            
        default:
            result = inputs[0] + inputs[1];
    }
    
    return result;
}

int main(int argc, char *argv[])
{
    uint64_t inputs[10];
    uint64_t total_result = 0;
    
    /* Initialize inputs with varying values based on argc */
    for (int i = 0; i < 10; i++) {
        inputs[i] = (uint64_t)(argc + i * 7) * 0x123456789ABCDEFULL;
    }
    
    /* Try different expansion paths based on command line arguments */
    int num_variants = 4;
    
    for (int i = 0; i < (argc > 1 ? atoi(argv[1]) : 100); i++) {
        int variant = i % num_variants;
        
        /* Call function that might trigger multi-operand expansion */
        uint64_t result = trigger_multi_operand_expansion(variant, inputs);
        
        /* Use result to prevent dead code elimination */
        total_result ^= result;
        
        /* Modify inputs slightly for next iteration */
        for (int j = 0; j < 10; j++) {
            inputs[j] = inputs[j] * 6364136223846793005ULL + 1;
        }
    }
    
    /* Print result to ensure code isn't optimized away */
    printf("Result: %lu\n", (unsigned long)total_result);
    
    return (int)(total_result & 0x7FFFFFFF);
}
