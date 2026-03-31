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

/* Prevent optimization and ensure expansion */
#define NOINLINE __attribute__((noinline, noipa, used))
#define VOLATILE_USE(x) do { volatile __typeof(x) _v = (x); (void)_v; } while(0)

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE static void pattern_a_vector_shuffle(int argc) {
#if defined(__SSE__) || defined(__x86_64__) || defined(__i386__)
    v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf b = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Complex shuffle pattern that may expand to many operands */
    v4sf result;
    
    /* Use different shuffle patterns based on argc to avoid constant propagation */
    int mask = argc & 0xFF;
    
    /* __builtin_ia32_shufps takes 3 args, but expansion may create many operands */
    result = __builtin_ia32_shufps(a, b, mask);
    
    /* Chain multiple shuffles to increase operand count in expansion */
    v4sf c = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf d = {13.0f, 14.0f, 15.0f, 16.0f};
    
    v4sf result2 = __builtin_ia32_shufps(result, c, (mask >> 1) & 0xFF);
    v4sf result3 = __builtin_ia32_shufps(result2, d, (mask >> 2) & 0xFF);
    
    VOLATILE_USE(result3);
#endif
}

/* Pattern B: Fused multiply-add chain */
NOINLINE static float pattern_b_fma_chain(float a, float b, float c, 
                                          float d, float e, float f,
                                          float g, float h, float i,
                                          float j, int argc) {
    float result = 0.0f;
    
#if defined(__FMA__) || defined(__AVX2__)
    /* Chain of FMA operations creating deep expression tree */
    result = __builtin_fmaf(a, b, c);
    result = __builtin_fmaf(result, d, e);
    result = __builtin_fmaf(result, f, g);
    result = __builtin_fmaf(result, h, i);
    result = __builtin_fmaf(result, j, argc * 1.0f);
#else
    /* Manual FMA simulation */
    result = a * b + c;
    result = result * d + e;
    result = result * f + g;
    result = result * h + i;
    result = result * j + argc * 1.0f;
#endif
    
    return result;
}

/* Pattern C: Vector extraction and horizontal sum */
NOINLINE static float pattern_c_vector_extract_sum(int argc) {
    float sum = 0.0f;
    
#if defined(__SSE__) || defined(__x86_64__) || defined(__i386__)
    v4sf v1 = {1.0f + argc, 2.0f + argc, 3.0f + argc, 4.0f + argc};
    v4sf v2 = {5.0f + argc, 6.0f + argc, 7.0f + argc, 8.0f + argc};
    
    /* Manual extraction and sum - creates many extract operations */
    sum += ((float*)&v1)[0] + ((float*)&v1)[1] + ((float*)&v1)[2] + ((float*)&v1)[3];
    sum += ((float*)&v2)[0] + ((float*)&v2)[1] + ((float*)&v2)[2] + ((float*)&v2)[3];
    
    /* Alternative using shuffle and add */
    v4sf vsum = v1 + v2;
    v4sf shuf = __builtin_ia32_shufps(vsum, vsum, 0x1B);
    v4sf sum2 = vsum + shuf;
    shuf = __builtin_ia32_shufps(sum2, sum2, 0x01);
    sum2 = sum2 + shuf;
    
    sum += ((float*)&sum2)[0];
#endif
    
    return sum;
}

/* Pattern D: Vector conditional select with complex mask */
NOINLINE static void pattern_d_vector_conditional(int argc) {
#if defined(__SSE__) || defined(__x86_64__) || defined(__i386__)
    v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf b = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf c = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf d = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Create comparison mask from multiple operations */
    v4sf cmp1 = a < b;
    v4sf cmp2 = c > d;
    
    /* Complex mask combination */
    v4sf mask = cmp1 & cmp2;
    
    /* Conditional select using mask */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        ((float*)&result)[i] = ((float*)&mask)[i] ? 
                              ((float*)&a)[i] + ((float*)&c)[i] : 
                              ((float*)&b)[i] + ((float*)&d)[i];
    }
    
    /* Use argc to vary the result */
    if (argc > 1) {
        result = result * (argc & 0xF);
    }
    
    VOLATILE_USE(result);
#endif
}

/* Pattern E: Inline assembly with many operands */
NOINLINE static int pattern_e_multi_operand_asm(int a, int b, int c, int d,
                                                int e, int f, int g, int h,
                                                int i, int j, int k) {
    int result1, result2, result3;
    
    /* Inline assembly with 11 operands (10 inputs + 1 output) */
    asm volatile (
        "add %[r1], %[a], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "add %[r2], %[d], %[e]\n\t"
        "add %[r2], %[r2], %[f]\n\t"
        "add %[r3], %[g], %[h]\n\t"
        "add %[r3], %[r3], %[i]\n\t"
        "mul %[out], %[r1], %[r2]\n\t"
        "add %[out], %[out], %[r3]\n\t"
        "add %[out], %[out], %[j]\n\t"
        "add %[out], %[out], %[k]"
        : [out] "=r" (result1),
          [r1] "=&r" (result2),
          [r2] "=&r" (result3)
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
        : "cc"
    );
    
    return result1;
}

/* Pattern F: AVX2 gather operation simulation */
NOINLINE static void pattern_f_gather_operation(int argc) {
#if defined(__AVX2__) || defined(__AVX__)
    v8sf src = {0};
    v8si index = {0};
    float base[32] = {0};
    v8sf mask = {0};
    
    /* Initialize with argc-dependent values */
    for (int i = 0; i < 8; i++) {
        ((float*)&src)[i] = i + argc;
        ((int*)&index)[i] = (i * 2) & 0x1F;
        ((float*)&mask)[i] = (argc & (1 << i)) ? -1.0f : 0.0f;
    }
    
    for (int i = 0; i < 32; i++) {
        base[i] = i * 0.5f;
    }
    
    /* Simulate gather operation with many operands */
    v8sf result = {0};
    for (int i = 0; i < 8; i++) {
        int idx = ((int*)&index)[i];
        if (((float*)&mask)[i] < 0) {
            ((float*)&result)[i] = base[idx] + ((float*)&src)[i];
        }
    }
    
    VOLATILE_USE(result);
#endif
}

/* Main test driver */
int main(int argc, char **argv) {
    float checksum = 0.0f;
    
    /* Use argc to vary execution paths */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Execute all patterns */
    pattern_a_vector_shuffle(argc);
    
    checksum += pattern_b_fma_chain(
        1.1f + rand() % 10, 2.2f + rand() % 10,
        3.3f + rand() % 10, 4.4f + rand() % 10,
        5.5f + rand() % 10, 6.6f + rand() % 10,
        7.7f + rand() % 10, 8.8f + rand() % 10,
        9.9f + rand() % 10, 10.1f + rand() % 10,
        argc
    );
    
    checksum += pattern_c_vector_extract_sum(argc);
    
    pattern_d_vector_conditional(argc);
    
    checksum += pattern_e_multi_operand_asm(
        rand() % 100, rand() % 100, rand() % 100,
        rand() % 100, rand() % 100, rand() % 100,
        rand() % 100, rand() % 100, rand() % 100,
        rand() % 100, rand() % 100
    );
    
    pattern_f_gather_operation(argc);
    
    /* Final volatile use to prevent dead code elimination */
    VOLATILE_USE(checksum);
    
    printf("Checksum: %f\n", checksum);
    return (int)checksum % 256;
}
