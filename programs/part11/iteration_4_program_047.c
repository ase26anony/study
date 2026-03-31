#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef int32_t int32x4_t __attribute__((vector_size(16)));
typedef float float32x8_t __attribute__((vector_size(32)));
typedef double float64x4_t __attribute__((vector_size(32)));
typedef int64_t int64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize array with pseudo-random data */
static void init_array(int32_t *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr[i] = (int32_t)lcg_rand();
    }
}

/* Complex shuffle mask computation - prevents constant propagation */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control + i * 3) % 32;
        if (mask_data[i] >= 16) {
            mask_data[i] = 31 - mask_data[i]; /* Non-linear transformation */
        }
    }
    
    /* Additional scrambling based on control */
    if (control & 1) {
        for (int i = 0; i < 8; i++) {
            int32_t temp = mask_data[i];
            mask_data[i] = mask_data[15 - i];
            mask_data[15 - i] = temp;
        }
    }
    
    return mask;
}

/* Chain of shuffles to accumulate operand count */
static int32x16_t shuffle_chain(int32x16_t a, int32x16_t b, 
                                int32x16_t c, int32x16_t d,
                                volatile int control) {
    /* First shuffle with computed mask */
    int32x16_t mask1 = compute_complex_mask(control);
    int32x16_t result1 = __builtin_shuffle(a, b, mask1);
    
    /* Second shuffle using result1 and another computed mask */
    int32x16_t mask2 = compute_complex_mask(control + 1);
    int32x16_t result2 = __builtin_shuffle(result1, c, mask2);
    
    /* Third shuffle with more complex pattern */
    int32x16_t mask3;
    int32_t *mask3_data = (int32_t*)&mask3;
    for (int i = 0; i < 16; i++) {
        mask3_data[i] = (control * i + 7) % 48;
        if (mask3_data[i] >= 32) mask3_data[i] -= 16;
    }
    
    /* This shuffle may require many operands during expansion */
    int32x16_t result3 = __builtin_shuffle(result2, d, mask3);
    
    return result3;
}

/* Conditional vector permutation */
static int32x16_t conditional_shuffle(int32x16_t a, int32x16_t b,
                                     int32x16_t c, int32x16_t d,
                                     volatile int condition) {
    int32x16_t mask_a = compute_complex_mask(condition);
    int32x16_t mask_b = compute_complex_mask(condition + 100);
    
    int32x16_t shuffle_a = __builtin_shuffle(a, b, mask_a);
    int32x16_t shuffle_b = __builtin_shuffle(c, d, mask_b);
    
    /* Conditional selection between two shuffle results */
    int32x16_t selector;
    int32_t *sel_data = (int32_t*)&selector;
    for (int i = 0; i < 16; i++) {
        sel_data[i] = (condition > (i * 10)) ? -1 : 0;
    }
    
    /* This creates a blend operation that may expand to many operands */
    int32x16_t result = (shuffle_a & selector) | (shuffle_b & ~selector);
    
    return result;
}

/* Mixed-type vector operations */
static void mixed_type_operations(float32x8_t *fvec, int32x8_t *ivec,
                                  float64x4_t *dvec, volatile int control) {
    /* Cast between types and shuffle */
    int32x8_t int_from_float = *(int32x8_t*)fvec;
    float32x8_t float_from_int = *(float32x8_t*)ivec;
    
    /* Create shuffle masks for different types */
    int32_t int_mask[8];
    for (int i = 0; i < 8; i++) {
        int_mask[i] = (control + i * 5) % 16;
    }
    
    /* Shuffle with mixed-type intermediate results */
    int32x8_t shuffled_int = __builtin_shuffle(int_from_float, *ivec, 
                                              *(int32x8_t*)int_mask);
    
    /* Chain another operation */
    float32x8_t temp_float = __builtin_shuffle(float_from_int, *fvec,
                                              *(int32x8_t*)int_mask);
    
    /* Store results */
    *ivec = shuffled_int;
    *fvec = temp_float;
}

/* Inline assembly with many vector operands */
static void multi_operand_assembly(int32x16_t *a, int32x16_t *b,
                                   int32x16_t *c, int32x16_t *d,
                                   int32x16_t *result) {
#ifdef __x86_64__
    /* x86-specific inline assembly with many vector operands */
    asm volatile (
        "vmovdqa %[vec_a], %%ymm0\n\t"
        "vmovdqa %[vec_b], %%ymm1\n\t"
        "vmovdqa %[vec_c], %%ymm2\n\t"
        "vmovdqa %[vec_d], %%ymm3\n\t"
        "vperm2i128 $0x21, %%ymm0, %%ymm1, %%ymm4\n\t"
        "vperm2i128 $0x21, %%ymm2, %%ymm3, %%ymm5\n\t"
        "vpblendd $0xF0, %%ymm4, %%ymm5, %%ymm6\n\t"
        "vmovdqa %%ymm6, %[out]\n\t"
        : [out] "=m" (*result)
        : [vec_a] "m" (*a),
          [vec_b] "m" (*b),
          [vec_c] "m" (*c),
          [vec_d] "m" (*d)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "memory"
    );
#elif defined(__aarch64__)
    /* ARM-specific inline assembly */
    asm volatile (
        "ld1 {v0.4s, v1.4s, v2.4s, v3.4s}, [%[vec_a]]\n\t"
        "ld1 {v4.4s, v5.4s, v6.4s, v7.4s}, [%[vec_b]]\n\t"
        "trn1 v16.4s, v0.4s, v4.4s\n\t"
        "trn2 v17.4s, v1.4s, v5.4s\n\t"
        "zip1 v18.4s, v2.4s, v6.4s\n\t"
        "zip2 v19.4s, v3.4s, v7.4s\n\t"
        "st1 {v16.4s, v17.4s, v18.4s, v19.4s}, [%[out]]\n\t"
        : 
        : [vec_a] "r" (a),
          [vec_b] "r" (b),
          [out] "r" (result)
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
          "v16", "v17", "v18", "v19", "memory"
    );
#else
    /* Generic fallback using builtin shuffle */
    int32x16_t mask;
    int32_t *mask_data = (int32_t*)&mask;
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (i * 3) % 32;
    }
    *result = __builtin_shuffle(*a, *b, mask);
#endif
}

/* Target-specific builtins wrapped in detection macros */
static int32x8_t use_target_specific_builtin(int32x8_t a, int32x8_t b) {
#ifdef __SSE2__
    /* x86 SSE/AVX builtins */
    return __builtin_ia32_pshufd(a, 0x1B);
#elif defined(__ARM_NEON)
    /* ARM NEON builtins */
    return __builtin_neon_vrev64q_s32(a);
#else
    /* Generic implementation */
    int32x8_t mask = {7, 6, 5, 4, 3, 2, 1, 0};
    return __builtin_shuffle(a, mask);
#endif
}

/* Compute checksum to prevent dead code elimination */
static int64_t compute_checksum(int32x16_t vec) {
    int64_t sum = 0;
    int32_t *data = (int32_t*)&vec;
    for (int i = 0; i < 16; i++) {
        sum += data[i];
    }
    return sum;
}

int main(void) {
    /* Initialize data arrays */
    const size_t ARRAY_SIZE = 1024;
    int32_t *data_a = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *data_b = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *data_c = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *data_d = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    
    init_array(data_a, ARRAY_SIZE);
    init_array(data_b, ARRAY_SIZE);
    init_array(data_c, ARRAY_SIZE);
    init_array(data_d, ARRAY_SIZE);
    
    /* Volatile control variable to prevent constant propagation */
    volatile int control = 42;
    
    /* Accumulate checksum */
    int64_t total_checksum = 0;
    
    /* Kernel 1: Complex shuffle with computed mask */
    for (int i = 0; i < ARRAY_SIZE - 64; i += 16) {
        int32x16_t *vec_a = (int32x16_t*)(data_a + i);
        int32x16_t *vec_b = (int32x16_t*)(data_b + i);
        
        int32x16_t mask = compute_complex_mask(control + i);
        int32x16_t result = __builtin_shuffle(*vec_a, *vec_b, mask);
        
        /* Store and compute checksum */
        int32x16_t *dest = (int32x16_t*)(data_c + i);
        *dest = result;
        total_checksum += compute_checksum(result);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 2: Chain of shuffles */
    for (int i = 0; i < ARRAY_SIZE - 128; i += 32) {
        int32x16_t vec_a = *(int32x16_t*)(data_a + i);
        int32x16_t vec_b = *(int32x16_t*)(data_b + i);
        int32x16_t vec_c = *(int32x16_t*)(data_c + i);
        int32x16_t vec_d = *(int32x16_t*)(data_d + i);
        
        int32x16_t result = shuffle_chain(vec_a, vec_b, vec_c, vec_d, control + i);
        
        *(int32x16_t*)(data_d + i) = result;
        total_checksum += compute_checksum(result);
        
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 3: Conditional vector permutation */
    for (int i = 0; i < ARRAY_SIZE - 64; i += 16) {
        int32x16_t vec_a = *(int32x16_t*)(data_a + i);
        int32x16_t vec_b = *(int32x16_t*)(data_b + i);
        int32x16_t vec_c = *(int32x16_t*)(data_c + i);
        int32x16_t vec_d = *(int32x16_t*)(data_d + i);
        
        int32x16_t result = conditional_shuffle(vec_a, vec_b, vec_c, vec_d, 
                                               control + (i % 100));
        
        *(int32x16_t*)(data_a + i) = result;
        total_checksum += compute_checksum(result);
        
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 4: Mixed-type operations */
    for (int i = 0; i < ARRAY_SIZE - 32; i += 8) {
        float32x8_t *fvec = (float32x8_t*)(data_b + i);
        int32x8_t *ivec = (int32x8_t*)(data_c + i);
        float64x4_t *dvec = (float64x4_t*)(data_d + i);
        
        mixed_type_operations(fvec, ivec, dvec, control + i);
        
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 5: Multi-operand inline assembly */
    for (int i = 0; i < ARRAY_SIZE - 64; i += 16) {
        int32x16_t *vec_a = (int32x16_t*)(data_a + i);
        int32x16_t *vec_b = (int32x16_t*)(data_b + i);
        int32x16_t *vec_c = (int32x16_t*)(data_c + i);
        int32x16_t *vec_d = (int32x16_t*)(data_d + i);
        int32x16_t result;
        
        multi_operand_assembly(vec_a, vec_b, vec_c, vec_d, &result);
        
        *(int32x16_t*)(data_b + i) = result;
        total_checksum += compute_checksum(result);
        
        asm volatile("" ::: "memory");
    }
    
    /* Use target-specific builtins */
    for (int i = 0; i < ARRAY_SIZE - 8; i += 8) {
        int32x8_t vec_a = *(int32x8_t*)(data_a + i);
        int32x8_t vec_b = *(int32x8_t*)(data_b + i);
        
        int32x8_t result = use_target_specific_builtin(vec_a, vec_b);
        
        *(int32x8_t*)(data_c + i) = result;
        
        /* Add to checksum */
        int32_t *data = (int32_t*)&result;
        for (int j = 0; j < 8; j++) {
            total_checksum += data[j];
        }
        
        asm volatile("" ::: "memory");
    }
    
    /* Final checksum output */
    printf("Total checksum: %ld\n", total_checksum);
    
    /* Cleanup */
    free(data_a);
    free(data_b);
    free(data_c);
    free(data_d);
    
    return 0;
}
