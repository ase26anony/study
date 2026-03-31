/* test_optabs.c - Test program to cover 10/11 operand switch cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types for portability */
#if defined(__SSE__) || defined(__x86_64__) || defined(__i386__)
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
#endif

#if defined(__ARM_NEON) || defined(__aarch64__)
typedef float float32x4_t __attribute__((vector_size(16)));
typedef int int32x4_t __attribute__((vector_size(16)));
#endif

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Volatile sink to prevent elimination */
static volatile int sink = 0;

/* Pattern A: Complex vector shuffle with many operands (x86 SSE) */
NOINLINE
#if defined(__SSE__) || defined(__x86_64__) || defined(__i386__)
v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d, 
                       int imm1, int imm2, int imm3, int imm4) {
    /* Complex shuffle pattern that may expand to many operands */
    v4sf t1 = __builtin_ia32_shufps(a, b, imm1);
    v4sf t2 = __builtin_ia32_shufps(c, d, imm2);
    v4sf t3 = __builtin_ia32_shufps(t1, t2, imm3);
    v4sf result = __builtin_ia32_shufps(t3, a, imm4);
    
    /* Mix with arithmetic to create dependencies */
    result = result + a * b - c / d;
    return result;
}
#else
v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d,
                       int imm1, int imm2, int imm3, int imm4) {
    return a; /* Dummy for non-x86 */
}
#endif

/* Pattern B: Fused multiply-add chain (generic) */
NOINLINE
float pattern_b_fma_chain(float a, float b, float c, float d, 
                         float e, float f, float g, float h,
                         float i, float j, float k, float l) {
    /* Deep FMA chain that may flatten to many operands */
    float t1 = __builtin_fma(a, b, c);
    float t2 = __builtin_fma(d, e, f);
    float t3 = __builtin_fma(g, h, i);
    float t4 = __builtin_fma(j, k, l);
    
    float t5 = __builtin_fma(t1, t2, t3);
    float result = __builtin_fma(t4, t5, a + b + c + d);
    
    /* Additional arithmetic to increase operand count */
    result = result * 2.0f - (t1 + t2) / (t3 * t4);
    return result;
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE
#if defined(__SSE__) || defined(__x86_64__) || defined(__i386__)
float pattern_c_vector_reduce(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Manually extract and sum all lanes - creates many extract operations */
    float sum = 0.0f;
    
    /* Extract each element (creates many operands during expansion) */
    sum += ((float*)&v1)[0] + ((float*)&v1)[1] + ((float*)&v1)[2] + ((float*)&v1)[3];
    sum += ((float*)&v2)[0] + ((float*)&v2)[1] + ((float*)&v2)[2] + ((float*)&v2)[3];
    sum += ((float*)&v3)[0] + ((float*)&v3)[1] + ((float*)&v3)[2] + ((float*)&v3)[3];
    sum += ((float*)&v4)[0] + ((float*)&v4)[1] + ((float*)&v4)[2] + ((float*)&v4)[3];
    
    /* Mix with vector operations */
    v4sf temp = v1 + v2 * v3 - v4;
    sum += ((float*)&temp)[0] + ((float*)&temp)[1];
    
    return sum;
}
#else
float pattern_c_vector_reduce(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    return 0.0f;
}
#endif

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE
#if defined(__SSE__) || defined(__x86_64__) || defined(__i386__)
v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c, v4sf d,
                                  v4sf e, v4sf f, v4sf mask1, v4sf mask2) {
    /* Complex conditional select with multiple comparisons */
    v4sf cmp1 = __builtin_ia32_cmpps(a, b, 0);  /* EQ */
    v4sf cmp2 = __builtin_ia32_cmpps(c, d, 1);  /* LT */
    v4sf cmp3 = __builtin_ia32_cmpps(e, f, 2);  /* LE */
    
    /* Combine masks - each operation adds operands */
    v4sf mask = __builtin_ia32_andps(cmp1, cmp2);
    mask = __builtin_ia32_orps(mask, cmp3);
    mask = __builtin_ia32_andps(mask, mask1);
    mask = __builtin_ia32_orps(mask, mask2);
    
    /* Select based on complex mask */
    v4sf result = __builtin_ia32_andps(a, mask);
    v4sf not_mask = __builtin_ia32_andnps(mask, mask);
    result = __builtin_ia32_orps(result, __builtin_ia32_andps(b, not_mask));
    
    /* Additional arithmetic */
    result = result + c * d - e / f;
    return result;
}
#else
v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c, v4sf d,
                                  v4sf e, v4sf f, v4sf mask1, v4sf mask2) {
    return a;
}
#endif

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE
int pattern_e_inline_asm(int a, int b, int c, int d, int e,
                         int f, int g, int h, int i, int j) {
    int result1, result2;
    
    /* Inline asm with 11 total operands (2 outputs, 9 inputs) */
    asm volatile (
        "add %[r1], %[a], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "add %[r1], %[r1], %[d]\n\t"
        "add %[r2], %[e], %[f]\n\t"
        "add %[r2], %[r2], %[g]\n\t"
        "mul %[r1], %[r1], %[r2]\n\t"
        "add %[r1], %[r1], %[h]\n\t"
        "add %[r1], %[r1], %[i]\n\t"
        "sub %[r1], %[r1], %[j]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2;
}

/* Pattern F: Vector blend with complex immediate calculation */
NOINLINE
#if defined(__SSE4_1__) || defined(__AVX__)
v4sf pattern_f_complex_blend(v4sf a, v4sf b, v4sf c, v4sf d,
                             int imm1, int imm2, int imm3, int imm4,
                             int imm5, int imm6) {
    /* Complex blend chain - each blend may expand to many operands */
    v4sf t1 = __builtin_ia32_blendps(a, b, imm1);
    v4sf t2 = __builtin_ia32_blendps(c, d, imm2);
    v4sf t3 = __builtin_ia32_blendps(t1, t2, imm3);
    v4sf t4 = __builtin_ia32_blendps(a, c, imm4);
    v4sf t5 = __builtin_ia32_blendps(b, d, imm5);
    v4sf result = __builtin_ia32_blendps(t3, t4, imm6);
    
    /* Additional operations to increase complexity */
    result = result + t5 * a - b / c + d;
    return result;
}
#else
v4sf pattern_f_complex_blend(v4sf a, v4sf b, v4sf c, v4sf d,
                             int imm1, int imm2, int imm3, int imm4,
                             int imm5, int imm6) {
    return a;
}
#endif

/* Main test function that exercises all patterns */
NOINLINE
int test_all_patterns(int argc, char **argv) {
    int checksum = 0;
    
    /* Initialize vectors with argc-dependent values */
    v4sf v1 = {argc * 1.0f, argc * 2.0f, argc * 3.0f, argc * 4.0f};
    v4sf v2 = {argc * 5.0f, argc * 6.0f, argc * 7.0f, argc * 8.0f};
    v4sf v3 = {argc * 9.0f, argc * 10.0f, argc * 11.0f, argc * 12.0f};
    v4sf v4 = {argc * 13.0f, argc * 14.0f, argc * 15.0f, argc * 16.0f};
    v4sf mask1 = {1.0f, 0.0f, 1.0f, 0.0f};
    v4sf mask2 = {0.0f, 1.0f, 0.0f, 1.0f};
    
    /* Execute different patterns based on argc to ensure all are compiled */
    if (argc > 1) {
        /* Pattern A: Complex shuffle */
        v4sf r1 = pattern_a_shuffle(v1, v2, v3, v4, 0x1B, 0x27, 0x39, 0x4E);
        checksum += (int)(((float*)&r1)[0] * 100);
        
        /* Pattern B: FMA chain */
        float r2 = pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f,
                                       7.7f, 8.8f, 9.9f, 10.1f, 11.1f, 12.2f);
        checksum += (int)(r2 * 10);
    }
    
    if (argc > 2) {
        /* Pattern C: Vector reduction */
        float r3 = pattern_c_vector_reduce(v1, v2, v3, v4);
        checksum += (int)(r3 * 5);
        
        /* Pattern D: Conditional select */
        v4sf r4 = pattern_d_conditional_select(v1, v2, v3, v4, v1, v2, mask1, mask2);
        checksum += (int)(((float*)&r4)[1] * 50);
    }
    
    if (argc > 3) {
        /* Pattern E: Inline asm with many operands */
        int r5 = pattern_e_inline_asm(argc, argc+1, argc+2, argc+3, argc+4,
                                      argc+5, argc+6, argc+7, argc+8, argc+9);
        checksum += r5;
        
        /* Pattern F: Complex blend */
        v4sf r6 = pattern_f_complex_blend(v1, v2, v3, v4, 0x5, 0xA, 0x3, 0xC, 0x9, 0x6);
        checksum += (int)(((float*)&r6)[2] * 25);
    }
    
    /* Use volatile sink to prevent elimination */
    sink = checksum;
    return checksum;
}

int main(int argc, char **argv) {
    /* Run the test multiple times with different argc values */
    int result = 0;
    
    /* Test with minimal args (some patterns) */
    result += test_all_patterns(1, argv);
    
    /* Test with more args (more patterns) */
    result += test_all_patterns(3, argv);
    
    /* Test with all patterns */
    result += test_all_patterns(5, argv);
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;  /* Return 0 on non-zero result */
}
