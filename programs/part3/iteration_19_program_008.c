/* test_optabs_10_11_operands.c
 * 
 * This program attempts to trigger GCC's RTL expansion for instructions
 * with 10 or 11 operands, covering the uncovered lines in optabs.cc.
 * 
 * Compile with: gcc -O2 -fdump-rtl-expand -c test_optabs_10_11_operands.c
 * Check the generated .expand file for multi-operand RTL patterns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to multi-operand RTL */
NOINLINE static uint64_t multi_operand_arithmetic(uint64_t a, uint64_t b, 
                                                  uint64_t c, uint64_t d,
                                                  uint64_t e, uint64_t f,
                                                  uint64_t g, uint64_t h,
                                                  uint64_t i, uint64_t j) {
    /* Complex expression that might require many temporaries */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Mix them in a way that prevents optimization */
    return ((t1 + t2) * (t3 + t4)) ^ t5;
}

/* Multi-precision arithmetic - often expands to complex RTL */
NOINLINE static __int128 multi_precision_mul(uint64_t a, uint64_t b, 
                                            uint64_t c, uint64_t d) {
    /* 128-bit multiplication using 64-bit parts */
    __int128 result = (__int128)a * b;
    result += (__int128)c * d;
    return result;
}

/* Vector operations that might expand to many operands */
NOINLINE static v4si vector_permute(v4si a, v4si b, v4si c, v4si mask) {
    /* Complex vector permutation */
    v4si t1 = a + b;
    v4si t2 = b + c;
    v4si t3 = a * c;
    
    /* Manual permutation based on mask */
    v4si result;
    for (int i = 0; i < 4; i++) {
        int idx = mask[i] & 3;
        switch (idx) {
            case 0: result[i] = t1[i]; break;
            case 1: result[i] = t2[i]; break;
            case 2: result[i] = t3[i]; break;
            default: result[i] = a[i] + b[i] + c[i];
        }
    }
    return result;
}

#ifdef __x86_64__
/* x86-specific inline assembly with many operands */
NOINLINE static uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b,
                                              uint64_t c, uint64_t d,
                                              uint64_t e, uint64_t f,
                                              uint64_t g, uint64_t h) {
    uint64_t result1, result2;
    
    /* Extended inline assembly with many input/output operands */
    asm volatile (
        /* Complex operation using many registers */
        "mov %[a], %%rax\n\t"
        "imul %[b], %%rax\n\t"
        "mov %[c], %%rbx\n\t"
        "imul %[d], %%rbx\n\t"
        "add %%rbx, %%rax\n\t"
        "mov %[e], %%rcx\n\t"
        "imul %[f], %%rcx\n\t"
        "add %%rcx, %%rax\n\t"
        "mov %[g], %%rdx\n\t"
        "imul %[h], %%rdx\n\t"
        "add %%rdx, %%rax\n\t"
        "mov %%rax, %[out1]\n\t"
        "xor %%rax, %%rax\n\t"
        "cpuid\n\t"
        "mov %%rbx, %[out2]"
        : [out1] "=r" (result1), [out2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    return result1 ^ result2;
}

/* Use x86 intrinsics for complex operations */
#include <x86intrin.h>
NOINLINE static __m128i x86_simd_ops(__m128i a, __m128i b, __m128i c,
                                     __m128i d, __m128i e) {
    /* Chain of SIMD operations that might expand to many operands */
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_mullo_epi32(c, d);
    __m128i t3 = _mm_slli_epi32(e, 3);
    __m128i t4 = _mm_blendv_epi8(t1, t2, t3);
    __m128i t5 = _mm_srai_epi32(t4, 1);
    
    return _mm_xor_si128(t5, _mm_set1_epi32(0x55555555));
}
#endif

#ifdef __aarch64__
/* ARM-specific inline assembly */
NOINLINE static uint64_t arm_multi_operand_asm(uint64_t a, uint64_t b,
                                              uint64_t c, uint64_t d,
                                              uint64_t e, uint64_t f) {
    uint64_t result;
    
    /* ARM inline assembly with multiple operands */
    asm volatile (
        "mul %x[res], %x[a], %x[b]\n\t"
        "madd %x[res], %x[c], %x[d], %x[res]\n\t"
        "msub %x[res], %x[e], %x[f], %x[res]\n\t"
        "eor %x[res], %x[res], #0xFF\n\t"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f)
        : "cc"
    );
    
    return result;
}
#endif

/* Test function that combines multiple expansion strategies */
NOINLINE static uint64_t combined_test(uint64_t base, int variant) {
    uint64_t result = base;
    
    /* Create many variables to use as operands */
    uint64_t a = base + 1;
    uint64_t b = base + 2;
    uint64_t c = base + 3;
    uint64_t d = base + 4;
    uint64_t e = base + 5;
    uint64_t f = base + 6;
    uint64_t g = base + 7;
    uint64_t h = base + 8;
    uint64_t i = base + 9;
    uint64_t j = base + 10;
    
    switch (variant % 4) {
        case 0:
            /* Complex arithmetic expression */
            result = multi_operand_arithmetic(a, b, c, d, e, f, g, h, i, j);
            break;
            
        case 1:
            /* Multi-precision arithmetic */
            __int128 mp = multi_precision_mul(a, b, c, d);
            result = (uint64_t)(mp >> 64) ^ (uint64_t)mp;
            break;
            
#ifdef __x86_64__
        case 2:
            /* x86-specific operations */
            result = x86_multi_operand_asm(a, b, c, d, e, f, g, h);
            break;
            
        case 3:
            /* x86 SIMD operations */
            __m128i va = _mm_set_epi64x(a, b);
            __m128i vb = _mm_set_epi64x(c, d);
            __m128i vc = _mm_set_epi64x(e, f);
            __m128i vd = _mm_set_epi64x(g, h);
            __m128i ve = _mm_set_epi64x(i, j);
            __m128i vr = x86_simd_ops(va, vb, vc, vd, ve);
            result = _mm_extract_epi64(vr, 0) ^ _mm_extract_epi64(vr, 1);
            break;
#endif
            
#ifdef __aarch64__
        case 2:
            /* ARM-specific operations */
            result = arm_multi_operand_asm(a, b, c, d, e, f);
            break;
#endif
            
        default:
            /* Fallback: mix all variables */
            result = a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j;
    }
    
    /* Additional mixing to prevent dead code elimination */
    return result * 0x9e3779b97f4a7c15ULL;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t accumulator = 0;
    
    /* Use command-line arguments to vary the test */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int variant = (argc > 2) ? atoi(argv[2]) : 0;
    
    printf("Testing multi-operand RTL expansion with %d iterations...\n", iterations);
    
    /* Loop to increase coverage chance */
    for (int n = 0; n < iterations; n++) {
        /* Vary the input to test different code paths */
        uint64_t base = (uint64_t)n * 0x123456789ABCDEFULL;
        
        /* Call test function with different variants */
        uint64_t result = combined_test(base, variant + n);
        
        /* Mix results to prevent optimization */
        accumulator ^= result;
        
        /* Occasionally call different test functions directly */
        if (n % 7 == 0) {
            v4si vec_a = {n, n+1, n+2, n+3};
            v4si vec_b = {n+4, n+5, n+6, n+7};
            v4si vec_c = {n+8, n+9, n+10, n+11};
            v4si mask = {0, 1, 2, 3};
            v4si vec_result = vector_permute(vec_a, vec_b, vec_c, mask);
            
            /* Use vector result to affect accumulator */
            for (int i = 0; i < 4; i++) {
                accumulator += vec_result[i];
            }
        }
    }
    
    printf("Final accumulator value: 0x%016llx\n", (unsigned long long)accumulator);
    return (int)(accumulator & 0x7FFFFFFF);
}
