#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Simple PRNG for deterministic testing */
static uint32_t seed = 123456789;
static inline uint32_t fast_rand(void) {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(__m256i* arr_i32, __m256i* arr_i16, 
                       __m256d* arr_f64, __m256* arr_f32, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr_i32[i] = _mm256_set_epi32(
            fast_rand(), fast_rand(), fast_rand(), fast_rand(),
            fast_rand(), fast_rand(), fast_rand(), fast_rand()
        );
        arr_i16[i] = _mm256_set_epi16(
            fast_rand(), fast_rand(), fast_rand(), fast_rand(),
            fast_rand(), fast_rand(), fast_rand(), fast_rand(),
            fast_rand(), fast_rand(), fast_rand(), fast_rand(),
            fast_rand(), fast_rand(), fast_rand(), fast_rand()
        );
        arr_f64[i] = _mm256_set_pd(
            (double)fast_rand() / 1000.0,
            (double)fast_rand() / 1000.0,
            (double)fast_rand() / 1000.0,
            (double)fast_rand() / 1000.0
        );
        arr_f32[i] = _mm256_set_ps(
            (float)fast_rand() / 1000.0f,
            (float)fast_rand() / 1000.0f,
            (float)fast_rand() / 1000.0f,
            (float)fast_rand() / 1000.0f,
            (float)fast_rand() / 1000.0f,
            (float)fast_rand() / 1000.0f,
            (float)fast_rand() / 1000.0f,
            (float)fast_rand() / 1000.0f
        );
    }
}

/* Complex expression with many temporaries to force expander work */
static inline __m256i create_complex_mask(int idx) {
    volatile int v_idx = idx; /* Prevent constant propagation */
    
    /* Multi-statement expression with many temporaries */
    char c1 = (v_idx >> 0) & 0xFF;
    short s1 = (v_idx >> 8) & 0xFFFF;
    int i1 = v_idx * 1103515245;
    long l1 = (long)v_idx * 123456789;
    
    /* Complex pointer arithmetic */
    char* ptr = (char*)&v_idx;
    short* sptr = (short*)ptr;
    int* iptr = (int*)ptr;
    
    /* Many intermediate operations */
    int t1 = c1 + *ptr;
    int t2 = s1 + *sptr;
    int t3 = i1 + *iptr;
    long t4 = l1 + (long)*iptr;
    int t5 = t1 ^ t2;
    int t6 = t3 ^ (int)t4;
    int t7 = t5 * t6;
    int t8 = t7 + v_idx;
    int t9 = t8 << 3;
    int t10 = t9 | 0x7;
    
    /* Create mask from computed values */
    return _mm256_set_epi32(t10, t9, t8, t7, t6, t5, t4 & 0xFFFFFFFF, t3);
}

/* Function with target attribute - prevents inlining */
__attribute__((target("avx2,avx512f"), noinline, optimize("no-tree-vectorize")))
static void test_many_args(__m256i* out, __m256i* in1, __m256i* in2, 
                          __m256d* in3, __m256* in4, size_t n) {
    volatile size_t counter = 0; /* Prevent loop unrolling */
    
    for (size_t i = 0; i < n; i++) {
        counter = i; /* Volatile write to inhibit optimizations */
        
        /* Load multiple vectors */
        __m256i v1 = in1[i];
        __m256i v2 = in2[i];
        __m256d v3 = in3[i];
        __m256 v4 = in4[i];
        
        /* Create complex mask with many arguments */
        __m256i mask = create_complex_mask((int)i);
        
#ifdef __AVX512F__
        /* AVX-512 specific: Use mask registers and blending with many arguments */
        __mmask8 k1 = _mm256_cmpeq_epi32_mask(v1, mask);
        __mmask8 k2 = _mm256_cmpeq_epi64_mask(v1, _mm256_srli_epi64(mask, 2));
        
        /* Complex shuffle with many arguments - may trigger 10-11 arg expansion */
        __m256i shuffled = _mm256_mask_shuffle_epi32(
            v1,                    /* src */
            k1,                    /* mask */
            v2,                    /* a */
            _MM_SHUFFLE(3, 2, 1, 0) /* imm8 */
        );
        
        /* Blend with multiple sources - 10+ arguments when expanded */
        __m256i blended = _mm256_mask_blend_epi32(
            k2,                    /* mask */
            shuffled,              /* a */
            mask                   /* b */
        );
        
        /* Store result */
        out[i] = blended;
#else
        /* AVX2 fallback: Use inline asm with many operands */
        __m256i result;
        
        /* Extended asm with 11 operands - may trigger optab expansion */
        asm volatile (
            "vpblendvb %[mask], %[src1], %[src2], %[out]\n\t"
            "vpshufd $0x1B, %[out], %[out]\n\t"  /* Additional shuffle */
            "vpaddd %[add1], %[out], %[out]\n\t"
            "vpxor %[xor1], %[out], %[out]\n\t"
            : [out] "=x" (result)
            : [src1] "x" (v1),
              [src2] "x" (v2),
              [mask] "x" (mask),
              [add1] "xm" (_mm256_set1_epi32(0x12345678)),
              [xor1] "xm" (_mm256_set1_epi32(0x87654321)),
              "m" (*in1),  /* Memory constraint */
              "m" (*in2),
              "m" (*in3),
              "m" (*in4),
              "i" (0x1B)   /* Immediate */
            : "memory"
        );
        
        out[i] = result;
#endif
        
        /* Additional complex operation chain */
        if (i > 0) {
            /* Multi-argument builtin-like operation */
            __m256i prev = out[i-1];
            
            /* Create dependency chain with many arguments */
            asm volatile (
                "vpmaddwd %[a], %[b], %[tmp]\n\t"
                "vpaddd %[tmp], %[c], %[tmp]\n\t"
                "vpsubd %[d], %[tmp], %[tmp]\n\t"
                "vpmulld %[e], %[tmp], %[tmp]\n\t"
                "vpor %[f], %[tmp], %[out]\n\t"
                : [out] "+x" (out[i]),
                  [tmp] "=&x" (result)
                : [a] "xm" (prev),
                  [b] "xm" (_mm256_set1_epi16(0x55)),
                  [c] "xm" (_mm256_set1_epi32(0x33333333)),
                  [d] "xm" (_mm256_set1_epi32(0x11111111)),
                  [e] "xm" (_mm256_set1_epi32(0x77777777)),
                  [f] "xm" (_mm256_set1_epi32(0xAAAAAAAA))
                : "memory"
            );
        }
    }
}

/* Compute checksum for validation */
static uint64_t compute_checksum(__m256i* arr, size_t n) {
    uint64_t checksum = 0;
    union {
        __m256i vec;
        uint32_t elems[8];
    } u;
    
    for (size_t i = 0; i < n; i++) {
        u.vec = arr[i];
        for (int j = 0; j < 8; j++) {
            checksum = checksum * 31 + u.elems[j];
        }
    }
    return checksum;
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    const size_t VEC_SIZE = ARRAY_SIZE / 8; /* 8 ints per __m256i */
    
    /* Allocate aligned memory for vector arrays */
    __m256i* arr_i32 = (__m256i*)_mm_malloc(sizeof(__m256i) * VEC_SIZE, 32);
    __m256i* arr_i16 = (__m256i*)_mm_malloc(sizeof(__m256i) * VEC_SIZE, 32);
    __m256d* arr_f64 = (__m256d*)_mm_malloc(sizeof(__m256d) * VEC_SIZE, 32);
    __m256* arr_f32 = (__m256*)_mm_malloc(sizeof(__m256) * VEC_SIZE, 32);
    __m256i* output = (__m256i*)_mm_malloc(sizeof(__m256i) * VEC_SIZE, 32);
    
    if (!arr_i32 || !arr_i16 || !arr_f64 || !arr_f32 || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_arrays(arr_i32, arr_i16, arr_f64, arr_f32, VEC_SIZE);
    memset(output, 0, sizeof(__m256i) * VEC_SIZE);
    
    /* Run the test function */
    test_many_args(output, arr_i32, arr_i16, arr_f64, arr_f32, VEC_SIZE);
    
    /* Compute and print checksum */
    uint64_t checksum = compute_checksum(output, VEC_SIZE);
    printf("Checksum: 0x%016llX\n", (unsigned long long)checksum);
    
    /* Cleanup */
    _mm_free(arr_i32);
    _mm_free(arr_i16);
    _mm_free(arr_f64);
    _mm_free(arr_f32);
    _mm_free(output);
    
    return 0;
}
