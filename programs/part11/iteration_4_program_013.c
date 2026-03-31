#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef int64_t int64x8_t __attribute__((vector_size(64)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize array with pseudo-random data */
static void init_array(void *array, size_t size) {
    uint32_t *ptr = (uint32_t *)array;
    size_t count = size / sizeof(uint32_t);
    for (size_t i = 0; i < count; i++) {
        ptr[i] = lcg_rand();
    }
}

/* Compute checksum to prevent dead code elimination */
static int64_t compute_checksum(const void *data, size_t size) {
    const uint8_t *ptr = (const uint8_t *)data;
    int64_t sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += ptr[i];
    }
    return sum;
}

int main(void) {
    /* Large arrays to hold vector data */
    int32_t array_a[256] __attribute__((aligned(64)));
    int32_t array_b[256] __attribute__((aligned(64)));
    int32_t array_c[256] __attribute__((aligned(64)));
    int32_t array_d[256] __attribute__((aligned(64)));
    int32_t result_array[256] __attribute__((aligned(64)));
    
    /* Initialize with pseudo-random data */
    init_array(array_a, sizeof(array_a));
    init_array(array_b, sizeof(array_b));
    init_array(array_c, sizeof(array_c));
    init_array(array_d, sizeof(array_d));
    
    /* Volatile variable to prevent constant propagation */
    volatile int mask_seed = 42;
    
    int64_t total_checksum = 0;
    
    /* KERNEL 1: Complex shuffle with computed mask vector */
    for (int iter = 0; iter < 100; iter++) {
        /* Load vectors from arrays */
        int32x16_t vec_a = *(int32x16_t *)&array_a[iter * 16];
        int32x16_t vec_b = *(int32x16_t *)&array_b[iter * 16];
        int32x16_t vec_c = *(int32x16_t *)&array_c[iter * 16];
        
        /* Create a complex, non-constant mask using arithmetic */
        int32x16_t mask;
        for (int i = 0; i < 16; i++) {
            mask[i] = (i + mask_seed + iter) % 32;
        }
        
        /* Complex shuffle operation that may require many operands */
        int32x16_t shuffled = __builtin_shuffle(vec_a, vec_b, mask);
        
        /* Store result */
        *(int32x16_t *)&result_array[iter * 16] = shuffled;
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    total_checksum += compute_checksum(result_array, sizeof(result_array));
    
    /* KERNEL 2: Chain of shuffles accumulating operand count */
    for (int iter = 0; iter < 50; iter++) {
        /* Load multiple vectors */
        int32x16_t v1 = *(int32x16_t *)&array_a[iter * 32];
        int32x16_t v2 = *(int32x16_t *)&array_b[iter * 32];
        int32x16_t v3 = *(int32x16_t *)&array_c[iter * 32];
        int32x16_t v4 = *(int32x16_t *)&array_d[iter * 32];
        
        /* Create dynamic masks */
        int32x16_t mask1, mask2, mask3;
        for (int i = 0; i < 16; i++) {
            mask1[i] = (i * 3 + mask_seed) % 32;
            mask2[i] = (i * 5 + mask_seed + 1) % 32;
            mask3[i] = (i * 7 + mask_seed + 2) % 32;
        }
        
        /* Chain of shuffles - output of one becomes input to next */
        int32x16_t step1 = __builtin_shuffle(v1, v2, mask1);
        int32x16_t step2 = __builtin_shuffle(step1, v3, mask2);
        int32x16_t step3 = __builtin_shuffle(step2, v4, mask3);
        
        /* Mix with arithmetic to prevent simplification */
        step3 = step3 + v1;
        
        /* Store */
        *(int32x16_t *)&result_array[iter * 16] = step3;
        
        asm volatile("" ::: "memory");
    }
    
    total_checksum += compute_checksum(result_array, sizeof(result_array) / 2);
    
    /* KERNEL 3: Conditional vector permutation */
    for (int iter = 0; iter < 50; iter++) {
        int32x16_t v1 = *(int32x16_t *)&array_a[iter * 16];
        int32x16_t v2 = *(int32x16_t *)&array_b[iter * 16];
        
        /* Create two different masks */
        int32x16_t mask_a, mask_b;
        for (int i = 0; i < 16; i++) {
            mask_a[i] = (i + mask_seed) % 32;
            mask_b[i] = (32 - i + mask_seed) % 32;
        }
        
        /* Conditional selection between two shuffle results */
        int32x16_t shuffle_a = __builtin_shuffle(v1, v2, mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(v2, v1, mask_b);
        
        /* Use data-dependent condition */
        int32x16_t selector;
        for (int i = 0; i < 16; i++) {
            selector[i] = (v1[i] > v2[i]) ? -1 : 0;
        }
        
        /* Conditional vector selection */
        int32x16_t result = (selector & shuffle_a) | (~selector & shuffle_b);
        
        *(int32x16_t *)&result_array[iter * 16] = result;
        
        asm volatile("" ::: "memory");
    }
    
    total_checksum += compute_checksum(result_array, sizeof(result_array) / 2);
    
    /* KERNEL 4: Mixed vector types and widths */
    {
        /* Mix different vector types */
        float32x16_t fvec1 = *(float32x16_t *)array_a;
        float32x16_t fvec2 = *(float32x16_t *)array_b;
        
        /* Create mask for float vectors */
        int32x16_t float_mask;
        for (int i = 0; i < 16; i++) {
            float_mask[i] = (i * 2 + mask_seed) % 32;
        }
        
        /* Shuffle float vectors */
        float32x16_t fshuffled = __builtin_shuffle(fvec1, fvec2, float_mask);
        
        /* Convert to int and back */
        int32x16_t iconverted = *(int32x16_t *)&fshuffled;
        iconverted = iconverted + *(int32x16_t *)array_c;
        
        /* Store */
        *(int32x16_t *)result_array = iconverted;
        
        asm volatile("" ::: "memory");
    }
    
    total_checksum += compute_checksum(result_array, 64);
    
#ifdef __x86_64__
    /* KERNEL 5: Target-specific builtins for x86 */
    {
        int32x8_t xmm1 = *(int32x8_t *)&array_a[0];
        int32x8_t xmm2 = *(int32x8_t *)&array_b[0];
        
        /* Use x86-specific shuffle intrinsic if available */
#ifdef __SSE4_2__
        /* Complex inline assembly with many operands */
        asm volatile(
            "vperm2f128 $0x01, %1, %0, %0\n\t"
            "vpermilps $0x1B, %0, %0\n\t"
            : "+x" (xmm1)
            : "x" (xmm2)
            : "memory"
        );
#endif
        
        *(int32x8_t *)result_array = xmm1;
    }
    
    total_checksum += compute_checksum(result_array, 32);
#endif
    
#ifdef __ARM_NEON
    /* KERNEL 6: ARM NEON specific operations */
    {
        int32x4_t neon1 = *(int32x4_t *)&array_a[0];
        int32x4_t neon2 = *(int32x4_t *)&array_b[0];
        int32x4_t neon3 = *(int32x4_t *)&array_c[0];
        
        /* Complex inline assembly for ARM */
        asm volatile(
            "vrev64.32 %q0, %q1\n\t"
            "vadd.i32 %q0, %q0, %q2\n\t"
            : "=w" (neon1)
            : "w" (neon2), "w" (neon3)
            : "memory"
        );
        
        *(int32x4_t *)result_array = neon1;
    }
    
    total_checksum += compute_checksum(result_array, 16);
#endif
    
    /* KERNEL 7: Inline assembly with many vector operands */
    {
        int32x16_t asm_vec1 = *(int32x16_t *)&array_a[0];
        int32x16_t asm_vec2 = *(int32x16_t *)&array_b[0];
        int32x16_t asm_vec3 = *(int32x16_t *)&array_c[0];
        int32x16_t asm_vec4 = *(int32x16_t *)&array_d[0];
        
        /* Hypothetical multi-operand vector operation */
        asm volatile(
            "# Complex vector operation with many operands\n\t"
            "vmovdqa %1, %%ymm0\n\t"
            "vmovdqa %2, %%ymm1\n\t"
            "vmovdqa %3, %%ymm2\n\t"
            "vmovdqa %4, %%ymm3\n\t"
            "# ... more operations ...\n\t"
            "vpermq $0x1B, %%ymm0, %%ymm0\n\t"
            "vpaddd %%ymm0, %%ymm1, %%ymm4\n\t"
            "vpshufd $0x1B, %%ymm2, %%ymm5\n\t"
            "vpblendd $0xF0, %%ymm4, %%ymm5, %%ymm0\n\t"
            "vmovdqa %%ymm0, %0\n\t"
            : "=v" (asm_vec1)
            : "v" (asm_vec1), "v" (asm_vec2), "v" (asm_vec3), "v" (asm_vec4)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "memory"
        );
        
        *(int32x16_t *)result_array = asm_vec1;
    }
    
    total_checksum += compute_checksum(result_array, 64);
    
    /* Final checksum output to prevent optimization */
    printf("Total checksum: %ld\n", (long)total_checksum);
    
    return 0;
}
