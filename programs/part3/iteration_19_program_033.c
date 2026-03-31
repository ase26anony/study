/* Test program to trigger 10/11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa))

/* Vector types for various architectures */
#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>
typedef __m256i v8si __attribute__((aligned(32)));
typedef __m128i v4si __attribute__((aligned(16)));
#elif defined(__aarch64__)
#include <arm_neon.h>
typedef int32x4_t v4si;
typedef int32x4x2_t v8si;
#else
/* Generic vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
#endif

/* Complex multi-operand inline assembly for x86 */
#ifdef __x86_64__
NOOPT static uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                           uint64_t d, uint64_t e, uint64_t f,
                                           uint64_t g, uint64_t h, uint64_t i,
                                           uint64_t j) {
    uint64_t result1, result2, result3;
    
    /* 10-operand inline assembly pattern */
    asm volatile (
        /* Complex multi-step operation using many registers */
        "mov %[a], %%rax\n\t"
        "add %[b], %%rax\n\t"
        "imul %[c], %%rax\n\t"
        "add %[d], %%rax\n\t"
        "sub %[e], %%rax\n\t"
        "xor %[f], %%rax\n\t"
        "or %[g], %%rax\n\t"
        "and %[h], %%rax\n\t"
        "add %[i], %%rax\n\t"
        "sub %[j], %%rax\n\t"
        "mov %%rax, %[out1]\n\t"
        
        /* Another operation chain */
        "mov %[b], %%rbx\n\t"
        "lea (%%rbx, %[c], 4), %%rbx\n\t"
        "add %[d], %%rbx\n\t"
        "sub %[e], %%rbx\n\t"
        "mov %%rbx, %[out2]\n\t"
        
        /* Third result */
        "mov %[a], %%rcx\n\t"
        "imul %[c], %%rcx\n\t"
        "add %[f], %%rcx\n\t"
        "mov %%rcx, %[out3]"
        
        : [out1] "=r" (result1),
          [out2] "=r" (result2),
          [out3] "=r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "rax", "rbx", "rcx", "cc", "memory"
    );
    
    return result1 + result2 + result3;
}

/* AVX-512 multi-operand intrinsic */
NOOPT static __m512i avx512_multi_operand(__m512i a, __m512i b, __m512i c,
                                         __m512i d, __m512i e, __m512i f,
                                         __m512i g, __m512i h, __m512i i,
                                         __m512i j, __mmask16 k) {
    /* Chain of AVX-512 operations that might expand to many operands */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_sub_epi32(c, d);
    __m512i t3 = _mm512_mullo_epi32(e, f);
    __m512i t4 = _mm512_and_si512(g, h);
    __m512i t5 = _mm512_or_si512(i, j);
    
    /* Masked blend operation with many operands */
    __m512i result = _mm512_mask_blend_epi32(k, t1, 
        _mm512_add_epi32(t2, 
            _mm512_sub_epi32(t3,
                _mm512_xor_si512(t4, t5))));
    
    return result;
}
#endif

/* ARM NEON multi-operand operations */
#ifdef __aarch64__
NOOPT static int32x4_t neon_multi_operand(int32x4_t a, int32x4_t b, int32x4_t c,
                                         int32x4_t d, int32x4_t e, int32x4_t f,
                                         int32x4_t g, int32x4_t h, int32x4_t i,
                                         int32x4_t j) {
    /* Complex NEON operation chain */
    int32x4_t t1 = vaddq_s32(a, b);
    int32x4_t t2 = vsubq_s32(c, d);
    int32x4_t t3 = vmulq_s32(e, f);
    int32x4_t t4 = vandq_s32(g, h);
    int32x4_t t5 = vorrq_s32(i, j);
    
    /* Fused multiply-add with lane (can have many operands) */
    int32x4_t result = vmlaq_s32(t1, t2, t3);
    result = vmlsq_s32(result, t4, t5);
    
    return result;
}
#endif

/* Generic multi-precision arithmetic that might expand to many operands */
NOOPT static uint64_t multi_precision_mul(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j) {
    /* Complex arithmetic expression that GCC might break into many operations */
    uint64_t t1 = (a * b) >> 32;      /* High part multiplication */
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Chain of operations that might require many temporary registers */
    uint64_t result = t1 + t2;
    result = result * t3;
    result = result - t4;
    result = result ^ t5;
    result = result | (a & b & c & d & e);
    result = result + (f | g | h | i | j);
    
    /* Division by non-power-of-two constant (expands to multiplication magic) */
    result = result / 7;
    result = result % 13;
    
    return result;
}

/* Vector permutation with many operands */
NOOPT static v8si vector_permute_multi(v8si a, v8si b, v8si c, v8si d,
                                      v8si e, v8si f, v8si g, v8si h,
                                      v8si i, v8si j) {
    /* Complex vector manipulation */
    v8si t1 = a + b;
    v8si t2 = c - d;
    v8si t3 = e * f;
    v8si t4 = g & h;
    v8si t5 = i | j;
    
    /* Manual permutation by extracting and inserting elements */
    v8si result;
    
    /* This kind of element-wise manipulation might expand to many RTL operands */
    for (int idx = 0; idx < 8; idx++) {
        int elem = ((int*)&t1)[idx] + ((int*)&t2)[idx] * ((int*)&t3)[idx];
        elem = elem ^ ((int*)&t4)[idx] | ((int*)&t5)[idx];
        ((int*)&result)[idx] = elem;
    }
    
    return result;
}

/* Function that tries multiple expansion paths */
NOOPT static uint64_t test_multi_operand_expansion(int argc, char **argv) {
    uint64_t result = 0;
    
    /* Initialize many variables to use as operands */
    uint64_t vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = (uint64_t)(i + argc) * 123456789;
    }
    
    /* Try different code paths based on argc */
    if (argc > 10) {
        /* Path 1: Multi-precision arithmetic */
        result = multi_precision_mul(vars[0], vars[1], vars[2], vars[3],
                                    vars[4], vars[5], vars[6], vars[7],
                                    vars[8], vars[9]);
    } else if (argc > 5) {
        /* Path 2: Architecture-specific inline assembly */
#ifdef __x86_64__
        result = x86_multi_operand_asm(vars[0], vars[1], vars[2], vars[3],
                                      vars[4], vars[5], vars[6], vars[7],
                                      vars[8], vars[9]);
#endif
    } else {
        /* Path 3: Vector operations */
        v8si vecs[10];
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 8; j++) {
                ((int*)&vecs[i])[j] = (i * 8 + j) * argc;
            }
        }
        
        v8si vec_result = vector_permute_multi(vecs[0], vecs[1], vecs[2],
                                              vecs[3], vecs[4], vecs[5],
                                              vecs[6], vecs[7], vecs[8],
                                              vecs[9]);
        
        /* Sum vector elements */
        for (int j = 0; j < 8; j++) {
            result += ((int*)&vec_result)[j];
        }
    }
    
    /* Mix in more operations to influence register allocation */
    for (int i = 0; i < 10; i++) {
        result = (result << 3) | (result >> 61);  /* Rotate */
        result ^= vars[i];
        result += (vars[i] * vars[(i+1)%10]) >> 32;
    }
    
    return result;
}

/* Main function with multiple test iterations */
int main(int argc, char **argv) {
    uint64_t final_result = 0;
    
    /* Test with different input patterns */
    for (int iteration = 0; iteration < (argc > 1 ? atoi(argv[1]) : 10); iteration++) {
        /* Modify argc for different code paths */
        int test_argc = (iteration % 3) + 1;
        
        /* Call the multi-operand function */
        uint64_t iter_result = test_multi_operand_expansion(test_argc, argv);
        
        /* Accumulate results to prevent dead code elimination */
        final_result ^= iter_result;
        final_result = (final_result * 1103515245) + 12345;  /* Simple PRNG step */
        
        /* Force spill/reload by using many local variables */
        uint64_t temp_vars[20];
        for (int i = 0; i < 20; i++) {
            temp_vars[i] = iter_result + i;
            final_result += temp_vars[i];
        }
    }
    
    /* Print result to prevent optimization */
    printf("Result: %lu\n", (unsigned long)final_result);
    
    return (final_result > 0) ? 0 : 1;
}
