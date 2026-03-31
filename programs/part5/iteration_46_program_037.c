#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

/* Simple PRNG for reproducible results */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(__m256i* arr1, __m256i* arr2, __m256i* arr3, 
                       __m256i* arr4, __m256i* arr5, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr1[i] = _mm256_set_epi32(prng_next(), prng_next(), prng_next(), prng_next(),
                                   prng_next(), prng_next(), prng_next(), prng_next());
        arr2[i] = _mm256_set_epi32(prng_next(), prng_next(), prng_next(), prng_next(),
                                   prng_next(), prng_next(), prng_next(), prng_next());
        arr3[i] = _mm256_set_epi32(prng_next(), prng_next(), prng_next(), prng_next(),
                                   prng_next(), prng_next(), prng_next(), prng_next());
        arr4[i] = _mm256_set_epi32(prng_next(), prng_next(), prng_next(), prng_next(),
                                   prng_next(), prng_next(), prng_next(), prng_next());
        arr5[i] = _mm256_set_epi32(prng_next(), prng_next(), prng_next(), prng_next(),
                                   prng_next(), prng_next(), prng_next(), prng_next());
    }
}

/* Complex expression with many temporaries - forces expander to create many operands */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args(__m256i* out, const __m256i* in1, const __m256i* in2,
                          const __m256i* in3, const __m256i* in4, const __m256i* in5,
                          size_t size) {
    /* Volatile counter to prevent loop unrolling */
    volatile size_t vcounter = 0;
    
    for (size_t i = 0; i < size; i++) {
        /* Load vectors with memory barriers to prevent optimization */
        __m256i v1, v2, v3, v4, v5;
        
        /* Use inline asm to create fake dependencies and prevent CSE */
        asm volatile("" : "+r"(i) : : "memory");
        
        v1 = _mm256_load_si256(&in1[i]);
        v2 = _mm256_load_si256(&in2[i]);
        v3 = _mm256_load_si256(&in3[i]);
        v4 = _mm256_load_si256(&in4[i]);
        v5 = _mm256_load_si256(&in5[i]);
        
        /* Complex multi-step operation creating many temporaries */
        __m256i t1 = _mm256_add_epi32(v1, v2);
        __m256i t2 = _mm256_sub_epi32(v3, v4);
        __m256i t3 = _mm256_mullo_epi32(t1, t2);
        __m256i t4 = _mm256_slli_epi32(v5, 3);
        __m256i t5 = _mm256_srli_epi32(v1, 2);
        __m256i t6 = _mm256_and_si256(t3, t4);
        __m256i t7 = _mm256_or_si256(t5, t6);
        __m256i t8 = _mm256_xor_si256(t7, v2);
        
        /* Extended inline asm with 11 operands - targets the uncovered optab case */
        __m256i result;
        asm volatile (
            "vpaddd %0, %1, %2\n\t"
            "vpsubd %0, %0, %3\n\t"
            "vpmulld %0, %0, %4\n\t"
            "vpslld $2, %5, %5\n\t"
            "vpor %0, %0, %5\n\t"
            "vpxor %0, %0, %6\n\t"
            "vpsrld $1, %0, %0\n\t"
            "vpblendd $0xAA, %0, %7, %0\n\t"
            "vpermq $0x4E, %0, %0\n\t"
            "vpaddd %0, %0, %8"
            : "=&x"(result)
            : "x"(t1), "x"(t2), "x"(t3), "x"(t4), 
              "x"(t5), "x"(t6), "x"(t7), "x"(t8),
              "m"(in1[i]), "m"(in2[i])  /* Memory operands to reach 11 total */
            : "memory"
        );
        
        /* Another approach: Use builtin with many arguments */
        /* This creates a complex shuffle-like operation with 10 arguments */
        uint64_t lane0 = i & 7;
        uint64_t lane1 = (i + 1) & 7;
        uint64_t lane2 = (i + 2) & 7;
        uint64_t lane3 = (i + 3) & 7;
        
        /* Complex expression that might expand to many-argument optab */
        __m256i shuffled = _mm256_set_epi64x(
            ((int64_t*)&t1)[lane3],
            ((int64_t*)&t2)[lane2],
            ((int64_t*)&t3)[lane1],
            ((int64_t*)&t4)[lane0]
        );
        
        /* Blend with many control inputs */
        __m256i final_result;
        asm volatile (
            "vblendvpd %0, %1, %2, %3\n\t"
            : "=x"(final_result)
            : "x"(result), "x"(shuffled), "x"(_mm256_set1_epi64x(0xAAAAAAAAAAAAAAAAULL)),
              "m"(in3[i]), "m"(in4[i]), "m"(in5[i]),  /* More memory operands */
              "i"(0xFF), "i"(0xAA), "i"(0x55), "i"(0x0F)  /* Immediate constants */
            : "memory"
        );
        
        _mm256_store_si256(&out[i], final_result);
        vcounter++;
    }
}

/* Alternative function using GCC vector extensions for more argument complexity */
#ifdef __GNUC__
typedef int32_t v8si __attribute__((vector_size(32)));

__attribute__((noinline, target("avx2")))
static v8si complex_vector_expr(v8si a, v8si b, v8si c, v8si d, v8si e,
                               v8si f, v8si g, v8si h, v8si i, v8si j) {
    /* This complex expression with many operands might trigger the 10-argument case */
    v8si t1 = a + b;
    v8si t2 = c - d;
    v8si t3 = e * f;
    v8si t4 = g & h;
    v8si t5 = i | j;
    
    /* Use __builtin_shuffle with many arguments */
    /* Note: Actual shuffle might need different syntax, but this shows the intent */
    v8si shuffled = __builtin_shuffle(t1, t2, 
        (v8si){0, 9, 2, 11, 4, 13, 6, 15});
    
    /* Complex blend operation */
    v8si result = __builtin_shufflevector(t3, t4, 
        0, 1, 2, 3, 12, 13, 14, 15);
    
    /* Final mix */
    return result + shuffled + t5;
}
#endif

/* Test with mixed scalar types to create complex RTL */
__attribute__((noinline))
static uint64_t test_mixed_types(char* cptr, short* sptr, int* iptr, 
                                long* lptr, size_t size) {
    uint64_t sum = 0;
    volatile size_t counter = 0;
    
    for (size_t idx = 0; idx < size; idx++) {
        /* Complex pointer arithmetic creating many temporaries */
        char* cp1 = cptr + idx;
        char* cp2 = cp1 + size;
        short* sp1 = sptr + idx;
        short* sp2 = sp1 + size;
        int* ip1 = iptr + idx;
        int* ip2 = ip1 + size;
        long* lp1 = lptr + idx;
        long* lp2 = lp1 + size;
        
        /* Multi-statement expression with many intermediate values */
        int t1 = *cp1 + *cp2;
        int t2 = *sp1 - *sp2;
        int t3 = *ip1 * *ip2;
        long t4 = *lp1 / (idx + 1);
        int t5 = t1 & t2;
        int t6 = t3 | t5;
        long t7 = t4 ^ t6;
        int t8 = (t1 << 3) + (t2 >> 2);
        long t9 = t7 * t8;
        int t10 = ~t6;
        long t11 = t9 - t10;
        
        /* Extended asm with 11 memory/register operands */
        long final_result;
        asm volatile (
            "add %1, %2\n\t"
            "sub %3, %4\n\t"
            "imul %5, %6\n\t"
            "xor %7, %8\n\t"
            "or %9, %10\n\t"
            "mov %0, %11"
            : "=r"(final_result)
            : "r"(t1), "r"(t2), "r"(t3), "r"(t4),
              "r"(t5), "r"(t6), "r"(t7), "r"(t8),
              "r"(t9), "r"(t10), "r"(t11),
              "m"(*cp1), "m"(*sp1)  /* Memory references */
            : "cc", "memory"
        );
        
        sum += final_result;
        counter++;
    }
    
    return sum;
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    const size_t VEC_SIZE = ARRAY_SIZE / 8;  /* 8 ints per __m256i */
    
    /* Allocate aligned memory for vector operations */
    __m256i* vec1 = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* vec2 = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* vec3 = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* vec4 = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* vec5 = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* out_vec = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    
    /* Allocate memory for mixed type test */
    char* char_arr = malloc(ARRAY_SIZE * sizeof(char));
    short* short_arr = malloc(ARRAY_SIZE * sizeof(short));
    int* int_arr = malloc(ARRAY_SIZE * sizeof(int));
    long* long_arr = malloc(ARRAY_SIZE * sizeof(long));
    
    if (!vec1 || !vec2 || !vec3 || !vec4 || !vec5 || !out_vec ||
        !char_arr || !short_arr || !int_arr || !long_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    init_arrays(vec1, vec2, vec3, vec4, vec5, VEC_SIZE);
    
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        char_arr[i] = prng_next() & 0xFF;
        short_arr[i] = prng_next() & 0xFFFF;
        int_arr[i] = prng_next();
        long_arr[i] = (long)prng_next() * prng_next();
    }
    
    printf("Testing many-argument optab expansion...\n");
    
    /* Test vector operations with many arguments */
    test_many_args(out_vec, vec1, vec2, vec3, vec4, vec5, VEC_SIZE);
    
    /* Test mixed types */
    uint64_t mixed_sum = test_mixed_types(char_arr, short_arr, int_arr, long_arr, ARRAY_SIZE / 4);
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (size_t i = 0; i < VEC_SIZE; i++) {
        int32_t* data = (int32_t*)&out_vec[i];
        for (int j = 0; j < 8; j++) {
            checksum += data[j];
        }
    }
    
    checksum += mixed_sum;
    
    printf("Final checksum: %lu\n", checksum);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(vec1);
    free(vec2);
    free(vec3);
    free(vec4);
    free(vec5);
    free(out_vec);
    free(char_arr);
    free(short_arr);
    free(int_arr);
    free(long_arr);
    
    return 0;
}
