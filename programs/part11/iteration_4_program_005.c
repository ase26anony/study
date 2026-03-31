#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef int32_t int32x4_t __attribute__((vector_size(16)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef float float32x8_t __attribute__((vector_size(32)));
typedef double float64x8_t __attribute__((vector_size(64)));
typedef double float64x4_t __attribute__((vector_size(32)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize array with pseudo-random data */
static void init_array(int32_t *arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = (int32_t)(lcg_rand() % 1000);
    }
}

/* Compute checksum to prevent dead code elimination */
static int64_t compute_checksum(int32_t *arr, size_t n) {
    int64_t sum = 0;
    for (size_t i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Complex mask computation - prevents constant propagation */
static int32x16_t compute_dynamic_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t *)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control + i * 3) % 32;
        if (mask_data[i] < 0) mask_data[i] += 32;
    }
    
    return mask;
}

/* Another mask computation with different pattern */
static int32x16_t compute_alternate_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t *)&mask;
    
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control * 7 + i * 5) % 32;
        mask_data[i] = mask_data[i] < 16 ? mask_data[i] : mask_data[i] - 16 + 100;
    }
    
    return mask;
}

int main(void) {
    /* Allocate aligned memory for vector operations */
    const size_t ARRAY_SIZE = 1024;
    int32_t *data_a = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *data_b = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *data_c = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *result = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    
    if (!data_a || !data_b || !data_c || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_array(data_a, ARRAY_SIZE);
    init_array(data_b, ARRAY_SIZE);
    init_array(data_c, ARRAY_SIZE);
    memset(result, 0, ARRAY_SIZE * sizeof(int32_t));
    
    volatile int control_var = 42; /* Volatile to prevent optimization */
    int64_t total_checksum = 0;
    
    /* KERNEL 1: Complex shuffle with computed mask vector */
    for (int iter = 0; iter < 10; iter++) {
        for (size_t i = 0; i < ARRAY_SIZE - 31; i += 16) {
            /* Load vectors */
            int32x16_t vec_a = *((int32x16_t*)(data_a + i));
            int32x16_t vec_b = *((int32x16_t*)(data_b + i));
            
            /* Compute dynamic mask - prevents constant propagation */
            int32x16_t mask = compute_dynamic_mask(control_var + iter);
            
            /* Complex shuffle operation that may require many operands during expansion */
            int32x16_t shuffled = __builtin_shuffle(vec_a, vec_b, mask);
            
            /* Store result */
            *((int32x16_t*)(result + i)) = shuffled;
        }
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Update control variable */
        control_var += result[iter % 16];
    }
    
    total_checksum += compute_checksum(result, ARRAY_SIZE);
    
    /* KERNEL 2: Chain of shuffles accumulating operand count */
    for (int iter = 0; iter < 8; iter++) {
        for (size_t i = 0; i < ARRAY_SIZE - 47; i += 16) {
            /* Load multiple vectors */
            int32x16_t vec_a = *((int32x16_t*)(data_a + i));
            int32x16_t vec_b = *((int32x16_t*)(data_b + i));
            int32x16_t vec_c = *((int32x16_t*)(data_c + i));
            
            /* First shuffle */
            int32x16_t mask1 = compute_dynamic_mask(control_var + iter * 2);
            int32x16_t shuffled1 = __builtin_shuffle(vec_a, vec_b, mask1);
            
            /* Second shuffle using result of first */
            int32x16_t mask2 = compute_alternate_mask(control_var + iter * 3 + 1);
            int32x16_t shuffled2 = __builtin_shuffle(shuffled1, vec_c, mask2);
            
            /* Third shuffle chaining more results */
            int32x16_t mask3 = compute_dynamic_mask(control_var + iter * 5 + 2);
            int32x16_t shuffled3 = __builtin_shuffle(shuffled2, vec_a, mask3);
            
            /* Store final result */
            *((int32x16_t*)(result + i)) = shuffled3;
        }
        
        asm volatile("" ::: "memory");
        control_var += result[(iter * 7) % 16];
    }
    
    total_checksum += compute_checksum(result, ARRAY_SIZE);
    
    /* KERNEL 3: Conditional vector permutation */
    for (int iter = 0; iter < 6; iter++) {
        for (size_t i = 0; i < ARRAY_SIZE - 31; i += 16) {
            int32x16_t vec_a = *((int32x16_t*)(data_a + i));
            int32x16_t vec_b = *((int32x16_t*)(data_b + i));
            int32x16_t vec_c = *((int32x16_t*)(data_c + i));
            
            /* Compute two different masks */
            int32x16_t mask_a = compute_dynamic_mask(control_var + iter);
            int32x16_t mask_b = compute_alternate_mask(control_var - iter);
            
            /* Two different shuffle results */
            int32x16_t shuffle_a = __builtin_shuffle(vec_a, vec_b, mask_a);
            int32x16_t shuffle_b = __builtin_shuffle(vec_b, vec_c, mask_b);
            
            /* Conditional selection between shuffle results */
            int32x16_t selector;
            int32_t *sel_data = (int32_t *)&selector;
            for (int j = 0; j < 16; j++) {
                sel_data[j] = (control_var + i + j) % 2 ? -1 : 0;
            }
            
            int32x16_t final_result = selector ? shuffle_a : shuffle_b;
            
            *((int32x16_t*)(result + i)) = final_result;
        }
        
        asm volatile("" ::: "memory");
        control_var ^= result[(iter * 11) % 16];
    }
    
    total_checksum += compute_checksum(result, ARRAY_SIZE);
    
    /* KERNEL 4: Mixed vector types and widths */
    for (int iter = 0; iter < 4; iter++) {
        for (size_t i = 0; i < ARRAY_SIZE - 63; i += 32) {
            /* Use different vector types and sizes */
            float32x16_t fvec_a = *((float32x16_t*)(data_a + i));
            float32x16_t fvec_b = *((float32x16_t*)(data_b + i));
            
            /* Convert to different type for shuffle */
            int32x16_t ivec_a = *((int32x16_t*)(data_a + i));
            int32x16_t ivec_b = *((int32x16_t*)(data_b + i + 16));
            
            /* Complex mask mixing different patterns */
            int32x16_t mask;
            int32_t *mask_data = (int32_t *)&mask;
            for (int j = 0; j < 16; j++) {
                mask_data[j] = (control_var * j + iter * 17) % 32;
                if (mask_data[j] >= 16) {
                    mask_data[j] = mask_data[j] - 16 + 50; /* Offset for second vector */
                }
            }
            
            /* Shuffle with mixed-type intermediate */
            int32x16_t int_shuffled = __builtin_shuffle(ivec_a, ivec_b, mask);
            
            /* Convert back and mix with float vectors */
            float32x16_t float_shuffled = __builtin_convertvector(int_shuffled, float32x16_t);
            float32x16_t mixed = fvec_a + float_shuffled * 0.5f;
            
            /* Store through int pointer */
            *((float32x16_t*)(result + i)) = mixed;
        }
        
        asm volatile("" ::: "memory");
        control_var = (control_var * 37 + 1) % 1000;
    }
    
    total_checksum += compute_checksum(result, ARRAY_SIZE);
    
#ifdef __x86_64__
    /* KERNEL 5: x86-specific builtins with many operands */
    for (int iter = 0; iter < 2; iter++) {
        for (size_t i = 0; i < ARRAY_SIZE - 15; i += 8) {
            int32x8_t vec_a = *((int32x8_t*)(data_a + i));
            int32x8_t vec_b = *((int32x8_t*)(data_b + i));
            
            /* Use x86-specific shuffle intrinsic if available */
            #ifdef __SSE2__
            /* Simulate complex multi-operand operation with inline asm */
            int32x8_t result_vec;
            asm volatile (
                "vmovdqa %[va], %%ymm0\n\t"
                "vmovdqa %[vb], %%ymm1\n\t"
                /* Complex permutation sequence */
                "vpshufd $0x1B, %%ymm0, %%ymm2\n\t"  /* Reverse elements */
                "vpshufd $0x4E, %%ymm1, %%ymm3\n\t"  /* Swap halves */
                "vpblendd $0xF0, %%ymm2, %%ymm3, %%ymm0\n\t"  /* Blend */
                "vmovdqa %%ymm0, %[out]"
                : [out] "=v" (result_vec)
                : [va] "v" (vec_a), [vb] "v" (vec_b)
                : "ymm0", "ymm1", "ymm2", "ymm3", "memory"
            );
            
            *((int32x8_t*)(result + i)) = result_vec;
            #endif
        }
        
        asm volatile("" ::: "memory");
    }
    
    total_checksum += compute_checksum(result, ARRAY_SIZE / 2);
#endif
    
#ifdef __ARM_NEON
    /* KERNEL 6: ARM-specific builtins */
    for (int iter = 0; iter < 2; iter++) {
        for (size_t i = 0; i < ARRAY_SIZE - 15; i += 4) {
            int32x4_t vec_a = *((int32x4_t*)(data_a + i));
            int32x4_t vec_b = *((int32x4_t*)(data_b + i));
            
            /* Use ARM NEON intrinsics */
            int32x4_t rev_a = __builtin_neon_vrev64q_s32(vec_a);
            int32x4_t rev_b = __builtin_neon_vrev64q_s32(vec_b);
            
            /* Complex operation mixing results */
            int32x4_t result_vec = __builtin_shuffle(rev_a, rev_b, 
                (int32x4_t){2, 3, 0, 1});
            
            *((int32x4_t*)(result + i)) = result_vec;
        }
        
        asm volatile("" ::: "memory");
    }
    
    total_checksum += compute_checksum(result, ARRAY_SIZE / 4);
#endif
    
    /* Final checksum output to prevent dead code elimination */
    printf("Total checksum: %ld\n", (long)total_checksum);
    
    /* Cleanup */
    free(data_a);
    free(data_b);
    free(data_c);
    free(result);
    
    return 0;
}
