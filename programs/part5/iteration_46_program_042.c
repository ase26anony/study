#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Simple PRNG for deterministic testing */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Vector types for different architectures */
#ifdef __AVX512F__
#include <immintrin.h>
typedef __m512i v512i;
typedef __m512  v512f;
typedef __mmask16 mask16;
#elif defined(__AVX2__)
#include <immintrin.h>
typedef __m256i v256i;
typedef __m256  v256f;
#elif defined(__ARM_NEON)
#include <arm_neon.h>
typedef int32x4_t v128i;
typedef float32x4_t v128f;
#else
/* Fallback to generic vectors */
typedef int32_t v128i __attribute__((vector_size(16)));
typedef float v128f __attribute__((vector_size(16)));
#endif

/* Function attributes to prevent optimization */
#define NOINLINE __attribute__((noinline))
#define NOIPA __attribute__((noipa))
#define VOLATILE_VAR(var) asm volatile("" : "+r"(var))

/* Complex expression with many temporaries */
NOINLINE static int complex_multi_arg_expr(int a, int b, int c, int d, 
                                          int e, int f, int g, int h,
                                          int i, int j, int k) {
    /* Force many intermediate values */
    int t1 = a * b + c;
    int t2 = d ^ e | f;
    int t3 = g << 2;
    int t4 = h >> 1;
    int t5 = i & j;
    int t6 = t1 - t2;
    int t7 = t3 + t4;
    int t8 = t5 * t6;
    int t9 = t7 ^ t8;
    int t10 = k + t9;
    
    /* Use volatile to prevent CSE */
    VOLATILE_VAR(t10);
    
    return t10 * a - b + c * d - e + f * g - h + i * j - k;
}

/* Target-specific vector function with many arguments */
#ifdef __AVX512F__
NOINLINE __attribute__((target("avx512f,avx512vl")))
static v512i avx512_complex_shuffle(v512i a, v512i b, v512i c, v512i d,
                                   v512i e, v512i f, v512i g, v512i h,
                                   int imm1, int imm2, mask16 mask) {
    /* Complex shuffle with many arguments that might use optabs */
    v512i t1 = _mm512_add_epi32(a, b);
    v512i t2 = _mm512_sub_epi32(c, d);
    v512i t3 = _mm512_mullo_epi32(e, f);
    v512i t4 = _mm512_slli_epi32(g, imm1);
    v512i t5 = _mm512_srli_epi32(h, imm2);
    
    /* Blend with mask - many arguments */
    v512i result = _mm512_mask_blend_epi32(mask, t1, t2);
    
    /* Additional operations to create complex expression */
    result = _mm512_add_epi32(result, t3);
    result = _mm512_add_epi32(result, t4);
    result = _mm512_sub_epi32(result, t5);
    
    return result;
}
#endif

#ifdef __AVX2__
NOINLINE __attribute__((target("avx2")))
static v256i avx2_many_arg_operation(v256i a, v256i b, v256i c, v256i d,
                                    v256i e, v256i f, v256i g, v256i h,
                                    int imm1, int imm2, int imm3) {
    /* Create complex expression with many vector arguments */
    v256i t1 = _mm256_add_epi32(a, b);
    v256i t2 = _mm256_sub_epi32(c, d);
    v256i t3 = _mm256_mullo_epi32(e, f);
    v256i t4 = _mm256_slli_epi32(g, imm1);
    v256i t5 = _mm256_srli_epi32(h, imm2);
    
    /* Permute with many arguments - might trigger optab */
    v256i perm = _mm256_permutevar8x32_epi32(t1, 
        _mm256_set_epi32(imm3, 6, 5, 4, 3, 2, 1, 0));
    
    /* Blend multiple vectors */
    v256i blend1 = _mm256_blend_epi32(t2, t3, 0xCC);
    v256i blend2 = _mm256_blend_epi32(t4, t5, 0xAA);
    
    v256i result = _mm256_add_epi32(perm, blend1);
    result = _mm256_add_epi32(result, blend2);
    
    return result;
}
#endif

/* Inline assembly with many operands */
NOINLINE static void many_operand_asm(uint64_t *out, 
                                     uint64_t a, uint64_t b, uint64_t c,
                                     uint64_t d, uint64_t e, uint64_t f,
                                     uint64_t g, uint64_t h, uint64_t i,
                                     uint64_t j, uint64_t k) {
    /* Extended asm with 11 input operands */
    asm volatile (
        "mov %[a], %%rax\n\t"
        "add %[b], %%rax\n\t"
        "sub %[c], %%rax\n\t"
        "imul %[d], %%rax\n\t"
        "xor %[e], %%rax\n\t"
        "or %[f], %%rax\n\t"
        "and %[g], %%rax\n\t"
        "shl $3, %%rax\n\t"
        "shr $1, %%rax\n\t"
        "add %[h], %%rax\n\t"
        "sub %[i], %%rax\n\t"
        "mov %%rax, %[out]\n\t"
        : [out] "=m" (*out)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "rax", "memory", "cc"
    );
}

/* Main test function with hot loop */
NOINLINE __attribute__((target("avx2")))
static void test_many_args(int32_t *output, const int32_t *input, size_t n) {
    volatile size_t counter = n; /* Prevent loop unrolling */
    
    for (size_t i = 0; i < counter; i += 8) {
        /* Load multiple vectors */
        v256i vec0 = _mm256_loadu_si256((const v256i*)(input + i));
        v256i vec1 = _mm256_loadu_si256((const v256i*)(input + i + 8));
        v256i vec2 = _mm256_loadu_si256((const v256i*)(input + i + 16));
        v256i vec3 = _mm256_loadu_si256((const v256i*)(input + i + 24));
        v256i vec4 = _mm256_loadu_si256((const v256i*)(input + i + 32));
        v256i vec5 = _mm256_loadu_si256((const v256i*)(input + i + 40));
        v256i vec6 = _mm256_loadu_si256((const v256i*)(input + i + 48));
        v256i vec7 = _mm256_loadu_si256((const v256i*)(input + i + 56));
        
        /* Complex expression with many arguments */
        int imm1 = (i & 0xF) + 1;
        int imm2 = (i & 0x7) + 2;
        int imm3 = (i & 0x3) + 3;
        
        v256i result = avx2_many_arg_operation(vec0, vec1, vec2, vec3,
                                              vec4, vec5, vec6, vec7,
                                              imm1, imm2, imm3);
        
        /* Store result */
        _mm256_storeu_si256((v256i*)(output + i), result);
        
        /* Also test scalar many-argument function */
        int scalar_result = complex_multi_arg_expr(
            input[i], input[i+1], input[i+2], input[i+3],
            input[i+4], input[i+5], input[i+6], input[i+7],
            input[i+8], input[i+9], input[i+10]
        );
        
        /* Mix scalar result into output */
        output[i] ^= scalar_result;
        
        /* Test inline asm with many operands */
        uint64_t asm_out;
        many_operand_asm(&asm_out,
                        (uint64_t)input[i], (uint64_t)input[i+1],
                        (uint64_t)input[i+2], (uint64_t)input[i+3],
                        (uint64_t)input[i+4], (uint64_t)input[i+5],
                        (uint64_t)input[i+6], (uint64_t)input[i+7],
                        (uint64_t)input[i+8], (uint64_t)input[i+9],
                        (uint64_t)input[i+10]);
        
        output[i+1] ^= (int32_t)(asm_out & 0xFFFFFFFF);
    }
}

/* Alternative using GCC vector builtins with many arguments */
NOINLINE static v128i gcc_vector_shuffle(v128i a, v128i b, v128i c, v128i d,
                                        v128i e, v128i f, v128i g, v128i h,
                                        int idx1, int idx2) {
    /* Use __builtin_shuffle with complex pattern */
    typedef int v4si __attribute__((vector_size(16)));
    
    /* Create complex shuffle with many source vectors */
    v4si t1 = a + b;
    v4si t2 = c - d;
    v4si t3 = e * f;
    v4si t4 = g & h;
    
    /* Complex shuffle pattern - might require many arguments */
    v4si shuffled = __builtin_shuffle(t1, t2, 
        (v4si){idx1, 5, 2, 7});
    
    /* Additional conversion */
    v4si converted = __builtin_convertvector(shuffled, v4si);
    
    /* Blend operations */
    v4si result = converted + t3;
    result = result ^ t4;
    
    return result;
}

int main() {
    const size_t ARRAY_SIZE = 1024;
    int32_t *input = aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *output = aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        input[i] = (int32_t)prng_next();
    }
    
    /* Clear output */
    memset(output, 0, ARRAY_SIZE * sizeof(int32_t));
    
    /* Run test */
    test_many_args(output, input, ARRAY_SIZE);
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        checksum += (uint64_t)output[i];
        checksum = (checksum << 13) | (checksum >> (64 - 13)); /* Rotate */
    }
    
    printf("Checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    free(input);
    free(output);
    
    return 0;
}
