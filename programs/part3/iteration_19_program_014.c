/* test_multi_operand.c - Test program for GCC optabs.cc 10/11-operand expansion */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our test code */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOINLINE uint64_t multi_operand_arithmetic(uint64_t a, uint64_t b, uint64_t c,
                                           uint64_t d, uint64_t e, uint64_t f,
                                           uint64_t g, uint64_t h, uint64_t i,
                                           uint64_t j) {
    /* Complex expression that might require many temporary operands */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Multi-step calculation that might expand to many RTL operands */
    uint64_t result = ((t1 + t2) * (t3 + t4)) >> 32;
    result = (result * t5) >> 32;
    result = (result + a + b + c + d) >> 8;
    
    return result;
}

/* Vector operations that might expand to many operands */
NOINLINE v4si vector_multi_operand(v4si a, v4si b, v4si c, v4si d,
                                   v4si e, v4si f, v4si g) {
    /* Complex vector operations */
    v4si t1 = a * b + c;
    v4si t2 = d * e + f;
    v4si t3 = g * a + b;
    v4si t4 = c * d + e;
    
    v4si result = (t1 & t2) | (t3 ^ t4);
    result = result + (a >> 2) + (b >> 3) + (c >> 4);
    
    return result;
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

/* x86-specific multi-operand operations */
NOINLINE __m128i x86_multi_operand_intrinsic(__m128i a, __m128i b, __m128i c,
                                             __m128i d, __m128i e, __m128i f,
                                             __m128i g, __m128i h, __m128i i) {
    /* Chain of SSE/AVX operations that might require many operands */
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_sub_epi32(c, d);
    __m128i t3 = _mm_mullo_epi32(e, f);
    __m128i t4 = _mm_and_si128(g, h);
    __m128i t5 = _mm_or_si128(i, t1);
    
    __m128i result = _mm_add_epi32(t1, t2);
    result = _mm_add_epi32(result, t3);
    result = _mm_add_epi32(result, t4);
    result = _mm_add_epi32(result, t5);
    
    /* Additional operations to increase operand count */
    result = _mm_slli_epi32(result, 2);
    result = _mm_srli_epi32(result, 1);
    
    return result;
}

/* Extended inline assembly with many operands */
NOINLINE uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i,
                                        uint64_t j) {
    uint64_t result1, result2, result3;
    
    /* Inline assembly with many input/output operands */
    asm volatile (
        /* Complex multi-step operation using many registers */
        "mov %[a], %%rax\n\t"
        "imul %[b], %%rax\n\t"
        "mov %%rax, %[r1]\n\t"
        "mov %[c], %%rbx\n\t"
        "imul %[d], %%rbx\n\t"
        "add %%rbx, %[r1]\n\t"
        "mov %[e], %%rcx\n\t"
        "imul %[f], %%rcx\n\t"
        "mov %[g], %%rdx\n\t"
        "imul %[h], %%rdx\n\t"
        "add %%rcx, %%rdx\n\t"
        "mov %[i], %%rsi\n\t"
        "imul %[j], %%rsi\n\t"
        "add %%rsi, %%rdx\n\t"
        "mov %[r1], %%rax\n\t"
        "imul %%rdx, %%rax\n\t"
        "mov %%rax, %[r2]\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "rax", "rbx", "rcx", "rdx", "rsi", "cc", "memory"
    );
    
    return result1 + result2 + result3;
}

#elif defined(__aarch64__)
#include <arm_neon.h>

/* ARM-specific multi-operand operations */
NOINLINE uint64x2_t arm_multi_operand_intrinsic(uint64x2_t a, uint64x2_t b,
                                                uint64x2_t c, uint64x2_t d,
                                                uint64x2_t e, uint64x2_t f,
                                                uint64x2_t g, uint64x2_t h) {
    /* Chain of NEON operations */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vsubq_u64(c, d);
    uint64x2_t t3 = vmulq_u64(e, f);
    uint64x2_t t4 = vandq_u64(g, h);
    
    uint64x2_t result = vaddq_u64(t1, t2);
    result = vaddq_u64(result, t3);
    result = vaddq_u64(result, t4);
    result = vshlq_n_u64(result, 2);
    result = vshrq_n_u64(result, 1);
    
    return result;
}

/* ARM inline assembly with many operands */
NOINLINE uint64_t arm_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i,
                                        uint64_t j) {
    uint64_t result1, result2, result3;
    
    asm volatile (
        /* Complex ARM64 assembly with many registers */
        "mul %[r1], %[a], %[b]\n\t"
        "mul x10, %[c], %[d]\n\t"
        "add %[r1], %[r1], x10\n\t"
        "mul x11, %[e], %[f]\n\t"
        "mul x12, %[g], %[h]\n\t"
        "add x11, x11, x12\n\t"
        "mul x13, %[i], %[j]\n\t"
        "add x11, x11, x13\n\t"
        "mul %[r2], %[r1], x11\n\t"
        "mov %[r3], #0\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "x10", "x11", "x12", "x13", "cc"
    );
    
    return result1 + result2 + result3;
}

#elif defined(__powerpc64__) || defined(__PPC64__)
/* PowerPC-specific operations */
NOINLINE unsigned long long ppc_multi_operand(unsigned long long a,
                                              unsigned long long b,
                                              unsigned long long c,
                                              unsigned long long d,
                                              unsigned long long e,
                                              unsigned long long f,
                                              unsigned long long g,
                                              unsigned long long h,
                                              unsigned long long i,
                                              unsigned long long j) {
    unsigned long long result1, result2, result3;
    
    /* PowerPC inline assembly with many operands */
    asm volatile (
        "mulld %[r1], %[a], %[b]\n\t"
        "mulld 10, %[c], %[d]\n\t"
        "add %[r1], %[r1], 10\n\t"
        "mulld 11, %[e], %[f]\n\t"
        "mulld 12, %[g], %[h]\n\t"
        "add 11, 11, 12\n\t"
        "mulld 13, %[i], %[j]\n\t"
        "add 11, 11, 13\n\t"
        "mulld %[r2], %[r1], 11\n\t"
        "li %[r3], 0\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "r10", "r11", "r12", "r13", "cc"
    );
    
    return result1 + result2 + result3;
}
#endif

/* Test function that combines multiple expansion strategies */
NOINLINE uint64_t test_multi_operand_expansion(int variant, uint64_t seed) {
    uint64_t result = seed;
    
    /* Initialize many variables to use as operands */
    uint64_t a = seed + 1;
    uint64_t b = seed + 2;
    uint64_t c = seed + 3;
    uint64_t d = seed + 4;
    uint64_t e = seed + 5;
    uint64_t f = seed + 6;
    uint64_t g = seed + 7;
    uint64_t h = seed + 8;
    uint64_t i = seed + 9;
    uint64_t j = seed + 10;
    
    /* Vector variables */
    v4si va = {a, b, c, d};
    v4si vb = {e, f, g, h};
    v4si vc = {i, j, a, b};
    v4si vd = {c, d, e, f};
    v4si ve = {g, h, i, j};
    v4si vf = {a, c, e, g};
    v4si vg = {b, d, f, h};
    
    switch (variant % 4) {
        case 0:
            /* Complex arithmetic expression */
            result = multi_operand_arithmetic(a, b, c, d, e, f, g, h, i, j);
            break;
            
        case 1:
            /* Vector operations */
            {
                v4si vresult = vector_multi_operand(va, vb, vc, vd, ve, vf, vg);
                result = vresult[0] + vresult[1] + vresult[2] + vresult[3];
            }
            break;
            
#ifdef __x86_64__
        case 2:
            /* x86 intrinsics */
            {
                __m128i ma = _mm_set_epi64x(a, b);
                __m128i mb = _mm_set_epi64x(c, d);
                __m128i mc = _mm_set_epi64x(e, f);
                __m128i md = _mm_set_epi64x(g, h);
                __m128i me = _mm_set_epi64x(i, j);
                __m128i mf = _mm_set_epi64x(a+b, c+d);
                __m128i mg = _mm_set_epi64x(e+f, g+h);
                __m128i mh = _mm_set_epi64x(i+j, a+c);
                __m128i mi = _mm_set_epi64x(b+d, e+g);
                
                __m128i mresult = x86_multi_operand_intrinsic(ma, mb, mc, md,
                                                              me, mf, mg, mh, mi);
                uint64_t res[2];
                _mm_storeu_si128((__m128i*)res, mresult);
                result = res[0] + res[1];
            }
            break;
            
        case 3:
            /* x86 inline assembly */
            result = x86_multi_operand_asm(a, b, c, d, e, f, g, h, i, j);
            break;
#elif defined(__aarch64__)
        case 2:
            /* ARM intrinsics */
            {
                uint64x2_t ma = {a, b};
                uint64x2_t mb = {c, d};
                uint64x2_t mc = {e, f};
                uint64x2_t md = {g, h};
                uint64x2_t me = {i, j};
                uint64x2_t mf = {a+b, c+d};
                uint64x2_t mg = {e+f, g+h};
                uint64x2_t mh = {i+j, a+c};
                
                uint64x2_t mresult = arm_multi_operand_intrinsic(ma, mb, mc, md,
                                                                 me, mf, mg, mh);
                result = mresult[0] + mresult[1];
            }
            break;
            
        case 3:
            /* ARM inline assembly */
            result = arm_multi_operand_asm(a, b, c, d, e, f, g, h, i, j);
            break;
#elif defined(__powerpc64__) || defined(__PPC64__)
        case 2:
        case 3:
            /* PowerPC inline assembly */
            result = ppc_multi_operand(a, b, c, d, e, f, g, h, i, j);
            break;
#endif
        default:
            result = a + b + c + d + e + f + g + h + i + j;
            break;
    }
    
    /* Additional operations to prevent dead code elimination */
    result = (result * 1103515245 + 12345) & 0x7fffffff;
    
    return result;
}

int main(int argc, char *argv[]) {
    uint64_t final_result = 0;
    int iterations = 100;
    
    /* Use command line argument to control which paths are taken */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    printf("Testing multi-operand expansion with %d iterations...\n", iterations);
    
    /* Loop with different inputs to trigger various expansion paths */
    for (int i = 0; i < iterations; i++) {
        /* Test different variants based on loop counter */
        int variant = i % 4;
        uint64_t seed = (uint64_t)i * 123456789;
        
        uint64_t result = test_multi_operand_expansion(variant, seed);
        
        /* Accumulate results to prevent optimization */
        final_result += result;
        
        /* Occasionally print progress */
        if (i % 25 == 0) {
            printf("Iteration %d: result = %lu\n", i, (unsigned long)result);
        }
    }
    
    printf("Final accumulated result: %lu\n", (unsigned long)final_result);
    
    /* Return non-zero if final_result is 0 (unlikely) */
    return (final_result == 0) ? 1 : 0;
}
