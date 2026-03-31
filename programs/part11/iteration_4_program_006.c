#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define large vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(int32_t *arr1, int32_t *arr2, int32_t *arr3, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr1[i] = (int32_t)(lcg_rand() % 1000);
        arr2[i] = (int32_t)(lcg_rand() % 1000);
        arr3[i] = (int32_t)(lcg_rand() % 1000);
    }
}

/* Complex shuffle mask computation - prevents constant propagation */
static int32x16_t compute_dynamic_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_ptr = (int32_t*)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_ptr[i] = (control + i * 3) % 16;
        if (i % 4 == 0) mask_ptr[i] = (mask_ptr[i] + control) % 8 + 8;
    }
    
    return mask;
}

/* Kernel 1: Complex shuffle with computed mask */
static void kernel1(int32x16_t *result, const int32x16_t *a, 
                    const int32x16_t *b, volatile int control) {
    int32x16_t mask = compute_dynamic_mask(control);
    
    /* This shuffle with dynamic mask may require many operands during expansion */
    *result = __builtin_shuffle(*a, *b, mask);
}

/* Kernel 2: Chain of shuffles - output of one becomes input to next */
static void kernel2(int32x16_t *result, const int32x16_t *a, 
                    const int32x16_t *b, const int32x16_t *c,
                    volatile int control) {
    int32x16_t mask1 = compute_dynamic_mask(control);
    int32x16_t mask2 = compute_dynamic_mask(control + 1);
    int32x16_t mask3 = compute_dynamic_mask(control + 2);
    
    /* Chain multiple shuffles - increases operand count */
    int32x16_t temp1 = __builtin_shuffle(*a, *b, mask1);
    int32x16_t temp2 = __builtin_shuffle(temp1, *c, mask2);
    *result = __builtin_shuffle(temp2, *a, mask3);
}

/* Kernel 3: Conditional vector permutation */
static void kernel3(int32x16_t *result, const int32x16_t *a,
                    const int32x16_t *b, const int32x16_t *c,
                    volatile int control) {
    int32x16_t mask1 = compute_dynamic_mask(control);
    int32x16_t mask2 = compute_dynamic_mask(control + 100);
    
    /* Conditional shuffle selection */
    int32x16_t shuffle1 = __builtin_shuffle(*a, *b, mask1);
    int32x16_t shuffle2 = __builtin_shuffle(*b, *c, mask2);
    
    /* Use conditional operator on vectors - complex operation */
    int32x16_t condition = *a > *b;
    *result = condition ? shuffle1 : shuffle2;
}

/* Mixed type operations */
static void kernel_mixed(float32x16_t *fresult, const int32x16_t *a,
                         const float32x16_t *b, volatile int control) {
    /* Convert int to float */
    float32x16_t fa = __builtin_convertvector(*a, float32x16_t);
    
    /* Create a shuffle mask */
    int32x16_t imask = compute_dynamic_mask(control);
    float32x16_t fmask = __builtin_convertvector(imask, float32x16_t);
    
    /* Shuffle with mixed-type considerations */
    *fresult = __builtin_shuffle(fa, *b, imask) + fmask;
}

/* Inline assembly with many vector operands */
static void kernel_asm(int32x16_t *out1, int32x16_t *out2,
                       const int32x16_t *in1, const int32x16_t *in2,
                       const int32x16_t *in3, const int32x16_t *in4) {
#if defined(__x86_64__) || defined(__i386__)
    /* x86-specific inline assembly with many vector operands */
    asm volatile (
        "vmovdqa %[in1], %%ymm0\n\t"
        "vmovdqa %[in2], %%ymm1\n\t"
        "vmovdqa %[in3], %%ymm2\n\t"
        "vmovdqa %[in4], %%ymm3\n\t"
        "vpermq $0x1B, %%ymm0, %%ymm4\n\t"
        "vpermq $0x4E, %%ymm1, %%ymm5\n\t"
        "vpblendd $0xF0, %%ymm4, %%ymm5, %%ymm6\n\t"
        "vmovdqa %%ymm6, %[out1]\n\t"
        "vpshufd $0x1B, %%ymm2, %%ymm7\n\t"
        "vpshufd $0x4E, %%ymm3, %%ymm8\n\t"
        "vpaddd %%ymm7, %%ymm8, %%ymm9\n\t"
        "vmovdqa %%ymm9, %[out2]\n\t"
        : [out1] "=m" (*out1),
          [out2] "=m" (*out2)
        : [in1] "m" (*in1),
          [in2] "m" (*in2),
          [in3] "m" (*in3),
          [in4] "m" (*in4)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
          "ymm6", "ymm7", "ymm8", "ymm9", "memory"
    );
#elif defined(__aarch64__) || defined(__arm__)
    /* ARM-specific inline assembly */
    asm volatile (
        "ld1 {v0.4s, v1.4s, v2.4s, v3.4s}, [%[in1]]\n\t"
        "ld1 {v4.4s, v5.4s, v6.4s, v7.4s}, [%[in2]]\n\t"
        "trn1 v8.4s, v0.4s, v1.4s\n\t"
        "trn2 v9.4s, v2.4s, v3.4s\n\t"
        "zip1 v10.4s, v4.4s, v5.4s\n\t"
        "zip2 v11.4s, v6.4s, v7.4s\n\t"
        "st1 {v8.4s, v9.4s, v10.4s, v11.4s}, [%[out1]]\n\t"
        : 
        : [in1] "r" (in1),
          [in2] "r" (in2),
          [out1] "r" (out1)
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
          "v8", "v9", "v10", "v11", "memory"
    );
#else
    /* Generic fallback */
    *out1 = *in1 + *in2;
    *out2 = *in3 + *in4;
#endif
}

/* Target-specific builtins */
static void kernel_target_builtin(int32x16_t *result, const int32x16_t *a,
                                  const int32x16_t *b, volatile int control) {
#if defined(__x86_64__) && defined(__AVX2__)
    /* Use x86-specific builtins that may map to multi-operand instructions */
    __m256i avx_a = *(__m256i*)a;
    __m256i avx_b = *(__m256i*)b;
    
    /* Complex sequence of builtins */
    __m256i temp1 = _mm256_shuffle_epi32(avx_a, _MM_SHUFFLE(1, 0, 3, 2));
    __m256i temp2 = _mm256_shuffle_epi32(avx_b, _MM_SHUFFLE(2, 3, 0, 1));
    __m256i temp3 = _mm256_blend_epi32(temp1, temp2, 0xCC);
    
    /* Additional permutation */
    __m256i temp4 = _mm256_permute4x64_epi64(temp3, 0x4E);
    __m256i temp5 = _mm256_slli_epi32(temp4, (control % 4));
    
    *(int32x16_t*)result = *(int32x16_t*)&temp5;
#elif defined(__aarch64__)
    /* ARM NEON builtins */
    int32x4_t neon_a1 = *(int32x4_t*)a;
    int32x4_t neon_a2 = *(int32x4_t*)(a + 4);
    int32x4_t neon_a3 = *(int32x4_t*)(a + 8);
    int32x4_t neon_a4 = *(int32x4_t*)(a + 12);
    
    /* Complex NEON operations */
    int32x4_t rev1 = __builtin_neon_vrev64q_s32(neon_a1);
    int32x4_t rev2 = __builtin_neon_vrev64q_s32(neon_a2);
    int32x4_t zip1 = __builtin_neon_vzip1q_s32(rev1, rev2);
    int32x4_t zip2 = __builtin_neon_vzip2q_s32(rev1, rev2);
    
    /* Store results */
    *(int32x4_t*)result = zip1;
    *(int32x4_t*)(result + 4) = zip2;
#else
    /* Generic fallback */
    *result = *a + *b;
#endif
}

/* Horizontal sum for checksum */
static int64_t horizontal_sum_i32x16(const int32x16_t *vec) {
    const int32_t *ptr = (const int32_t*)vec;
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += ptr[i];
    }
    return sum;
}

static float horizontal_sum_f32x16(const float32x16_t *vec) {
    const float *ptr = (const float*)vec;
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += ptr[i];
    }
    return sum;
}

int main(void) {
    /* Allocate aligned memory for vector operations */
    const size_t array_size = 1024;
    int32_t *array1 = aligned_alloc(64, array_size * sizeof(int32_t));
    int32_t *array2 = aligned_alloc(64, array_size * sizeof(int32_t));
    int32_t *array3 = aligned_alloc(64, array_size * sizeof(int32_t));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_arrays(array1, array2, array3, array_size);
    
    /* Cast to vector types */
    int32x16_t *vec1 = (int32x16_t*)array1;
    int32x16_t *vec2 = (int32x16_t*)array2;
    int32x16_t *vec3 = (int32x16_t*)array3;
    
    /* Result vectors */
    int32x16_t result1, result2, result3, result4, result5;
    float32x16_t fresult;
    
    /* Volatile control variable to prevent optimization */
    volatile int control = 42;
    
    /* Loop with data-dependent operations */
    for (int iteration = 0; iteration < 100; iteration++) {
        /* Update control to make operations data-dependent */
        control = (control * 13 + 17) % 256;
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Kernel 1: Complex shuffle with dynamic mask */
        kernel1(&result1, &vec1[iteration % 16], &vec2[iteration % 16], control);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Kernel 2: Chain of shuffles */
        kernel2(&result2, &vec1[(iteration + 1) % 16], 
                &vec2[(iteration + 2) % 16], &vec3[(iteration + 3) % 16], 
                control + iteration);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Kernel 3: Conditional permutation */
        kernel3(&result3, &vec1[(iteration + 4) % 16],
                &vec2[(iteration + 5) % 16], &vec3[(iteration + 6) % 16],
                control + iteration * 3);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Mixed type operations */
        float32x16_t fvec = *(float32x16_t*)&vec1[(iteration + 7) % 16];
        kernel_mixed(&fresult, &vec2[(iteration + 8) % 16], &fvec, control);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Inline assembly kernel */
        kernel_asm(&result4, &result5,
                   &vec1[(iteration + 9) % 16], &vec2[(iteration + 10) % 16],
                   &vec3[(iteration + 11) % 16], &vec1[(iteration + 12) % 16]);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Target-specific builtins */
        kernel_target_builtin(&result4, &vec1[(iteration + 13) % 16],
                              &vec2[(iteration + 14) % 16], control);
    }
    
    /* Compute checksums to prevent dead code elimination */
    int64_t checksum1 = horizontal_sum_i32x16(&result1);
    int64_t checksum2 = horizontal_sum_i32x16(&result2);
    int64_t checksum3 = horizontal_sum_i32x16(&result3);
    int64_t checksum4 = horizontal_sum_i32x16(&result4);
    int64_t checksum5 = horizontal_sum_i32x16(&result5);
    float fchecksum = horizontal_sum_f32x16(&fresult);
    
    /* Print checksums (prevents optimization) */
    printf("Checksums: %ld %ld %ld %ld %ld %f\n", 
           checksum1, checksum2, checksum3, checksum4, checksum5, fchecksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
