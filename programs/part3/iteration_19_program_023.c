/* Test program to trigger 10/11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimization and inlining */
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
    
    /* Mix them in a way that prevents optimization */
    return t1 + t2 - t3 * t4 / (t5 + 1);
}

/* Vector operations that might expand to many operands */
NOINLINE v4si vector_multi_op(v4si a, v4si b, v4si c, v4si d,
                              v4si e, v4si f, v4si g, v4si h) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = d * e - f;
    v4si t3 = g * h;
    v4si t4 = t1 >> 2;
    v4si t5 = t2 << 1;
    
    return (t3 + t4) * t5;
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

/* x86-specific inline assembly with many operands */
NOINLINE uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i,
                                        uint64_t j) {
    uint64_t result1, result2, result3;
    
    /* Extended inline assembly with many operands */
    asm volatile (
        /* Complex multi-step operation using many registers */
        "mov %[a], %%rax\n\t"
        "mul %[b]\n\t"
        "mov %%rax, %[r1]\n\t"
        "mov %%rdx, %[r2]\n\t"
        "mov %[c], %%rax\n\t"
        "mul %[d]\n\t"
        "add %%rax, %[r1]\n\t"
        "adc %%rdx, %[r2]\n\t"
        "mov %[e], %%rax\n\t"
        "mul %[f]\n\t"
        "sub %%rax, %[r1]\n\t"
        "sbb %%rdx, %[r2]\n\t"
        "mov %[g], %%rax\n\t"
        "mul %[h]\n\t"
        "xor %%rax, %[r1]\n\t"
        "xor %%rdx, %[r2]\n\t"
        "mov %[i], %%rax\n\t"
        "mul %[j]\n\t"
        "and %%rax, %[r1]\n\t"
        "and %%rdx, %[r2]\n\t"
        "mov %[r1], %%rax\n\t"
        "add %[r2], %%rax\n\t"
        "mov %%rax, %[r3]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "rax", "rdx", "cc", "memory"
    );
    
    return result3;
}

/* AVX-512 intrinsic that might expand to many operands */
NOINLINE __m512i avx512_multi_op(__m512i a, __m512i b, __m512i c,
                                 __m512i d, __m512i e, __m512i f) {
    /* Complex AVX-512 expression with masking */
    __mmask16 mask = 0xAAAA;
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_sub_epi32(c, d);
    __m512i t3 = _mm512_mullo_epi32(e, f);
    __m512i t4 = _mm512_slli_epi32(t1, 2);
    __m512i t5 = _mm512_srli_epi32(t2, 1);
    
    return _mm512_mask_add_epi32(t3, mask, t4, t5);
}

#elif defined(__aarch64__)
#include <arm_neon.h>

/* ARM-specific NEON operations */
NOINLINE uint64x2_t arm_multi_op(uint64x2_t a, uint64x2_t b, uint64x2_t c,
                                 uint64x2_t d, uint64x2_t e, uint64x2_t f,
                                 uint64x2_t g, uint64x2_t h) {
    /* Complex NEON expression */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vsubq_u64(c, d);
    uint64x2_t t3 = vmulq_u64(e, f);
    uint64x2_t t4 = vshlq_u64(g, vdupq_n_u64(2));
    uint64x2_t t5 = vshrq_n_u64(h, 1);
    
    return vaddq_u64(vaddq_u64(t1, t2), vaddq_u64(t3, vaddq_u64(t4, t5)));
}

/* ARM inline assembly with many operands */
NOINLINE uint64_t arm_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i,
                                        uint64_t j) {
    uint64_t result;
    
    asm volatile (
        /* Complex ARM64 assembly with many operands */
        "mul %x[res], %x[a], %x[b]\n\t"
        "madd %x[res], %x[c], %x[d], %x[res]\n\t"
        "msub %x[res], %x[e], %x[f], %x[res]\n\t"
        "umulh %x[a], %x[g], %x[h]\n\t"
        "add %x[res], %x[res], %x[a]\n\t"
        "umulh %x[b], %x[i], %x[j]\n\t"
        "sub %x[res], %x[res], %x[b]"
        : [res] "=&r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result;
}

#elif defined(__powerpc64__) || defined(__PPC64__)
/* PowerPC-specific operations */
NOINLINE unsigned long ppc_multi_operand(unsigned long a, unsigned long b,
                                         unsigned long c, unsigned long d,
                                         unsigned long e, unsigned long f,
                                         unsigned long g, unsigned long h,
                                         unsigned long i, unsigned long j) {
    unsigned long result;
    
    /* PowerPC inline assembly with many operands */
    asm volatile (
        "mulld %0, %1, %2\n\t"
        "mulhdu %3, %4, %5\n\t"
        "add %0, %0, %3\n\t"
        "mulld %3, %6, %7\n\t"
        "sub %0, %0, %3\n\t"
        "mulhdu %3, %8, %9\n\t"
        "xor %0, %0, %3"
        : "=&r" (result), "+r" (a), "+r" (b), "=&r" (c)
        : "r" (d), "r" (e), "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    return result;
}
#endif

/* Function that mixes different operations to trigger various expansions */
NOINLINE uint64_t mixed_operations(int argc, char **argv) {
    uint64_t result = 0;
    
    /* Initialize many variables to use as operands */
    uint64_t vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = (uint64_t)(argc + i) * 123456789;
    }
    
    /* Call multi-operand arithmetic function */
    result += multi_operand_arithmetic(vars[0], vars[1], vars[2], vars[3],
                                       vars[4], vars[5], vars[6], vars[7],
                                       vars[8], vars[9]);
    
    /* Target-specific operations */
#ifdef __x86_64__
    if (argc > 1) {
        result += x86_multi_operand_asm(vars[0], vars[1], vars[2], vars[3],
                                        vars[4], vars[5], vars[6], vars[7],
                                        vars[8], vars[9]);
        
        /* Initialize vectors for AVX-512 */
        __m512i vecs[6];
        for (int i = 0; i < 6; i++) {
            int32_t data[16];
            for (int j = 0; j < 16; j++) data[j] = i * 16 + j + argc;
            vecs[i] = _mm512_loadu_si512(data);
        }
        
        __m512i avx_result = avx512_multi_op(vecs[0], vecs[1], vecs[2],
                                             vecs[3], vecs[4], vecs[5]);
        int32_t avx_data[16];
        _mm512_storeu_si512(avx_data, avx_result);
        result += avx_data[0];
    }
#elif defined(__aarch64__)
    if (argc > 2) {
        result += arm_multi_operand_asm(vars[0], vars[1], vars[2], vars[3],
                                        vars[4], vars[5], vars[6], vars[7],
                                        vars[8], vars[9]);
        
        /* NEON vector operations */
        uint64x2_t neon_vecs[8];
        for (int i = 0; i < 8; i++) {
            uint64_t data[2] = {vars[i*2], vars[i*2+1]};
            neon_vecs[i] = vld1q_u64(data);
        }
        
        uint64x2_t neon_result = arm_multi_op(neon_vecs[0], neon_vecs[1],
                                              neon_vecs[2], neon_vecs[3],
                                              neon_vecs[4], neon_vecs[5],
                                              neon_vecs[6], neon_vecs[7]);
        uint64_t neon_data[2];
        vst1q_u64(neon_data, neon_result);
        result += neon_data[0] + neon_data[1];
    }
#elif defined(__powerpc64__) || defined(__PPC64__)
    if (argc > 3) {
        result += ppc_multi_operand(vars[0], vars[1], vars[2], vars[3],
                                    vars[4], vars[5], vars[6], vars[7],
                                    vars[8], vars[9]);
    }
#endif
    
    /* Vector operations using GCC extensions */
    v4si vec_a = {vars[0], vars[1], vars[2], vars[3]};
    v4si vec_b = {vars[4], vars[5], vars[6], vars[7]};
    v4si vec_c = {vars[8], vars[9], vars[10], vars[11]};
    v4si vec_d = {vars[12], vars[13], vars[14], vars[15]};
    v4si vec_e = {vars[16], vars[17], vars[18], vars[19]};
    v4si vec_f = {1, 2, 3, 4};
    v4si vec_g = {5, 6, 7, 8};
    v4si vec_h = {9, 10, 11, 12};
    
    v4si vec_result = vector_multi_op(vec_a, vec_b, vec_c, vec_d,
                                      vec_e, vec_f, vec_g, vec_h);
    
    /* Use the result to prevent dead code elimination */
    int *vec_ptr = (int*)&vec_result;
    for (int i = 0; i < 4; i++) {
        result += vec_ptr[i];
    }
    
    return result;
}

int main(int argc, char **argv) {
    uint64_t total_result = 0;
    
    /* Loop with different inputs to trigger different code paths */
    for (int iteration = 0; iteration < (argc > 0 ? argc : 1); iteration++) {
        /* Modify argc slightly for each iteration to vary inputs */
        int modified_argc = argc + iteration;
        
        /* Call the function with many operands */
        total_result += mixed_operations(modified_argc, argv);
        
        /* Additional complex arithmetic that might expand to many operands */
        uint64_t a = total_result + 1;
        uint64_t b = total_result + 2;
        uint64_t c = total_result + 3;
        uint64_t d = total_result + 4;
        uint64_t e = total_result + 5;
        uint64_t f = total_result + 6;
        uint64_t g = total_result + 7;
        uint64_t h = total_result + 8;
        uint64_t i = total_result + 9;
        uint64_t j = total_result + 10;
        
        /* Another complex expression */
        uint64_t temp = ((a * b) + (c * d) - (e * f)) / ((g * h) + (i * j) + 1);
        total_result ^= temp;
    }
    
    printf("Result: %lu\n", (unsigned long)total_result);
    return (int)(total_result & 0xFF);
}
