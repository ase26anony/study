#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));
typedef uint8_t uint8x64_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(int32_t *arr1, int32_t *arr2, int32_t *arr3, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr1[i] = (int32_t)lcg_rand();
        arr2[i] = (int32_t)lcg_rand();
        arr3[i] = (int32_t)lcg_rand();
    }
}

/* Complex mask computation that prevents constant propagation */
static int32x16_t compute_dynamic_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control + i) % 32;
        if (i % 3 == 0) mask_data[i] += control;
        if (i % 5 == 0) mask_data[i] -= control;
    }
    
    return mask;
}

/* Kernel 1: Complex shuffle with computed mask */
static void kernel1(int32x16_t *result, const int32x16_t *a, 
                    const int32x16_t *b, volatile int control) {
    int32x16_t mask = compute_dynamic_mask(control);
    
    /* This shuffle with large vectors and dynamic mask may require 
       many operands during RTL expansion */
    *result = __builtin_shuffle(*a, *b, mask);
}

/* Kernel 2: Chain of shuffles to accumulate operand count */
static void kernel2(int32x16_t *result, const int32x16_t *a, 
                    const int32x16_t *b, const int32x16_t *c,
                    volatile int control) {
    int32x16_t mask1 = compute_dynamic_mask(control);
    int32x16_t mask2 = compute_dynamic_mask(control + 1);
    
    /* Chain shuffles - output of first becomes input to second */
    int32x16_t temp = __builtin_shuffle(*a, *b, mask1);
    *result = __builtin_shuffle(temp, *c, mask2);
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
    
    *result = (control & 1) ? shuffle1 : shuffle2;
}

/* Kernel 4: Mixed-type permutations */
static void kernel4(float32x16_t *result, const float32x16_t *fa,
                    const int32x16_t *ia, volatile int control) {
    /* Convert int vector to byte vector for complex permutation */
    uint8x64_t byte_vec = *(const uint8x64_t*)ia;
    
    /* Create a complex mask for byte-level permutation */
    uint8x64_t byte_mask = {0};
    uint8_t *mask_data = (uint8_t*)&byte_mask;
    for (int i = 0; i < 64; i++) {
        mask_data[i] = (control + i * 3) % 128;
    }
    
    /* Permute bytes */
    uint8x64_t permuted_bytes = __builtin_shuffle(byte_vec, byte_mask);
    
    /* Convert back and mix with float vector */
    int32x16_t permuted_ints = *(int32x16_t*)&permuted_bytes;
    float32x16_t float_from_int = __builtin_convertvector(permuted_ints, float32x16_t);
    
    /* Final shuffle mixing float and int-converted vectors */
    int32x16_t mix_mask = compute_dynamic_mask(control);
    *result = __builtin_shuffle(*fa, float_from_int, mix_mask);
}

/* Inline assembly with many vector operands */
static void kernel5_asm(int32x16_t *r1, int32x16_t *r2,
                        const int32x16_t *a, const int32x16_t *b,
                        const int32x16_t *c, const int32x16_t *d) {
#ifdef __x86_64__
    /* x86-specific inline assembly with many vector operands */
    asm volatile (
        "vmovdqa %[a], %%ymm0\n\t"
        "vmovdqa %[b], %%ymm1\n\t"
        "vmovdqa %[c], %%ymm2\n\t"
        "vmovdqa %[d], %%ymm3\n\t"
        "vperm2i128 $0x21, %%ymm0, %%ymm1, %%ymm4\n\t"
        "vperm2i128 $0x21, %%ymm2, %%ymm3, %%ymm5\n\t"
        "vpblendd $0xF0, %%ymm4, %%ymm5, %%ymm6\n\t"
        "vmovdqa %%ymm6, %[r1]\n\t"
        "vpshufd $0x1B, %%ymm6, %%ymm7\n\t"
        "vmovdqa %%ymm7, %[r2]\n\t"
        : [r1] "=m" (*r1), [r2] "=m" (*r2)
        : [a] "m" (*a), [b] "m" (*b), [c] "m" (*c), [d] "m" (*d)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "memory"
    );
#elif defined(__aarch64__)
    /* ARM-specific inline assembly */
    asm volatile (
        "ld1 {v0.4s, v1.4s, v2.4s, v3.4s}, [%[a]]\n\t"
        "ld1 {v4.4s, v5.4s, v6.4s, v7.4s}, [%[b]]\n\t"
        "trn1 v8.4s, v0.4s, v4.4s\n\t"
        "trn2 v9.4s, v0.4s, v4.4s\n\t"
        "zip1 v10.4s, v8.4s, v9.4s\n\t"
        "zip2 v11.4s, v8.4s, v9.4s\n\t"
        "st1 {v10.4s, v11.4s}, [%[r1]]\n\t"
        "rev64 v12.4s, v10.4s\n\t"
        "rev64 v13.4s, v11.4s\n\t"
        "st1 {v12.4s, v13.4s}, [%[r2]]\n\t"
        : 
        : [a] "r" (a), [b] "r" (b), [r1] "r" (r1), [r2] "r" (r2)
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
          "v8", "v9", "v10", "v11", "v12", "v13", "memory"
    );
#else
    /* Generic fallback */
    *r1 = __builtin_shuffle(*a, *b, compute_dynamic_mask(42));
    *r2 = __builtin_shuffle(*b, *a, compute_dynamic_mask(43));
#endif
}

/* Compute checksum to prevent dead code elimination */
static int64_t compute_checksum(const int32x16_t *vec, size_t count) {
    int64_t sum = 0;
    const int32_t *data = (const int32_t*)vec;
    
    for (size_t i = 0; i < count * 16; i++) {
        sum += data[i];
    }
    
    return sum;
}

int main(void) {
    /* Allocate aligned memory for vectors */
    const size_t ARRAY_SIZE = 1024;
    const size_t VEC_COUNT = ARRAY_SIZE / 16;
    
    int32_t *array1 = aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *array2 = aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *array3 = aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    float *array_f = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    /* Initialize with pseudo-random data */
    init_arrays(array1, array2, array3, ARRAY_SIZE);
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        array_f[i] = (float)lcg_rand() / 1000.0f;
    }
    
    /* Cast to vector types */
    int32x16_t *vec_a = (int32x16_t*)array1;
    int32x16_t *vec_b = (int32x16_t*)array2;
    int32x16_t *vec_c = (int32x16_t*)array3;
    float32x16_t *vec_f = (float32x16_t*)array_f;
    
    /* Result storage */
    int32x16_t *results1 = aligned_alloc(64, VEC_COUNT * sizeof(int32x16_t));
    int32x16_t *results2 = aligned_alloc(64, VEC_COUNT * sizeof(int32x16_t));
    int32x16_t *results3 = aligned_alloc(64, VEC_COUNT * sizeof(int32x16_t));
    float32x16_t *results4 = aligned_alloc(64, VEC_COUNT * sizeof(float32x16_t));
    int32x16_t *results5a = aligned_alloc(64, VEC_COUNT * sizeof(int32x16_t));
    int32x16_t *results5b = aligned_alloc(64, VEC_COUNT * sizeof(int32x16_t));
    
    volatile int control = 0;
    
    /* Main loop with data-dependent operations */
    for (size_t iter = 0; iter < 100; iter++) {
        control = lcg_rand() & 0xFF;
        
        for (size_t i = 0; i < VEC_COUNT; i++) {
            /* Compiler barrier */
            asm volatile("" ::: "memory");
            
            /* Kernel 1: Basic complex shuffle */
            kernel1(&results1[i], &vec_a[i], &vec_b[i], control + i);
            
            /* Compiler barrier */
            asm volatile("" ::: "memory");
            
            /* Kernel 2: Chained shuffles */
            kernel2(&results2[i], &vec_a[i], &vec_b[i], &vec_c[i], control + i * 2);
            
            /* Compiler barrier */
            asm volatile("" ::: "memory");
            
            /* Kernel 3: Conditional permutation */
            kernel3(&results3[i], &vec_a[i], &vec_b[i], &vec_c[i], control + i * 3);
            
            /* Compiler barrier */
            asm volatile("" ::: "memory");
            
            /* Kernel 4: Mixed-type operations */
            float32x16_t temp_f = *(float32x16_t*)&vec_a[i];
            kernel4(&results4[i], &temp_f, &vec_b[i], control + i * 4);
            
            /* Compiler barrier */
            asm volatile("" ::: "memory");
            
            /* Kernel 5: Inline assembly with many operands */
            kernel5_asm(&results5a[i], &results5b[i], 
                       &vec_a[i], &vec_b[i], &vec_c[i], &results1[i]);
        }
        
        /* Modify control variable to prevent loop optimization */
        control += VEC_COUNT;
    }
    
    /* Compute checksums to prevent dead code elimination */
    int64_t checksum1 = compute_checksum(results1, VEC_COUNT);
    int64_t checksum2 = compute_checksum(results2, VEC_COUNT);
    int64_t checksum3 = compute_checksum(results3, VEC_COUNT);
    int64_t checksum5a = compute_checksum(results5a, VEC_COUNT);
    int64_t checksum5b = compute_checksum(results5b, VEC_COUNT);
    
    /* Use checksums to affect output */
    printf("Checksums: %ld %ld %ld %ld %ld\n", 
           checksum1, checksum2, checksum3, checksum5a, checksum5b);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array_f);
    free(results1);
    free(results2);
    free(results3);
    free(results4);
    free(results5a);
    free(results5b);
    
    return 0;
}
