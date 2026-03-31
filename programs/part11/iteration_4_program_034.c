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

/* Initialize array with pseudo-random data */
static void init_array(int32_t *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr[i] = (int32_t)lcg_rand();
    }
}

/* Compute checksum to prevent dead code elimination */
static int64_t compute_checksum(int32_t *data, size_t size) {
    int64_t sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

int main(void) {
    /* Allocate and initialize large arrays */
    const size_t ARRAY_SIZE = 1024;
    int32_t *array_a = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *array_b = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *array_c = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *array_d = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    
    init_array(array_a, ARRAY_SIZE);
    init_array(array_b, ARRAY_SIZE);
    init_array(array_c, ARRAY_SIZE);
    init_array(array_d, ARRAY_SIZE);
    
    /* Volatile variables to prevent constant propagation */
    volatile int v_idx = 0;
    volatile int v_mask_sel = 1;
    
    /* Result arrays */
    int32_t result1[ARRAY_SIZE] __attribute__((aligned(64)));
    int32_t result2[ARRAY_SIZE] __attribute__((aligned(64)));
    int32_t result3[ARRAY_SIZE] __attribute__((aligned(64)));
    
    /* KERNEL 1: Complex shuffle with computed mask vector */
    for (int i = 0; i < ARRAY_SIZE - 31; i += 16) {
        /* Load vectors */
        int32x16_t vec_a = *(int32x16_t*)(&array_a[i]);
        int32x16_t vec_b = *(int32x16_t*)(&array_b[i]);
        
        /* Compute dynamic mask based on volatile variables */
        int32x16_t mask;
        for (int j = 0; j < 16; j++) {
            mask[j] = (v_idx + j * v_mask_sel) & 0x1F;
        }
        v_idx++;
        
        /* Complex shuffle with 3 vector operands (2 sources + mask) */
        int32x16_t shuffled = __builtin_shuffle(vec_a, vec_b, mask);
        
        /* Store result */
        *(int32x16_t*)(&result1[i]) = shuffled;
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 2: Chain of shuffles accumulating operand count */
    for (int i = 0; i < ARRAY_SIZE - 63; i += 16) {
        /* Load multiple vectors */
        int32x16_t v1 = *(int32x16_t*)(&array_a[i]);
        int32x16_t v2 = *(int32x16_t*)(&array_b[i]);
        int32x16_t v3 = *(int32x16_t*)(&array_c[i]);
        int32x16_t v4 = *(int32x16_t*)(&array_d[i]);
        
        /* Create complex mask using arithmetic */
        int32x16_t mask1, mask2;
        for (int j = 0; j < 16; j++) {
            mask1[j] = (i + j) % 32;
            mask2[j] = (i + j + 8) % 32;
        }
        
        /* Chain shuffles - each shuffle result feeds into next */
        int32x16_t s1 = __builtin_shuffle(v1, v2, mask1);
        int32x16_t s2 = __builtin_shuffle(v3, v4, mask2);
        
        /* Create another mask mixing results */
        int32x16_t mask3;
        for (int j = 0; j < 16; j++) {
            mask3[j] = (v_idx + j * 3) & 0x1F;
        }
        
        /* Final shuffle combining previous results */
        int32x16_t final = __builtin_shuffle(s1, s2, mask3);
        
        /* Store with offset to avoid overlap */
        *(int32x16_t*)(&result2[i]) = final;
        
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 3: Conditional vector permutation */
    for (int i = 0; i < ARRAY_SIZE - 31; i += 16) {
        int32x16_t vec_a = *(int32x16_t*)(&array_a[i]);
        int32x16_t vec_b = *(int32x16_t*)(&array_b[i]);
        
        /* Two different masks */
        int32x16_t mask_a, mask_b;
        for (int j = 0; j < 16; j++) {
            mask_a[j] = j * 2;
            mask_b[j] = j * 2 + 1;
        }
        
        /* Conditional selection of shuffle result */
        int32x16_t shuffled_a = __builtin_shuffle(vec_a, vec_b, mask_a);
        int32x16_t shuffled_b = __builtin_shuffle(vec_a, vec_b, mask_b);
        
        /* Use volatile to make condition dynamic */
        int32x16_t result = (v_mask_sel > 0) ? shuffled_a : shuffled_b;
        
        *(int32x16_t*)(&result3[i]) = result;
        
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 4: Mixed vector types and widths */
    for (int i = 0; i < ARRAY_SIZE - 63; i += 32) {
        /* Use different vector types */
        int32x8_t short_vec1 = *(int32x8_t*)(&array_a[i]);
        int32x8_t short_vec2 = *(int32x8_t*)(&array_b[i]);
        int32x16_t long_vec1 = *(int32x16_t*)(&array_c[i]);
        
        /* Shuffle with different sized vectors */
        int32x8_t mask_short;
        for (int j = 0; j < 8; j++) {
            mask_short[j] = (i + j) % 16;
        }
        
        /* This may require conversion between different vector sizes */
        int32x8_t shuffled_short = __builtin_shufflevector(short_vec1, short_vec2, 
            0, 2, 4, 6, 8, 10, 12, 14);
        
        /* Store back */
        *(int32x8_t*)(&result1[i]) = shuffled_short;
        
        asm volatile("" ::: "memory");
    }
    
#ifdef __x86_64__
    /* KERNEL 5: Target-specific builtins for x86 */
    for (int i = 0; i < ARRAY_SIZE - 15; i += 8) {
        int32x8_t vec = *(int32x8_t*)(&array_a[i]);
        
        /* Use x86-specific shuffle intrinsic if available */
        #ifdef __SSE2__
        /* Simulate complex operation with inline asm */
        int32x8_t result;
        asm volatile (
            "vmovdqu %1, %%ymm0\n\t"
            "vpshufd $0x1B, %%ymm0, %%ymm1\n\t"
            "vmovdqu %%ymm1, %0\n\t"
            : "=m"(result)
            : "m"(vec)
            : "ymm0", "ymm1", "memory"
        );
        *(int32x8_t*)(&result2[i]) = result;
        #endif
        
        asm volatile("" ::: "memory");
    }
#endif
    
#ifdef __ARM_NEON
    /* KERNEL 6: ARM-specific vector operations */
    for (int i = 0; i < ARRAY_SIZE - 15; i += 4) {
        int32x4_t vec = *(int32x4_t*)(&array_a[i]);
        
        /* Complex inline asm with multiple operands */
        int32x4_t result;
        asm volatile (
            "vld1.32 {%q0}, [%1]\n\t"
            "vrev64.32 %q0, %q0\n\t"
            "vst1.32 {%q0}, [%2]\n\t"
            : "=w"(result)
            : "r"(&array_a[i]), "r"(&result3[i])
            : "memory"
        );
    }
#endif
    
    /* Compute final checksum to prevent optimization */
    int64_t checksum1 = compute_checksum(result1, ARRAY_SIZE);
    int64_t checksum2 = compute_checksum(result2, ARRAY_SIZE);
    int64_t checksum3 = compute_checksum(result3, ARRAY_SIZE);
    
    printf("Checksums: %ld %ld %ld\n", 
           (long)checksum1, (long)checksum2, (long)checksum3);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
