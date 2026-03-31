#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

/* Simple PRNG for reproducible results */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(__m256i* vec_int, __m256i* vec_mask, 
                       __m256d* vec_double, __m256* vec_float,
                       int size) {
    for (int i = 0; i < size; i++) {
        uint32_t data[8];
        for (int j = 0; j < 8; j++) data[j] = prng_next();
        vec_int[i] = _mm256_loadu_si256((const __m256i*)data);
        
        for (int j = 0; j < 8; j++) data[j] = prng_next() & 0x7; /* Mask values 0-7 */
        vec_mask[i] = _mm256_loadu_si256((const __m256i*)data);
        
        double ddata[4];
        float fdata[8];
        for (int j = 0; j < 4; j++) ddata[j] = (double)(prng_next() % 1000) / 100.0;
        for (int j = 0; j < 8; j++) fdata[j] = (float)(prng_next() % 1000) / 100.0f;
        
        vec_double[i] = _mm256_loadu_pd(ddata);
        vec_float[i] = _mm256_loadu_ps(fdata);
    }
}

/* Function with many arguments to trigger optab expansion */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args(__m256i* output, const __m256i* input1, 
                          const __m256i* input2, const __m256i* masks,
                          const __m256d* doubles, const __m256* floats,
                          int size) {
    
    /* Volatile counter to prevent loop unrolling */
    volatile int volatile_counter = 0;
    
    for (int i = 0; i < size; i++) {
        volatile_counter = i;
        
        /* Load multiple vectors */
        __m256i v1 = _mm256_loadu_si256(&input1[i]);
        __m256i v2 = _mm256_loadu_si256(&input2[i]);
        __m256i mask = _mm256_loadu_si256(&masks[i]);
        __m256d d1 = _mm256_loadu_pd((const double*)&doubles[i]);
        __m256 f1 = _mm256_loadu_ps((const float*)&floats[i]);
        
        /* Create complex expression with many temporaries */
        __m256i temp1, temp2, temp3, temp4, temp5;
        __m256d dtemp1, dtemp2;
        __m256 ftemp1, ftemp2;
        
        /* Chain of operations creating many intermediate values */
        temp1 = _mm256_add_epi32(v1, v2);
        temp2 = _mm256_sub_epi32(v1, v2);
        temp3 = _mm256_mullo_epi32(temp1, temp2);
        
        /* Use inline asm with many operands (10-11 arguments) */
        /* This should trigger the optab expansion for many arguments */
        asm volatile (
            "vpaddd %0, %1, %2\n\t"
            "vpsubd %3, %1, %2\n\t"
            "vpmulld %4, %0, %3\n\t"
            "vpslld %5, %4, %6\n\t"
            "vpsrld %7, %4, %8\n\t"
            "vpor %9, %5, %7"
            : "=v"(temp4), "=v"(temp5), "+v"(temp1), "+v"(temp2),
              "+v"(temp3), "=v"(dtemp1), "=v"(dtemp2), "=v"(ftemp1),
              "=v"(ftemp2)
            : "v"(v1), "v"(v2), "v"(mask), "m"(doubles[i]), 
              "m"(floats[i]), "i"(2), "i"(1), "i"(3), "i"(4)
            : "memory"
        );
        
        /* Another complex operation with vector shuffle/blend */
        /* Using builtins that might expand to many-argument optabs */
        __m256i shuffled;
        
        /* Create a complex shuffle with many arguments through inline asm */
        /* 10 arguments: 2 input vectors, 8 immediate constants for shuffle control */
        asm volatile (
            "vpshufd $0x1B, %1, %0\n\t"      /* Shuffle with immediate */
            "vpermq $0x4E, %0, %0\n\t"       /* Another permute */
            "vpblendd $0xF0, %2, %0, %0"     /* Blend with immediate */
            : "=v"(shuffled)
            : "v"(temp3), "v"(temp4), "m"(input1[i]), "m"(input2[i]),
              "i"(0x1B), "i"(0x4E), "i"(0xF0), "i"(0xAA), "i"(0x55), "i"(0x33)
            : "memory"
        );
        
        /* Final operation with type conversion - another candidate for many arguments */
        __m256 converted;
        asm volatile (
            "vcvtdq2ps %1, %0\n\t"
            "vmulps %0, %2, %0"
            : "=v"(converted)
            : "v"(shuffled), "v"(f1), "m"(floats[i]), "m"(doubles[i]),
              "i"(0), "i"(1), "i"(2), "i"(3), "i"(4), "i"(5)
            : "memory"
        );
        
        /* Store result */
        _mm256_storeu_si256(&output[i], shuffled);
        
        /* Prevent CSE with volatile asm */
        asm volatile("" : "+v"(temp1), "+v"(temp2), "+v"(temp3));
    }
}

/* Alternative approach using GCC vector extensions */
#ifdef __GNUC__
typedef int32_t v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

__attribute__((noinline, target("avx2")))
static void test_vector_extensions(v8si* output, const v8si* input1,
                                  const v8si* input2, const v8si* masks,
                                  int size) {
    
    volatile int vol_idx = 0;
    
    for (int i = 0; i < size; i++) {
        vol_idx = i;
        
        v8si v1 = input1[i];
        v8si v2 = input2[i];
        v8si mask = masks[i];
        
        /* Complex expression with many operations */
        v8si result = (v1 + v2) * (v1 - v2);
        result = result << (mask & 0x7);
        result = result >> ((mask >> 3) & 0x7);
        
        /* Use __builtin_shuffle with many arguments */
        /* Create a complex shuffle pattern */
        int shuffle_pattern[8] = {7, 6, 5, 4, 3, 2, 1, 0};
        
        /* Force the compiler to handle many arguments by unrolling the shuffle */
        v8si shuffled = __builtin_shuffle(result, result, 
            shuffle_pattern[0], shuffle_pattern[1], shuffle_pattern[2],
            shuffle_pattern[3], shuffle_pattern[4], shuffle_pattern[5],
            shuffle_pattern[6], shuffle_pattern[7]);
        
        /* More operations to create complex RTL */
        shuffled = shuffled + __builtin_shuffle(v1, v2, 0, 1, 2, 3, 4, 5, 6, 7);
        shuffled = shuffled * __builtin_shuffle(v2, v1, 7, 6, 5, 4, 3, 2, 1, 0);
        
        output[i] = shuffled;
        
        /* Prevent optimization */
        asm volatile("" : "+v"(v1), "+v"(v2), "+v"(result));
    }
}
#endif

/* Test with AVX-512 if available */
#ifdef __AVX512F__
__attribute__((noinline, target("avx512f")))
static void test_avx512(__m512i* output, const __m512i* input1,
                       const __m512i* input2, int size) {
    
    for (int i = 0; i < size; i++) {
        __m512i v1 = _mm512_loadu_si512(&input1[i]);
        __m512i v2 = _mm512_loadu_si512(&input2[i]);
        
        /* AVX-512 has many operations with mask registers */
        /* Create a complex expression that might use many arguments */
        __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, v2);
        
        /* Blend operation with many arguments */
        __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
        
        /* Permute with many arguments through inline asm */
        __m512i permuted;
        asm volatile (
            "vpermq %0, %1, %2\n\t"
            "vpermd %0, %3, %0"
            : "=v"(permuted)
            : "v"(blended), "v"(v2), "v"(v1), "m"(input1[i]), "m"(input2[i]),
              "i"(0x01), "i"(0x23), "i"(0x45), "i"(0x67), "i"(0x89), "i"(0xAB)
            : "memory"
        );
        
        _mm512_storeu_si512(&output[i], permuted);
    }
}
#endif

int main() {
    const int SIZE = 1024;
    const int VEC_SIZE = SIZE / 8; /* 8 ints per __m256i */
    
    /* Allocate aligned memory for better performance */
    __m256i* vec_int1 = (__m256i*)_mm_malloc(VEC_SIZE * sizeof(__m256i), 32);
    __m256i* vec_int2 = (__m256i*)_mm_malloc(VEC_SIZE * sizeof(__m256i), 32);
    __m256i* vec_masks = (__m256i*)_mm_malloc(VEC_SIZE * sizeof(__m256i), 32);
    __m256d* vec_doubles = (__m256d*)_mm_malloc(VEC_SIZE * sizeof(__m256d), 32);
    __m256* vec_floats = (__m256*)_mm_malloc(VEC_SIZE * sizeof(__m256), 32);
    __m256i* output = (__m256i*)_mm_malloc(VEC_SIZE * sizeof(__m256i), 32);
    
    if (!vec_int1 || !vec_int2 || !vec_masks || !vec_doubles || 
        !vec_floats || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random data */
    init_arrays(vec_int1, vec_masks, vec_doubles, vec_floats, VEC_SIZE);
    init_arrays(vec_int2, vec_masks, vec_doubles, vec_floats, VEC_SIZE);
    
    printf("Testing many-argument optab expansion...\n");
    
    /* Call the function that should trigger the uncovered lines */
    test_many_args(output, vec_int1, vec_int2, vec_masks, 
                   vec_doubles, vec_floats, VEC_SIZE);
    
#ifdef __GNUC__
    /* Test with GCC vector extensions */
    test_vector_extensions((v8si*)output, (const v8si*)vec_int1,
                          (const v8si*)vec_int2, (const v8si*)vec_masks,
                          VEC_SIZE);
#endif
    
#ifdef __AVX512F__
    /* Test AVX-512 if available */
    __m512i* avx512_out = (__m512i*)_mm_malloc((SIZE/16) * sizeof(__m512i), 64);
    __m512i* avx512_in1 = (__m512i*)_mm_malloc((SIZE/16) * sizeof(__m512i), 64);
    __m512i* avx512_in2 = (__m512i*)_mm_malloc((SIZE/16) * sizeof(__m512i), 64);
    
    if (avx512_out && avx512_in1 && avx512_in2) {
        test_avx512(avx512_out, avx512_in1, avx512_in2, SIZE/16);
        _mm_free(avx512_out);
        _mm_free(avx512_in1);
        _mm_free(avx512_in2);
    }
#endif
    
    /* Compute checksum */
    uint64_t checksum = 0;
    int32_t* out_data = (int32_t*)output;
    for (int i = 0; i < SIZE; i++) {
        checksum += (uint64_t)out_data[i];
    }
    
    printf("Checksum: 0x%016llX\n", (unsigned long long)checksum);
    printf("Test completed.\n");
    
    /* Cleanup */
    _mm_free(vec_int1);
    _mm_free(vec_int2);
    _mm_free(vec_masks);
    _mm_free(vec_doubles);
    _mm_free(vec_floats);
    _mm_free(output);
    
    return 0;
}
