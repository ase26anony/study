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

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Function to create non-constant mask vectors */
static inline int32x16_t create_complex_mask(volatile int offset) {
    int32x16_t mask = {0};
    int32_t *mask_ptr = (int32_t*)&mask;
    
    /* Data-dependent mask generation */
    for (int i = 0; i < 16; i++) {
        mask_ptr[i] = (i + offset) % 32;
        if (i % 3 == 0) mask_ptr[i] = (i * 2 + offset) % 32;
        if (i % 5 == 0) mask_ptr[i] = (i + offset * 3) % 32;
    }
    
    /* Force compiler to keep this dynamic */
    asm volatile("" : "+r"(offset) : : "memory");
    return mask;
}

/* Complex shuffle chain that may require many operands */
static void kernel_complex_shuffle(int32x16_t *result, 
                                   const int32x16_t *a, 
                                   const int32x16_t *b,
                                   volatile int mask_offset) {
    
    /* Create non-constant mask */
    int32x16_t mask1 = create_complex_mask(mask_offset);
    int32x16_t mask2 = create_complex_mask(mask_offset + 1);
    
    /* First shuffle with 3 operands (2 sources + mask) */
    int32x16_t shuffle1 = __builtin_shuffle(*a, *b, mask1);
    
    /* Second shuffle using result of first as source */
    int32x16_t shuffle2 = __builtin_shuffle(shuffle1, *a, mask2);
    
    /* Third shuffle with mixed sources */
    int32x16_t mask3 = create_complex_mask(mask_offset + 2);
    int32x16_t shuffle3 = __builtin_shuffle(shuffle2, *b, mask3);
    
    /* Chain more shuffles to increase operand complexity */
    int32x16_t mask4 = create_complex_mask(mask_offset + 3);
    int32x16_t shuffle4 = __builtin_shuffle(shuffle3, shuffle1, mask4);
    
    int32x16_t mask5 = create_complex_mask(mask_offset + 4);
    *result = __builtin_shuffle(shuffle4, shuffle2, mask5);
}

/* Mixed-type permutation operations */
static void kernel_mixed_types(float32x8_t *f_result,
                               int32x8_t *i_result,
                               const float32x8_t *f_vec,
                               const int32x8_t *i_vec,
                               volatile int offset) {
    
    /* Create masks for different vector types */
    int32x8_t int_mask = {0};
    int32_t *int_mask_ptr = (int32_t*)&int_mask;
    for (int i = 0; i < 8; i++) {
        int_mask_ptr[i] = (i + offset) % 16;
    }
    
    /* Shuffle integer vector */
    int32x8_t shuffled_int = __builtin_shuffle(*i_vec, *i_vec, int_mask);
    
    /* Convert and shuffle float vector */
    int32x8_t float_as_int = __builtin_convertvector(*f_vec, int32x8_t);
    int32x8_t shuffled_float_int = __builtin_shuffle(float_as_int, shuffled_int, int_mask);
    
    /* Convert back and shuffle again */
    float32x8_t temp_float = __builtin_convertvector(shuffled_float_int, float32x8_t);
    
    /* Another shuffle with the float result */
    int32x8_t mask2 = {7, 6, 5, 4, 3, 2, 1, 0};
    for (int i = 0; i < 8; i++) {
        ((int32_t*)&mask2)[i] = ((int32_t*)&mask2)[i] + offset;
    }
    
    *f_result = __builtin_shuffle(temp_float, temp_float, mask2);
    *i_result = shuffled_int;
}

/* Conditional vector permutation */
static void kernel_conditional_permute(int32x16_t *result,
                                       const int32x16_t *a,
                                       const int32x16_t *b,
                                       volatile int condition) {
    
    int32x16_t mask1 = create_complex_mask(condition);
    int32x16_t mask2 = create_complex_mask(condition + 100);
    
    /* Create two different shuffle results */
    int32x16_t shuffle1 = __builtin_shuffle(*a, *b, mask1);
    int32x16_t shuffle2 = __builtin_shuffle(*b, *a, mask2);
    
    /* Create a selector mask based on condition */
    int32x16_t selector = {0};
    int32_t *sel_ptr = (int32_t*)&selector;
    for (int i = 0; i < 16; i++) {
        sel_ptr[i] = (condition > (i * 10)) ? -1 : 0;
    }
    
    /* Conditional selection between two shuffle results */
    *result = (selector != 0) ? shuffle1 : shuffle2;
    
    /* Additional shuffle chain */
    int32x16_t mask3 = create_complex_mask(condition * 2);
    *result = __builtin_shuffle(*result, shuffle1, mask3);
}

/* Inline assembly with many vector operands */
static void kernel_inline_asm(int32x16_t *out1, int32x16_t *out2,
                              const int32x16_t *in1, const int32x16_t *in2,
                              const int32x16_t *in3) {
    
#ifdef __x86_64__
    /* x86-specific inline assembly with many vector operands */
    asm volatile (
        "vmovdqa %[vec1], %%ymm0\n\t"
        "vmovdqa %[vec2], %%ymm1\n\t"
        "vmovdqa %[vec3], %%ymm2\n\t"
        "vpermq $0x1B, %%ymm0, %%ymm3\n\t"
        "vpermq $0x4E, %%ymm1, %%ymm4\n\t"
        "vpblendd $0xF0, %%ymm3, %%ymm4, %%ymm5\n\t"
        "vmovdqa %%ymm5, %[out1]\n\t"
        "vpshufd $0x1B, %%ymm2, %%ymm6\n\t"
        "vmovdqa %%ymm6, %[out2]\n\t"
        : [out1] "=m" (*out1), [out2] "=m" (*out2)
        : [vec1] "m" (*in1), [vec2] "m" (*in2), [vec3] "m" (*in3)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "memory"
    );
#elif defined(__aarch64__)
    /* ARM-specific inline assembly */
    asm volatile (
        "ld1 {v0.4s, v1.4s, v2.4s, v3.4s}, [%[vec1]]\n\t"
        "ld1 {v4.4s, v5.4s, v6.4s, v7.4s}, [%[vec2]]\n\t"
        "trn1 v8.4s, v0.4s, v4.4s\n\t"
        "trn2 v9.4s, v1.4s, v5.4s\n\t"
        "zip1 v10.4s, v2.4s, v6.4s\n\t"
        "zip2 v11.4s, v3.4s, v7.4s\n\t"
        "st1 {v8.4s, v9.4s, v10.4s, v11.4s}, [%[out1]]\n\t"
        : 
        : [vec1] "r" (in1), [vec2] "r" (in2), [out1] "r" (out1)
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", 
          "v8", "v9", "v10", "v11", "memory"
    );
    *out2 = *in3;
#endif
}

/* Compute checksum to prevent dead code elimination */
static int64_t compute_checksum(const int32x16_t *vec, size_t count) {
    int64_t sum = 0;
    const int32_t *ptr = (const int32_t*)vec;
    
    for (size_t i = 0; i < count * 16; i++) {
        sum += ptr[i];
    }
    
    return sum;
}

int main(void) {
    /* Initialize large arrays with pseudo-random data */
    const size_t ARRAY_SIZE = 1024;
    int32_t data_a[ARRAY_SIZE];
    int32_t data_b[ARRAY_SIZE];
    float float_data[ARRAY_SIZE];
    
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        data_a[i] = (int32_t)lcg_rand();
        data_b[i] = (int32_t)lcg_rand();
        float_data[i] = (float)lcg_rand() / 1000.0f;
    }
    
    /* Cast to vector types */
    const int32x16_t *vec_a = (const int32x16_t*)data_a;
    const int32x16_t *vec_b = (const int32x16_t*)data_b;
    const float32x8_t *f_vec = (const float32x8_t*)float_data;
    
    /* Result storage */
    int32x16_t results[16];
    float32x8_t float_results[16];
    int32x8_t int_results[16];
    
    volatile int dynamic_offset = 0;
    
    /* Kernel 1: Complex shuffle with computed masks */
    for (int i = 0; i < 4; i++) {
        kernel_complex_shuffle(&results[i], &vec_a[i], &vec_b[i], dynamic_offset + i);
        dynamic_offset += results[i][0]; /* Data-dependent update */
        asm volatile("" ::: "memory"); /* Compiler barrier */
    }
    
    /* Kernel 2: Mixed type permutations */
    for (int i = 0; i < 4; i++) {
        kernel_mixed_types(&float_results[i], &int_results[i], 
                          &f_vec[i*2], (const int32x8_t*)&data_a[i*8], 
                          dynamic_offset + i);
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 3: Conditional permutations */
    for (int i = 4; i < 8; i++) {
        kernel_conditional_permute(&results[i], &vec_a[i], &vec_b[i], dynamic_offset);
        dynamic_offset += i;
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 4: Inline assembly with many operands */
    for (int i = 8; i < 12; i += 2) {
        kernel_inline_asm(&results[i], &results[i+1], 
                         &vec_a[i], &vec_b[i], &vec_a[i+1]);
        asm volatile("" ::: "memory");
    }
    
    /* Additional complex shufflevector operations */
    for (int i = 12; i < 16; i++) {
        /* __builtin_shufflevector can require many operands */
        int32x8_t vec1 = __builtin_convertvector(float_results[i-12], int32x8_t);
        int32x8_t vec2 = int_results[i-12];
        
        /* Create a large shufflevector operation */
        int32x16_t large_shuffle = __builtin_shufflevector(
            vec1, vec2, 
            0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15
        );
        
        /* Mix with another vector */
        int32x16_t mixed = __builtin_shuffle(
            large_shuffle, 
            results[i-4],
            create_complex_mask(dynamic_offset + i)
        );
        
        results[i] = mixed;
        asm volatile("" ::: "memory");
    }
    
    /* Compute final checksum */
    int64_t checksum = compute_checksum(results, 16);
    
    /* Add checksums from all result arrays */
    for (int i = 0; i < 16; i++) {
        checksum += compute_checksum(&results[i], 1);
    }
    
    printf("Final checksum: %ld\n", (long)checksum);
    
    return 0;
}
