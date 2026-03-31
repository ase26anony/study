/* Test program to trigger 10/11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations from removing our test code */
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
    
    /* Mix them all together */
    return t1 + t2 * t3 + t4 / (t5 + 1);
}

/* Vector operations that might expand to many operands */
NOINLINE v4si vector_ops(v4si a, v4si b, v4si c, v4si d) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = d * a - b;
    v4si t3 = c * d + a;
    v4si t4 = b * c - d;
    
    return (t1 & t2) | (t3 ^ t4);
}

#ifdef __x86_64__
#include <x86intrin.h>

NOINLINE __m128i x86_multi_operand_intrinsic(__m128i a, __m128i b, __m128i c,
                                            __m128i d, __m128i e) {
    /* Chain of intrinsics that might expand to many operands */
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_sub_epi32(c, d);
    __m128i t3 = _mm_mullo_epi32(t1, t2);
    __m128i t4 = _mm_slli_epi32(e, 3);
    __m128i t5 = _mm_srli_epi32(t3, 1);
    
    return _mm_or_si128(t4, t5);
}

/* Inline assembly with many operands */
NOINLINE uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                       uint64_t d, uint64_t e, uint64_t f,
                                       uint64_t g, uint64_t h, uint64_t i) {
    uint64_t result1, result2;
    
    /* Extended inline assembly with many operands */
    asm volatile (
        "mov %[a], %%rax\n\t"
        "imul %[b], %%rax\n\t"
        "mov %%rax, %[r1]\n\t"
        "mov %[c], %%rbx\n\t"
        "imul %[d], %%rbx\n\t"
        "add %%rbx, %%rax\n\t"
        "mov %[e], %%rcx\n\t"
        "imul %[f], %%rcx\n\t"
        "add %%rcx, %%rax\n\t"
        "mov %[g], %%rdx\n\t"
        "imul %[h], %%rdx\n\t"
        "add %%rdx, %%rax\n\t"
        "mov %[i], %%rsi\n\t"
        "add %%rsi, %%rax\n\t"
        "mov %%rax, %[r2]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "rax", "rbx", "rcx", "rdx", "rsi", "cc"
    );
    
    return result1 + result2;
}
#endif

#ifdef __aarch64__
#include <arm_neon.h>

NOINLINE int32x4_t arm_multi_operand_intrinsic(int32x4_t a, int32x4_t b,
                                              int32x4_t c, int32x4_t d) {
    /* ARM NEON intrinsics that might use many operands */
    int32x4_t t1 = vaddq_s32(a, b);
    int32x4_t t2 = vsubq_s32(c, d);
    int32x4_t t3 = vmulq_s32(t1, t2);
    int32x4_t t4 = vshlq_s32(a, vdupq_n_s32(2));
    int32x4_t t5 = vrshrq_n_s32(t3, 1);
    
    return vorrq_s32(t4, t5);
}
#endif

/* Function that tries to trigger multi-operand expansion through
   complex constant division - this often expands to many operations */
NOINLINE uint64_t complex_division(uint64_t a, uint64_t b) {
    /* Division by non-power-of-two constant often expands to
       multiplication with magic constants and shifts */
    uint64_t t1 = a / 7ULL;
    uint64_t t2 = b / 13ULL;
    uint64_t t3 = (a + b) / 19ULL;
    uint64_t t4 = (a - b) / 23ULL;
    
    return t1 * t2 + t3 / t4;
}

/* Mixed operations to increase chances of hitting the target */
NOINLINE uint64_t mixed_operations(uint64_t a, uint64_t b, uint64_t c,
                                  uint64_t d, uint64_t e) {
    uint64_t result = 0;
    
    for (int i = 0; i < 4; i++) {
        /* Various operations that might expand differently */
        uint64_t t = (a * b + c) >> (d & 0x1F);
        uint64_t u = (e * a - b) << (c & 0x1F);
        uint64_t v = (d * e + a) / (b + 1);
        
        result += t ^ u ^ v;
        
        /* Rotate values to create different patterns */
        uint64_t tmp = a;
        a = b;
        b = c;
        c = d;
        d = e;
        e = tmp;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize with non-zero values to avoid constant folding */
    uint64_t vals[10];
    for (int i = 0; i < 10; i++) {
        vals[i] = (uint64_t)(argc + i + 1) * 0x123456789ABCDEFULL;
    }
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Complex arithmetic */
        result += multi_operand_arithmetic(vals[0], vals[1], vals[2],
                                          vals[3], vals[4], vals[5],
                                          vals[6], vals[7], vals[8],
                                          vals[9]);
        
        /* Path 2: Mixed operations */
        result += mixed_operations(vals[0], vals[1], vals[2],
                                  vals[3], vals[4]);
        
        /* Path 3: Complex division patterns */
        result += complex_division(vals[0], vals[1]);
    } else {
        /* Alternative path with vector operations */
        v4si vec_a = {vals[0], vals[1], vals[2], vals[3]};
        v4si vec_b = {vals[4], vals[5], vals[6], vals[7]};
        v4si vec_c = {vals[8], vals[9], vals[0], vals[1]};
        v4si vec_d = {vals[2], vals[3], vals[4], vals[5]};
        
        v4si vec_result = vector_ops(vec_a, vec_b, vec_c, vec_d);
        
        /* Sum vector elements */
        for (int i = 0; i < 4; i++) {
            result += vec_result[i];
        }
        
        #ifdef __x86_64__
        /* Try x86-specific paths */
        __m128i xmm_a = _mm_set_epi64x(vals[0], vals[1]);
        __m128i xmm_b = _mm_set_epi64x(vals[2], vals[3]);
        __m128i xmm_c = _mm_set_epi64x(vals[4], vals[5]);
        __m128i xmm_d = _mm_set_epi64x(vals[6], vals[7]);
        __m128i xmm_e = _mm_set_epi64x(vals[8], vals[9]);
        
        __m128i xmm_result = x86_multi_operand_intrinsic(xmm_a, xmm_b,
                                                        xmm_c, xmm_d, xmm_e);
        
        /* Add to result */
        uint64_t xmm_arr[2];
        _mm_storeu_si128((__m128i*)xmm_arr, xmm_result);
        result += xmm_arr[0] + xmm_arr[1];
        
        /* Try inline assembly path */
        result += x86_multi_operand_asm(vals[0], vals[1], vals[2],
                                       vals[3], vals[4], vals[5],
                                       vals[6], vals[7], vals[8]);
        #endif
        
        #ifdef __aarch64__
        /* Try ARM-specific paths */
        int32x4_t neon_a = vdupq_n_s32(vals[0]);
        int32x4_t neon_b = vdupq_n_s32(vals[1]);
        int32x4_t neon_c = vdupq_n_s32(vals[2]);
        int32x4_t neon_d = vdupq_n_s32(vals[3]);
        
        int32x4_t neon_result = arm_multi_operand_intrinsic(neon_a, neon_b,
                                                           neon_c, neon_d);
        
        /* Add to result */
        int32_t neon_arr[4];
        vst1q_s32(neon_arr, neon_result);
        for (int i = 0; i < 4; i++) {
            result += neon_arr[i];
        }
        #endif
    }
    
    /* Loop with varying inputs to explore different expansion paths */
    for (int i = 0; i < (argc % 10); i++) {
        uint64_t a = vals[i % 10] + i;
        uint64_t b = vals[(i + 1) % 10] + i * 2;
        uint64_t c = vals[(i + 2) % 10] + i * 3;
        
        result += complex_division(a, b);
        result += (a * b + c) >> 5;
    }
    
    printf("Result: %lu\n", (unsigned long)result);
    return 0;
}
