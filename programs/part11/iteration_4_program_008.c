#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Vector type definitions */
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
static void init_arrays(int32_t* arr32, size_t n32, float* arrf, size_t nf) {
    for (size_t i = 0; i < n32; i++) {
        arr32[i] = (int32_t)lcg_rand();
    }
    for (size_t i = 0; i < nf; i++) {
        arrf[i] = (float)(lcg_rand() / 4294967296.0);
    }
}

/* Complex mask computation - prevents constant propagation */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t* mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask generation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control + i * 3) % 32;
        if (i % 4 == 0) mask_data[i] += control % 5;
        if (i % 3 == 0) mask_data[i] -= control % 7;
    }
    
    /* Additional non-linear transformation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (mask_data[i] * 13 + 7) % 32;
        mask_data[i] = mask_data[i] < 0 ? mask_data[i] + 32 : mask_data[i];
    }
    
    return mask;
}

/* Another mask with different pattern */
static int32x16_t compute_alternate_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t* mask_data = (int32_t*)&mask;
    
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control * i + 17) % 32;
        mask_data[i] ^= (control << 3);
        mask_data[i] &= 31;
    }
    
    return mask;
}

int main(void) {
    /* Large arrays for vector operations */
    int32_t data_int[256] __attribute__((aligned(64)));
    float data_float[256] __attribute__((aligned(64)));
    volatile int control_var = 42; /* Volatile to prevent optimization */
    
    /* Initialize with pseudo-random data */
    init_arrays(data_int, 256, data_float, 256);
    
    /* Cast to various vector types */
    int32x16_t* vec_int16 = (int32x16_t*)data_int;
    int32x8_t* vec_int8 = (int32x8_t*)data_int;
    float32x16_t* vec_float16 = (float32x16_t*)data_float;
    float64x8_t* vec_double8 = (float64x8_t*)data_float;
    
    /* Intermediate result storage */
    int32x16_t results[4] __attribute__((aligned(64)));
    float32x16_t float_results[2] __attribute__((aligned(64)));
    
    /* KERNEL 1: Complex shuffle with computed mask */
    /* This should generate many operands during RTL expansion */
    asm volatile("" ::: "memory"); /* Compiler barrier */
    
    int32x16_t mask1 = compute_complex_mask(control_var);
    int32x16_t mask2 = compute_alternate_mask(control_var + 1);
    
    /* Complex shuffle operation - likely to require many operands */
    results[0] = __builtin_shuffle(vec_int16[0], vec_int16[1], mask1);
    results[1] = __builtin_shuffle(vec_int16[2], vec_int16[3], mask2);
    
    /* Mixed shuffle with different vector types */
    int32x16_t temp_mask = mask1 + mask2;
    for (int i = 0; i < 16; i++) {
        ((int32_t*)&temp_mask)[i] &= 31;
    }
    
    /* KERNEL 2: Chain of shuffles - increases operand count */
    asm volatile("" ::: "memory");
    
    /* Chain multiple shuffle operations */
    int32x16_t chain1 = __builtin_shuffle(results[0], results[1], temp_mask);
    int32x16_t chain2 = __builtin_shuffle(chain1, vec_int16[4], mask1);
    int32x16_t chain3 = __builtin_shuffle(chain2, vec_int16[5], mask2);
    results[2] = __builtin_shuffle(chain3, vec_int16[6], temp_mask);
    
    /* KERNEL 3: Conditional vector permutation */
    asm volatile("" ::: "memory");
    
    /* Compute two different shuffle results */
    int32x16_t shuffle_a = __builtin_shuffle(vec_int16[7], vec_int16[8], mask1);
    int32x16_t shuffle_b = __builtin_shuffle(vec_int16[9], vec_int16[10], mask2);
    
    /* Conditional selection between shuffle results */
    int32x16_t condition_mask;
    for (int i = 0; i < 16; i++) {
        ((int32_t*)&condition_mask)[i] = (control_var > 20) ? -1 : 0;
    }
    
    results[3] = __builtin_shuffle(
        __builtin_shuffle(shuffle_a, shuffle_b, mask1),
        __builtin_shuffle(shuffle_b, shuffle_a, mask2),
        condition_mask
    );
    
    /* KERNEL 4: Inline assembly with many vector operands */
    asm volatile("" ::: "memory");
    
    /* Inline assembly that uses many vector registers */
    /* This may directly trigger the multi-operand expansion */
#if defined(__x86_64__) || defined(__i386__)
    /* x86-specific vector operations */
    asm volatile(
        "vmovdqa %[vec1], %%ymm0\n\t"
        "vmovdqa %[vec2], %%ymm1\n\t"
        "vmovdqa %[mask], %%ymm2\n\t"
        "vpermd %%ymm0, %%ymm2, %%ymm3\n\t"
        "vpermd %%ymm1, %%ymm2, %%ymm4\n\t"
        "vpaddd %%ymm3, %%ymm4, %%ymm5\n\t"
        "vmovdqa %%ymm5, %[result]\n\t"
        : [result] "=m" (results[0])
        : [vec1] "m" (vec_int16[0]),
          [vec2] "m" (vec_int16[1]),
          [mask] "m" (mask1)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "memory"
    );
#elif defined(__aarch64__)
    /* ARM-specific vector operations */
    asm volatile(
        "ld1 {v0.4s, v1.4s, v2.4s, v3.4s}, [%[vec1]]\n\t"
        "ld1 {v4.4s, v5.4s, v6.4s, v7.4s}, [%[vec2]]\n\t"
        "tbl v8.16b, {v0.16b, v1.16b, v2.16b, v3.16b}, v16.16b\n\t"
        "tbl v9.16b, {v4.16b, v5.16b, v6.16b, v7.16b}, v17.16b\n\t"
        "add v10.4s, v8.4s, v9.4s\n\t"
        "st1 {v10.4s, v11.4s, v12.4s, v13.4s}, [%[result]]\n\t"
        : [result] "=m" (results[0])
        : [vec1] "r" (vec_int16),
          [vec2] "r" (&vec_int16[1])
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
          "v8", "v9", "v10", "v11", "v12", "v13", "v16", "v17", "memory"
    );
#endif
    
    /* Additional complex shufflevector operations */
    /* These builtins can generate many operands */
    asm volatile("" ::: "memory");
    
    /* __builtin_shufflevector with many elements */
    int32x8_t shufflevec_result = __builtin_shufflevector(
        vec_int8[0], vec_int8[1],
        0, 8, 1, 9, 2, 10, 3, 11,
        4, 12, 5, 13, 6, 14, 7, 15
    );
    
    /* Store to prevent elimination */
    memcpy(&results[0], &shufflevec_result, sizeof(int32x8_t));
    
    /* Loop-dependent vector operations */
    asm volatile("" ::: "memory");
    
    for (volatile int iter = 0; iter < 3; iter++) {
        /* Data-dependent mask computation inside loop */
        int32x16_t loop_mask = compute_complex_mask(control_var + iter);
        
        /* Complex shuffle that depends on loop iteration */
        int32x16_t loop_vec1 = __builtin_shuffle(
            vec_int16[iter % 4],
            vec_int16[(iter + 1) % 4],
            loop_mask
        );
        
        int32x16_t loop_vec2 = __builtin_shuffle(
            vec_int16[(iter + 2) % 4],
            vec_int16[(iter + 3) % 4],
            loop_mask + mask1
        );
        
        /* Nested shuffle operation */
        results[iter % 2] = __builtin_shuffle(
            loop_vec1,
            loop_vec2,
            loop_mask ^ mask2
        );
    }
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < 4; i++) {
        int32_t* data = (int32_t*)&results[i];
        for (int j = 0; j < 16; j++) {
            checksum += (uint64_t)(data[j] & 0xFFFF);
        }
    }
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    return 0;
}
