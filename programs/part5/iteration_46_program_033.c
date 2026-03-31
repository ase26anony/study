#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Simple PRNG for reproducible results */
static uint32_t seed = 123456789;
static inline uint32_t fast_rand(void) {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(__m256i* arr1, __m256i* arr2, __m256i* arr3, 
                       __m256i* arr4, __m256i* arr5, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr1[i] = _mm256_set_epi32(
            fast_rand(), fast_rand(), fast_rand(), fast_rand(),
            fast_rand(), fast_rand(), fast_rand(), fast_rand()
        );
        arr2[i] = _mm256_set_epi32(
            fast_rand(), fast_rand(), fast_rand(), fast_rand(),
            fast_rand(), fast_rand(), fast_rand(), fast_rand()
        );
        arr3[i] = _mm256_set_epi32(
            fast_rand(), fast_rand(), fast_rand(), fast_rand(),
            fast_rand(), fast_rand(), fast_rand(), fast_rand()
        );
        arr4[i] = _mm256_set_epi32(
            fast_rand(), fast_rand(), fast_rand(), fast_rand(),
            fast_rand(), fast_rand(), fast_rand(), fast_rand()
        );
        arr5[i] = _mm256_set_epi32(
            fast_rand(), fast_rand(), fast_rand(), fast_rand(),
            fast_rand(), fast_rand(), fast_rand(), fast_rand()
        );
    }
}

/* Complex expression with many temporaries to force optab expansion */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args(__m256i* restrict out, 
                          const __m256i* restrict in1,
                          const __m256i* restrict in2,
                          const __m256i* restrict in3,
                          const __m256i* restrict in4,
                          const __m256i* restrict in5,
                          size_t size) {
    
    /* Volatile counter to prevent loop unrolling */
    volatile size_t vcounter = 0;
    
    for (size_t i = 0; i < size; i++) {
        vcounter = i;
        
        /* Load multiple vectors */
        __m256i v1 = in1[i];
        __m256i v2 = in2[i];
        __m256i v3 = in3[i];
        __m256i v4 = in4[i];
        __m256i v5 = in5[i];
        
        /* Create complex dependencies to inhibit CSE */
        __m256i temp1, temp2, temp3, temp4, temp5;
        
        /* Use inline asm to create fake dependencies */
        asm volatile("" : "+x"(v1), "+x"(v2), "+x"(v3));
        
        /* Complex multi-step operation that could be folded into optab */
        /* This creates many intermediate values */
        temp1 = _mm256_add_epi32(v1, v2);
        temp2 = _mm256_sub_epi32(v3, v4);
        temp3 = _mm256_mullo_epi32(temp1, temp2);
        temp4 = _mm256_slli_epi32(v5, 3);
        temp5 = _mm256_xor_si256(temp3, temp4);
        
        /* Create a shuffle with many arguments using inline asm */
        /* This should trigger the 10-11 argument optab expansion */
        __m256i result;
        
        /* Extended inline asm with 10 operands */
        asm volatile (
            "vpaddd %0, %1, %2\n\t"
            "vpsubd %0, %0, %3\n\t"
            "vpmulld %0, %0, %4\n\t"
            "vpslld $3, %5, %6\n\t"
            "vpxor %0, %0, %6\n\t"
            "vpshufd $0xE4, %0, %0\n\t"
            "vpermq $0x39, %0, %0\n\t"
            : "=&x"(result)
            : "x"(v1), "x"(v2), "x"(v3), "x"(v4), 
              "x"(v5), "x"(temp1), "x"(temp2), 
              "x"(temp3), "x"(temp4)
            : "memory"
        );
        
        /* Another complex expression with 11 arguments using builtins */
        /* Mix of vector and scalar operations */
        int imm1 = (i & 0xF) + 1;
        int imm2 = (i & 0x7) + 2;
        int imm3 = (i & 0x3) + 4;
        int imm4 = (i & 0x1) + 8;
        
        /* Force these to be used in complex ways */
        __m256i blend_result;
        
        /* Complex blending operation that might use many arguments */
        blend_result = _mm256_blend_epi32(v1, v2, imm1);
        blend_result = _mm256_blend_epi32(blend_result, v3, imm2);
        blend_result = _mm256_blend_epi32(blend_result, v4, imm3);
        blend_result = _mm256_blend_epi32(blend_result, v5, imm4);
        
        /* Combine results */
        out[i] = _mm256_add_epi32(result, blend_result);
        
        /* Prevent dead code elimination */
        asm volatile("" :: "x"(out[i]));
    }
}

/* Alternative function using GCC vector builtins directly */
__attribute__((noinline, target("avx2")))
static void test_vector_builtins(__m256i* restrict out,
                                const __m256i* restrict in,
                                size_t size) {
    
    typedef int32_t v8si __attribute__((vector_size(32)));
    
    for (size_t i = 0; i < size; i++) {
        /* Load vectors */
        v8si v1 = *(const v8si*)&in[i * 5];
        v8si v2 = *(const v8si*)&in[i * 5 + 1];
        v8si v3 = *(const v8si*)&in[i * 5 + 2];
        v8si v4 = *(const v8si*)&in[i * 5 + 3];
        v8si v5 = *(const v8si*)&in[i * 5 + 4];
        
        /* Complex expression with many operations */
        v8si t1 = v1 + v2;
        v8si t2 = v3 - v4;
        v8si t3 = t1 * t2;
        v8si t4 = v5 << 3;
        v8si t5 = t3 ^ t4;
        
        /* Use __builtin_shuffle with complex pattern */
        /* This can generate many arguments */
        int idx[8] = {7, 6, 5, 4, 3, 2, 1, 0};
        
        /* Create shuffle with many arguments */
        v8si shuffled = __builtin_shuffle(t1, t2, 
            idx[0], idx[1], idx[2], idx[3],
            idx[4] + 8, idx[5] + 8, idx[6] + 8, idx[7] + 8);
        
        /* More complex operations */
        v8si result = shuffled + t3;
        result = result * t4;
        result = result | t5;
        
        /* Store result */
        *(v8si*)&out[i] = result;
    }
}

/* Function with mixed scalar/vector operations */
__attribute__((noinline))
static void test_mixed_operations(int32_t* restrict out,
                                 const int32_t* restrict in,
                                 size_t size) {
    
    /* Force many temporaries */
    for (size_t i = 0; i < size; i += 8) {
        /* Load 8 elements */
        int32_t a0 = in[i];
        int32_t a1 = in[i + 1];
        int32_t a2 = in[i + 2];
        int32_t a3 = in[i + 3];
        int32_t a4 = in[i + 4];
        int32_t a5 = in[i + 5];
        int32_t a6 = in[i + 6];
        int32_t a7 = in[i + 7];
        
        /* Complex expression tree with many operations */
        int32_t t0 = a0 * a1 + a2;
        int32_t t1 = a3 - a4 * a5;
        int32_t t2 = a6 << (a7 & 0x3);
        int32_t t3 = (a0 ^ a1) | (a2 & a3);
        int32_t t4 = a4 + (a5 << 2) - a6;
        int32_t t5 = a7 * 3 + a0;
        int32_t t6 = (a1 >> 1) + (a2 << 1);
        int32_t t7 = a3 ^ a4 ^ a5;
        
        /* Even more complex combining */
        int32_t r0 = t0 + t1 - t2;
        int32_t r1 = t3 * t4 / (t5 + 1);
        int32_t r2 = t6 & t7 | t0;
        int32_t r3 = (t1 << 3) ^ t2;
        int32_t r4 = t3 + t4 - t5;
        int32_t r5 = t6 * t7 + t0;
        int32_t r6 = (t1 >> 2) | t2;
        int32_t r7 = t3 ^ t4 ^ t5 ^ t6;
        
        /* Store results */
        out[i] = r0 + r1;
        out[i + 1] = r2 - r3;
        out[i + 2] = r4 * r5;
        out[i + 3] = r6 & r7;
        out[i + 4] = r0 | r1;
        out[i + 5] = r2 ^ r3;
        out[i + 6] = r4 + r5;
        out[i + 7] = r6 - r7;
    }
}

/* Compute checksum for validation */
static uint64_t compute_checksum(const __m256i* data, size_t size) {
    uint64_t checksum = 0;
    const uint32_t* ptr = (const uint32_t*)data;
    
    for (size_t i = 0; i < size * 8; i++) {
        checksum += ptr[i];
        checksum = (checksum << 13) | (checksum >> 51);
    }
    
    return checksum;
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    const size_t VEC_SIZE = ARRAY_SIZE / 8;
    
    /* Allocate aligned memory for vectors */
    __m256i* vec1 = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* vec2 = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* vec3 = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* vec4 = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* vec5 = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* out_vec = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    
    /* Allocate scalar arrays */
    int32_t* scalar_in = aligned_alloc(32, ARRAY_SIZE * sizeof(int32_t));
    int32_t* scalar_out = aligned_alloc(32, ARRAY_SIZE * sizeof(int32_t));
    
    if (!vec1 || !vec2 || !vec3 || !vec4 || !vec5 || !out_vec ||
        !scalar_in || !scalar_out) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    init_arrays(vec1, vec2, vec3, vec4, vec5, VEC_SIZE);
    
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        scalar_in[i] = fast_rand();
    }
    
    printf("Testing many-argument operations...\n");
    
    /* Test vector operations with many arguments */
    test_many_args(out_vec, vec1, vec2, vec3, vec4, vec5, VEC_SIZE);
    
    /* Test GCC vector builtins */
    test_vector_builtins(out_vec, vec1, VEC_SIZE / 5);
    
    /* Test mixed scalar/vector operations */
    test_mixed_operations(scalar_out, scalar_in, ARRAY_SIZE);
    
    /* Compute and print checksum */
    uint64_t checksum = compute_checksum(out_vec, VEC_SIZE);
    printf("Checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    /* Cleanup */
    free(vec1);
    free(vec2);
    free(vec3);
    free(vec4);
    free(vec5);
    free(out_vec);
    free(scalar_in);
    free(scalar_out);
    
    return 0;
}
