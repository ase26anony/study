#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Simple PRNG for reproducible results */
static uint32_t seed = 123456789;
static inline uint32_t fast_rand(void) {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

/* Vector types for different architectures */
#ifdef __AVX512F__
#include <immintrin.h>
typedef __m512i v512i;
typedef __m512 v512f;
typedef __mmask16 kmask;
#elif defined(__AVX2__)
#include <immintrin.h>
typedef __m256i v256i;
typedef __m256 v256f;
#elif defined(__ARM_NEON)
#include <arm_neon.h>
typedef int32x4_t v128i;
typedef float32x4_t v128f;
#else
/* Fallback to generic types */
typedef int32_t v128i __attribute__((vector_size(16)));
typedef float v128f __attribute__((vector_size(16)));
#endif

/* Prevent optimization of arguments */
#define NOOPT __attribute__((noinline, noclone))
#define VOLATILE_VAR(var) asm volatile("" : "+r"(var))

/* Function with many arguments - targeting optab expansion */
NOOPT
#ifdef __AVX512F__
__attribute__((target("avx512f,avx512bw")))
static v512i complex_shuffle_10arg(v512i a, v512i b, v512i c, v512i d,
                                   v512i e, v512i f, v512i g, v512i h,
                                   int imm1, int imm2) {
    /* This should trigger 10-argument optab expansion */
    return _mm512_shuffle_i64x2(_mm512_permutex2var_epi32(a, b, c),
                               _mm512_permutex2var_epi32(d, e, f),
                               _mm512_permutex2var_epi32(g, h, _mm512_set1_epi32(imm1)),
                               imm2);
}
#elif defined(__AVX2__)
__attribute__((target("avx2")))
static v256i complex_shuffle_10arg(v256i a, v256i b, v256i c, v256i d,
                                   v256i e, v256i f, v256i g, v256i h,
                                   int imm1, int imm2, int imm3) {
    /* 11 arguments to target the uncovered case */
    v256i temp1 = _mm256_blend_epi32(a, b, imm1);
    v256i temp2 = _mm256_blend_epi32(c, d, imm2);
    v256i temp3 = _mm256_blend_epi32(e, f, imm3);
    
    /* Complex permutation chain */
    v256i perm1 = _mm256_permutevar8x32_epi32(temp1, temp2);
    v256i perm2 = _mm256_permutevar8x32_epi32(temp3, g);
    
    /* Final blend with all inputs used */
    return _mm256_blendv_epi8(perm1, perm2, h);
}
#endif

/* Inline assembly with many operands */
NOOPT
static inline uint64_t many_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i,
                                        uint64_t j, uint64_t k) {
    uint64_t result;
    /* 11-operand inline asm - should trigger optab expansion */
    asm volatile (
        "lea (%[a], %[b], 2), %[res]\n\t"
        "add %[c], %[res]\n\t"
        "imul %[d], %[res]\n\t"
        "add %[e], %[res]\n\t"
        "sub %[f], %[res]\n\t"
        "add %[g], %[res]\n\t"
        "xor %[h], %[res]\n\t"
        "add %[i], %[res]\n\t"
        "sub %[j], %[res]\n\t"
        "add %[k], %[res]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc", "memory"
    );
    return result;
}

/* Complex expression with many temporaries */
NOOPT
static uint64_t complex_multi_statement(uint64_t *arr, int idx1, int idx2,
                                        int idx3, int idx4, int idx5,
                                        int idx6, int idx7, int idx8,
                                        int idx9, int idx10) {
    /* Force many intermediate values */
    uint64_t t1 = arr[idx1] + arr[idx2];
    uint64_t t2 = arr[idx3] * arr[idx4];
    uint64_t t3 = arr[idx5] ^ arr[idx6];
    uint64_t t4 = arr[idx7] | arr[idx8];
    uint64_t t5 = arr[idx9] & arr[idx10];
    
    uint64_t t6 = (t1 << 3) | (t2 >> 5);
    uint64_t t7 = (t3 * t4) + t5;
    uint64_t t8 = (t6 ^ t7) * 0x9e3779b97f4a7c15ULL;
    
    /* Chain more operations */
    uint64_t t9 = t8 + (t1 * t3);
    uint64_t t10 = t9 - (t2 | t4);
    uint64_t t11 = t10 ^ (t5 << 7);
    
    /* Use all temporaries in final expression */
    return ((t11 * t1) >> (t2 & 63)) + 
           ((t3 * t4) ^ t5) - 
           ((t6 | t7) & t8) + 
           (t9 ^ t10);
}

/* Main test function */
NOOPT
#ifdef __AVX2__
__attribute__((target("avx2")))
#endif
static void test_many_args(uint64_t *output, uint64_t *input, int size) {
    volatile int i; /* Prevent loop unrolling */
    
    for (i = 0; i < size; i++) {
        VOLATILE_VAR(i);
        
        /* Method 1: Complex vector operations */
#ifdef __AVX2__
        if (i + 8 < size) {
            /* Load multiple vectors */
            v256i v1 = _mm256_loadu_si256((v256i*)&input[i]);
            v256i v2 = _mm256_loadu_si256((v256i*)&input[i+8]);
            v256i v3 = _mm256_loadu_si256((v256i*)&input[i+16]);
            v256i v4 = _mm256_loadu_si256((v256i*)&input[i+24]);
            v256i v5 = _mm256_loadu_si256((v256i*)&input[i+32]);
            v256i v6 = _mm256_loadu_si256((v256i*)&input[i+40]);
            v256i v7 = _mm256_loadu_si256((v256i*)&input[i+48]);
            v256i v8 = _mm256_loadu_si256((v256i*)&input[i+56]);
            
            /* Call function with many arguments */
            v256i result = complex_shuffle_10arg(v1, v2, v3, v4, v5, v6, v7, v8,
                                                (i & 0xFF), ((i >> 8) & 0xFF), 
                                                ((i >> 16) & 0xFF));
            
            _mm256_storeu_si256((v256i*)&output[i], result);
            i += 7; /* Skip ahead */
            continue;
        }
#endif
        
        /* Method 2: Inline assembly with many operands */
        uint64_t a = input[i];
        uint64_t b = input[(i + 1) % size];
        uint64_t c = input[(i + 2) % size];
        uint64_t d = input[(i + 3) % size];
        uint64_t e = input[(i + 4) % size];
        uint64_t f = input[(i + 5) % size];
        uint64_t g = input[(i + 6) % size];
        uint64_t h = input[(i + 7) % size];
        uint64_t j = input[(i + 8) % size];
        uint64_t k = input[(i + 9) % size];
        uint64_t l = input[(i + 10) % size];
        
        output[i] = many_operand_asm(a, b, c, d, e, f, g, h, j, k, l);
        
        /* Method 3: Complex multi-statement expression */
        if (i + 10 < size) {
            uint64_t temp = complex_multi_statement(input, 
                                                   i, (i+1)%size, (i+2)%size,
                                                   (i+3)%size, (i+4)%size,
                                                   (i+5)%size, (i+6)%size,
                                                   (i+7)%size, (i+8)%size,
                                                   (i+9)%size);
            output[i] ^= temp; /* Combine results */
        }
    }
}

int main(void) {
    const int SIZE = 1024;
    uint64_t *input = aligned_alloc(64, SIZE * sizeof(uint64_t));
    uint64_t *output = aligned_alloc(64, SIZE * sizeof(uint64_t));
    
    if (!input || !output) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        input[i] = ((uint64_t)fast_rand() << 32) | fast_rand();
        output[i] = 0;
    }
    
    /* Run the test */
    test_many_args(output, input, SIZE);
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= output[i];
        checksum = (checksum << 13) | (checksum >> 51);
        checksum += 0x9e3779b97f4a7c15ULL;
    }
    
    printf("Checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    free(input);
    free(output);
    return 0;
}
