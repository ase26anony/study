#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
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
        arr1[i] = _mm256_set_epi32(prng_next(), prng_next(), prng_next(), 
                                  prng_next(), prng_next(), prng_next(), 
                                  prng_next(), prng_next());
        arr2[i] = _mm256_set_epi32(prng_next(), prng_next(), prng_next(), 
                                  prng_next(), prng_next(), prng_next(), 
                                  prng_next(), prng_next());
        arr3[i] = _mm256_set_epi32(prng_next(), prng_next(), prng_next(), 
                                  prng_next(), prng_next(), prng_next(), 
                                  prng_next(), prng_next());
        arr4[i] = _mm256_set_epi32(prng_next(), prng_next(), prng_next(), 
                                  prng_next(), prng_next(), prng_next(), 
                                  prng_next(), prng_next());
        arr5[i] = _mm256_set_epi32(prng_next(), prng_next(), prng_next(), 
                                  prng_next(), prng_next(), prng_next(), 
                                  prng_next(), prng_next());
    }
}

/* Complex expression with many temporaries - forces expander to generate many operands */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args(__m256i* out, const __m256i* in1, const __m256i* in2,
                          const __m256i* in3, const __m256i* in4, const __m256i* in5,
                          size_t size) {
    /* Volatile counter to prevent loop unrolling */
    volatile size_t vcounter = 0;
    
    for (size_t i = 0; i < size; i++) {
        /* Load vectors with volatile memory access to inhibit CSE */
        __m256i v1, v2, v3, v4, v5;
        asm volatile("" : "+m"(in1[i]));
        asm volatile("" : "+m"(in2[i]));
        asm volatile("" : "+m"(in3[i]));
        asm volatile("" : "+m"(in4[i]));
        asm volatile("" : "+m"(in5[i]));
        
        v1 = in1[i];
        v2 = in2[i];
        v3 = in3[i];
        v4 = in4[i];
        v5 = in5[i];
        
        /* Complex multi-statement expression with many intermediate values */
        __m256i t1 = _mm256_add_epi32(v1, v2);
        __m256i t2 = _mm256_sub_epi32(v3, v4);
        __m256i t3 = _mm256_mullo_epi32(t1, t2);
        __m256i t4 = _mm256_slli_epi32(v5, 3);
        __m256i t5 = _mm256_srli_epi32(v1, 2);
        __m256i t6 = _mm256_and_si256(t3, t4);
        __m256i t7 = _mm256_or_si256(t5, t6);
        __m256i t8 = _mm256_xor_si256(t7, v2);
        __m256i t9 = _mm256_add_epi32(t8, v3);
        __m256i t10 = _mm256_sub_epi32(t9, v4);
        
        /* Inline assembly with 10-11 operands to trigger optab expansion */
        __m256i result;
        asm volatile (
            /* 10 input operands + 1 output operand = 11 total */
            "vpaddd %0, %1, %2\n\t"
            "vpsubd %0, %0, %3\n\t"
            "vpmulld %0, %0, %4\n\t"
            "vpslld $3, %5, %5\n\t"
            "vpsrld $2, %6, %6\n\t"
            "vpand %0, %0, %5\n\t"
            "vpor %0, %0, %6\n\t"
            "vpxor %0, %0, %7\n\t"
            "vpaddd %0, %0, %8\n\t"
            "vpsubd %0, %0, %9"
            : "=&x"(result)  /* output operand */
            : "x"(t1), "x"(t2), "x"(t3), "x"(t4), 
              "x"(t5), "x"(t6), "x"(t7), "x"(t8), "x"(t9)  /* 9 input operands */
            : "memory"
        );
        
        /* Another asm with exactly 10 arguments using memory constraints */
        __m256i final_result;
        asm volatile (
            /* 10 memory operands */
            "vmovdqa %1, %0\n\t"
            "vpaddd %0, %0, %2\n\t"
            "vpsubd %0, %0, %3\n\t"
            "vpmulld %0, %0, %4\n\t"
            "vpslld $3, %0, %0\n\t"
            "vpsrld $2, %0, %0\n\t"
            "vpand %0, %0, %5\n\t"
            "vpor %0, %0, %6\n\t"
            "vpxor %0, %0, %7\n\t"
            "vpaddd %0, %0, %8\n\t"
            "vpsubd %0, %0, %9"
            : "=&x"(final_result)
            : "m"(result), "m"(v1), "m"(v2), "m"(v3), 
              "m"(v4), "m"(v5), "m"(t1), "m"(t2), "m"(t3)
            : "memory"
        );
        
        out[i] = final_result;
        
        /* Update volatile counter */
        vcounter++;
        asm volatile("" : "+r"(vcounter));
    }
}

/* Alternative approach using vector builtins with many arguments */
#ifdef __AVX512F__
__attribute__((noinline, target("avx512f")))
static void test_vector_builtins(__m512i* out, const __m512i* in1, 
                                const __m512i* in2, const __m512i* in3,
                                size_t size) {
    for (size_t i = 0; i < size; i++) {
        /* Create complex shuffle with many immediate arguments */
        __m512i v1 = in1[i];
        __m512i v2 = in2[i];
        __m512i v3 = in3[i];
        
        /* Complex expression that might expand to many operands */
        __m512i result = _mm512_add_epi32(v1, v2);
        result = _mm512_sub_epi32(result, v3);
        
        /* Use blend with mask - potentially many arguments when expanded */
        __mmask16 mask = 0xAAAA;  /* alternating pattern */
        result = _mm512_mask_blend_epi32(mask, result, v1);
        
        /* Permutation with many lane indices */
        __m512i permuted = _mm512_permutexvar_epi32(
            _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0), result);
        
        out[i] = permuted;
    }
}
#endif

/* Test with mixed scalar types to force complex expansion */
__attribute__((noinline))
static long test_mixed_types(char* cptr, short* sptr, int* iptr, 
                            long* lptr, float* fptr, double* dptr,
                            size_t size) {
    long sum = 0;
    volatile size_t vcounter = 0;
    
    for (size_t i = 0; i < size; i++) {
        /* Complex expression with many intermediate values */
        char c = cptr[i];
        short s = sptr[i];
        int i1 = iptr[i];
        long l = lptr[i];
        float f = fptr[i];
        double d = dptr[i];
        
        /* Multi-statement expression forcing many temporaries */
        int t1 = (int)c + (int)s;
        long t2 = (long)i1 * l;
        float t3 = f * 2.0f;
        double t4 = d / 3.0;
        int t5 = t1 ^ (int)t2;
        long t6 = t2 + (long)t5;
        float t7 = t3 + (float)t4;
        double t8 = t4 - (double)t7;
        int t9 = t5 | (int)t6;
        long t10 = t6 & (long)t9;
        
        /* Final complex expression that might expand to many operands */
        long result = t10 + (long)t9 + (long)t8 + (long)t7 + t6 + t5 + 
                     (long)t4 + (long)t3 + t2 + t1;
        
        sum += result;
        
        vcounter++;
        asm volatile("" : "+r"(vcounter));
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
    
    /* Initialize arrays */
    init_arrays(vec1, vec2, vec3, vec4, vec5, VEC_SIZE);
    
    /* Test the many-argument function */
    test_many_args(out_vec, vec1, vec2, vec3, vec4, vec5, VEC_SIZE);
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (size_t i = 0; i < VEC_SIZE; i++) {
        int32_t* data = (int32_t*)&out_vec[i];
        for (int j = 0; j < 8; j++) {
            checksum += (uint64_t)data[j];
        }
    }
    
    printf("Vector checksum: %lu\n", checksum);
    
    /* Test mixed types */
    char* cdata = malloc(ARRAY_SIZE * sizeof(char));
    short* sdata = malloc(ARRAY_SIZE * sizeof(short));
    int* idata = malloc(ARRAY_SIZE * sizeof(int));
    long* ldata = malloc(ARRAY_SIZE * sizeof(long));
    float* fdata = malloc(ARRAY_SIZE * sizeof(float));
    double* ddata = malloc(ARRAY_SIZE * sizeof(double));
    
    /* Initialize mixed arrays */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        cdata[i] = (char)(prng_next() & 0xFF);
        sdata[i] = (short)(prng_next() & 0xFFFF);
        idata[i] = (int)prng_next();
        ldata[i] = (long)prng_next();
        fdata[i] = (float)prng_next() / 1000.0f;
        ddata[i] = (double)prng_next() / 1000.0;
    }
    
    long mixed_sum = test_mixed_types(cdata, sdata, idata, ldata, fdata, ddata, ARRAY_SIZE);
    printf("Mixed types sum: %ld\n", mixed_sum);
    
    /* Cleanup */
    free(vec1); free(vec2); free(vec3); free(vec4); free(vec5); free(out_vec);
    free(cdata); free(sdata); free(idata); free(ldata); free(fdata); free(ddata);
    
    return 0;
}
