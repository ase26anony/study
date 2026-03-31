/* Test program to trigger 10/11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
#define NOINLINE __attribute__((noinline, noipa))

/* Different target-specific implementations */
#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>
#elif defined(__aarch64__)
#include <arm_neon.h>
#endif

/* Complex type that might expand to multiple operations */
typedef struct {
    long long a, b, c, d;
} quad_word;

/* Vector types for potential multi-operand expansion */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Function that attempts to create 10-operand expansion */
NOINLINE static quad_word ten_operand_op(long long a, long long b, 
                                         long long c, long long d,
                                         long long e, long long f,
                                         long long g, long long h,
                                         long long i, long long j) {
    quad_word result;
    
    /* Complex arithmetic that might expand to multi-operand RTL */
    result.a = (a * b) + (c * d) + (e * f) + (g * h) + (i * j);
    result.b = (a + b) * (c + d) * (e + f) * (g + h) * (i + j);
    
    /* Multi-precision calculation */
    unsigned long long carry1, carry2;
    unsigned long long t1 = (unsigned long long)a * b;
    unsigned long long t2 = (unsigned long long)c * d;
    unsigned long long t3 = (unsigned long long)e * f;
    unsigned long long t4 = (unsigned long long)g * h;
    unsigned long long t5 = (unsigned long long)i * j;
    
    /* Chain of additions with carries */
    unsigned long long sum = t1 + t2;
    carry1 = sum < t1;
    sum += t3;
    carry2 = sum < t3;
    result.c = sum + t4 + t5;
    
    /* Complex bit manipulation */
    result.d = ((a & b) | (c & d)) ^ ((e & f) | (g & h)) ^ (i & j);
    
    return result;
}

/* Function that might trigger 11-operand expansion through inline asm */
NOINLINE static long long eleven_operand_asm(long long op1, long long op2,
                                            long long op3, long long op4,
                                            long long op5, long long op6,
                                            long long op7, long long op8,
                                            long long op9, long long op10,
                                            long long op11) {
    long long result;
    
#ifdef __x86_64__
    /* Extended inline assembly with many operands */
    asm volatile (
        "imulq %[a], %[b]\n\t"
        "addq %[c], %[b]\n\t"
        "subq %[d], %[b]\n\t"
        "xorq %[e], %[b]\n\t"
        "orq %[f], %[b]\n\t"
        "andq %[g], %[b]\n\t"
        "shlq $3, %[b]\n\t"
        "shrq $1, %[b]\n\t"
        "addq %[h], %[b]\n\t"
        "subq %[i], %[b]\n\t"
        "movq %[b], %[res]"
        : [res] "=r" (result)
        : [a] "r" (op1), [b] "r" (op2), [c] "r" (op3),
          [d] "r" (op4), [e] "r" (op5), [f] "r" (op6),
          [g] "r" (op7), [h] "r" (op8), [i] "r" (op9),
          [j] "r" (op10), [k] "r" (op11)
        : "cc"
    );
#elif defined(__aarch64__)
    /* ARM-specific inline assembly */
    asm volatile (
        "mul %x[res], %x[a], %x[b]\n\t"
        "add %x[res], %x[res], %x[c]\n\t"
        "sub %x[res], %x[res], %x[d]\n\t"
        "eor %x[res], %x[res], %x[e]\n\t"
        "orr %x[res], %x[res], %x[f]\n\t"
        "and %x[res], %x[res], %x[g]\n\t"
        "lsl %x[res], %x[res], #3\n\t"
        "lsr %x[res], %x[res], #1\n\t"
        "add %x[res], %x[res], %x[h]\n\t"
        "sub %x[res], %x[res], %x[i]\n\t"
        "add %x[res], %x[res], %x[j]\n\t"
        "sub %x[res], %x[res], %x[k]"
        : [res] "=r" (result)
        : [a] "r" (op1), [b] "r" (op2), [c] "r" (op3),
          [d] "r" (op4), [e] "r" (op5), [f] "r" (op6),
          [g] "r" (op7), [h] "r" (op8), [i] "r" (op9),
          [j] "r" (op10), [k] "r" (op11)
        : "cc"
    );
#else
    /* Generic fallback - complex expression that might expand */
    result = (((((((((op1 * op2) + op3) - op4) ^ op5) | op6) & op7) 
               << 3) >> 1) + op8) - op9 + op10 - op11;
#endif
    
    return result;
}

/* Vector operations that might create multi-operand patterns */
NOINLINE static v4si vector_multi_op(v4si a, v4si b, v4si c, v4si d,
                                     v4si e, v4si f, v4si g) {
    v4si result;
    
    /* Complex vector expression */
    result = (a * b) + (c * d) - (e & f) | g;
    
    /* Element-wise operations that might expand separately */
    result[0] = a[0] * b[0] + c[0] * d[0] - e[0] * f[0] + g[0];
    result[1] = a[1] * b[1] + c[1] * d[1] - e[1] * f[1] + g[1];
    result[2] = a[2] * b[2] + c[2] * d[2] - e[2] * f[2] + g[2];
    result[3] = a[3] * b[3] + c[3] * d[3] - e[3] * f[3] + g[3];
    
    return result;
}

/* Test function that uses builtins/intrinsics */
NOINLINE static void test_intrinsics(void) {
#ifdef __AVX512F__
    /* AVX-512 operations with many operands */
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    __m512i e = _mm512_set1_epi32(5);
    
    __mmask16 mask = 0xAAAA;
    __m512i result = _mm512_mask_add_epi32(a, mask, b, c);
    result = _mm512_mask_mul_epi32(result, mask, d, e);
    
    volatile __m512i sink = result;
    (void)sink;
#endif

#ifdef __ARM_NEON
    /* ARM NEON operations */
    int32x4_t va = vdupq_n_s32(1);
    int32x4_t vb = vdupq_n_s32(2);
    int32x4_t vc = vdupq_n_s32(3);
    int32x4_t vd = vdupq_n_s32(4);
    
    int32x4_t result = vmlaq_s32(va, vb, vc);
    result = vmlsq_s32(result, vd, va);
    
    volatile int32x4_t sink = result;
    (void)sink;
#endif
}

/* Main test driver */
int main(int argc, char **argv) {
    long long total = 0;
    
    /* Initialize operands with varying values */
    long long operands[20];
    for (int i = 0; i < 20; i++) {
        operands[i] = (long long)(argc + i) * (i + 1);
    }
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Test 10-operand expansion */
        for (int i = 0; i < (argc % 5); i++) {
            quad_word res = ten_operand_op(
                operands[0] + i, operands[1] + i, operands[2] + i,
                operands[3] + i, operands[4] + i, operands[5] + i,
                operands[6] + i, operands[7] + i, operands[8] + i,
                operands[9] + i
            );
            total += res.a + res.b + res.c + res.d;
        }
        
        /* Test vector operations */
        v4si vec_a = {operands[0], operands[1], operands[2], operands[3]};
        v4si vec_b = {operands[4], operands[5], operands[6], operands[7]};
        v4si vec_c = {operands[8], operands[9], operands[10], operands[11]};
        v4si vec_d = {operands[12], operands[13], operands[14], operands[15]};
        v4si vec_e = {operands[16], operands[17], operands[18], operands[19]};
        v4si vec_f = {1, 2, 3, 4};
        v4si vec_g = {5, 6, 7, 8};
        
        v4si vec_res = vector_multi_op(vec_a, vec_b, vec_c, vec_d, vec_e, vec_f, vec_g);
        for (int i = 0; i < 4; i++) {
            total += vec_res[i];
        }
    } else {
        /* Path 2: Test 11-operand expansion */
        for (int i = 0; i < (argc + 1); i++) {
            long long res = eleven_operand_asm(
                operands[0], operands[1], operands[2], operands[3],
                operands[4], operands[5], operands[6], operands[7],
                operands[8], operands[9], operands[10]
            );
            total += res;
        }
    }
    
    /* Test intrinsics if available */
    test_intrinsics();
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %lld\n", total);
    
    return (int)(total % 256);
}
