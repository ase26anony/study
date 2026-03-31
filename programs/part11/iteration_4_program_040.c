#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
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

/* Initialize array with pseudo-random values */
static void init_array(int32_t *arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = (int32_t)lcg_rand();
    }
}

/* Compute mask vector with runtime-dependent values */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Volatile read to prevent constant propagation */
    volatile int seed = control;
    
    for (int i = 0; i < 16; i++) {
        /* Complex, non-constant mask calculation */
        mask_data[i] = (seed + i * 3) % 32;
        mask_data[i] ^= (seed >> 2);
        mask_data[i] &= 0x1F;
    }
    
    return mask;
}

/* Horizontal sum of vector elements */
static int32_t horizontal_sum_int32x16(int32x16_t v) {
    int32_t sum = 0;
    int32_t *data = (int32_t*)&v;
    for (int i = 0; i < 16; i++) {
        sum += data[i];
    }
    return sum;
}

static float horizontal_sum_float32x16(float32x16_t v) {
    float sum = 0.0f;
    float *data = (float*)&v;
    for (int i = 0; i < 16; i++) {
        sum += data[i];
    }
    return sum;
}

int main(void) {
    /* Large arrays to hold vector data */
    int32_t array_a[256];
    int32_t array_b[256];
    float float_array[256];
    
    /* Initialize with pseudo-random data */
    init_array(array_a, 256);
    init_array(array_b, 256);
    
    for (int i = 0; i < 256; i++) {
        float_array[i] = (float)lcg_rand() / 1000.0f;
    }
    
    /* Volatile control variable to prevent compile-time optimization */
    volatile int control_var = array_a[0] & 0xFF;
    
    int32_t checksum = 0;
    float float_checksum = 0.0f;
    
    /* ========== KERNEL 1: Complex shuffle with computed mask ========== */
    {
        /* Load vectors from different parts of arrays */
        int32x16_t vec_a = *((int32x16_t*)&array_a[0]);
        int32x16_t vec_b = *((int32x16_t*)&array_a[64]);
        
        /* Compute mask at runtime - prevents constant propagation */
        int32x16_t mask = compute_complex_mask(control_var);
        
        /* Complex shuffle operation that may require many operands */
        int32x16_t result1 = __builtin_shuffle(vec_a, vec_b, mask);
        
        /* Another shuffle with different mask pattern */
        int32x16_t mask2 = mask + (int32x16_t){1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
        int32x16_t result2 = __builtin_shuffle(vec_b, vec_a, mask2);
        
        /* Combine results */
        int32x16_t combined = result1 + result2;
        checksum += horizontal_sum_int32x16(combined);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* ========== KERNEL 2: Chain of shuffles ========== */
    {
        float32x16_t fvec_a = *((float32x16_t*)&float_array[0]);
        float32x16_t fvec_b = *((float32x16_t*)&float_array[32]);
        float32x16_t fvec_c = *((float32x16_t*)&float_array[64]);
        
        /* Create a complex mask using arithmetic */
        int32x16_t base_mask = compute_complex_mask(control_var + 1);
        int32x16_t float_mask = base_mask & (int32x16_t){15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15};
        
        /* Chain of shuffles - each result feeds into next operation */
        float32x16_t shuffle1 = __builtin_shuffle(fvec_a, fvec_b, float_mask);
        float32x16_t shuffle2 = __builtin_shuffle(shuffle1, fvec_c, float_mask + 
            (int32x16_t){1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1});
        float32x16_t shuffle3 = __builtin_shuffle(fvec_c, shuffle2, float_mask + 
            (int32x16_t){2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2});
        
        /* Final combination */
        float32x16_t final_result = shuffle1 + shuffle2 + shuffle3;
        float_checksum += horizontal_sum_float32x16(final_result);
        
        asm volatile("" ::: "memory");
    }
    
    /* ========== KERNEL 3: Conditional vector permutation ========== */
    {
        int32x8_t vec_small_a = *((int32x8_t*)&array_a[128]);
        int32x8_t vec_small_b = *((int32x8_t*)&array_a[160]);
        int32x8_t vec_small_c = *((int32x8_t*)&array_a[192]);
        
        /* Create different masks based on control flow */
        int32x8_t mask_a, mask_b;
        if (control_var & 1) {
            mask_a = (int32x8_t){0,2,4,6,1,3,5,7};
            mask_b = (int32x8_t){7,6,5,4,3,2,1,0};
        } else {
            mask_a = (int32x8_t){1,3,5,7,0,2,4,6};
            mask_b = (int32x8_t){0,1,2,3,4,5,6,7};
        }
        
        /* Conditional shuffle selection */
        int32x8_t shuffle_a = __builtin_shuffle(vec_small_a, vec_small_b, mask_a);
        int32x8_t shuffle_b = __builtin_shuffle(vec_small_b, vec_small_c, mask_b);
        
        /* Use conditional operator on vectors */
        int32x8_t selected = (control_var & 2) ? shuffle_a : shuffle_b;
        
        /* Additional shufflevector with many operands */
        int32x8_t extended_shuffle = __builtin_shufflevector(
            vec_small_a, vec_small_b, vec_small_c,
            0, 8, 1, 9, 2, 10, 3, 11);
        
        int32x8_t combined_small = selected + extended_shuffle;
        int32_t *data = (int32_t*)&combined_small;
        for (int i = 0; i < 8; i++) {
            checksum += data[i];
        }
        
        asm volatile("" ::: "memory");
    }
    
    /* ========== KERNEL 4: Inline assembly with many operands ========== */
    {
        /* Use architecture-specific builtins when available */
#ifdef __x86_64__
        /* x86 specific vector operations */
        int32x8_t x86_vec_a = *((int32x8_t*)&array_a[0]);
        int32x8_t x86_vec_b = *((int32x8_t*)&array_a[8]);
        
        /* Inline asm with multiple vector operands */
        int32x8_t asm_result;
        asm volatile (
            "vpaddd %0, %1, %2\n\t"
            "vpshufd $0x1B, %0, %0\n\t"
            : "=x"(asm_result)
            : "x"(x86_vec_a), "x"(x86_vec_b)
            : "memory"
        );
        
        int32_t *asm_data = (int32_t*)&asm_result;
        for (int i = 0; i < 8; i++) {
            checksum += asm_data[i];
        }
#endif
        
#ifdef __ARM_NEON
        /* ARM NEON specific operations */
        int32x4_t neon_vec_a = *((int32x4_t*)&array_a[0]);
        int32x4_t neon_vec_b = *((int32x4_t*)&array_a[4]);
        
        int32x4_t neon_result;
        asm volatile (
            "vadd.i32 %0, %1, %2\n\t"
            "vrev64.32 %0, %0\n\t"
            : "=w"(neon_result)
            : "w"(neon_vec_a), "w"(neon_vec_b)
            : "memory"
        );
        
        int32_t *neon_data = (int32_t*)&neon_result;
        for (int i = 0; i < 4; i++) {
            checksum += neon_data[i];
        }
#endif
        
        asm volatile("" ::: "memory");
    }
    
    /* ========== KERNEL 5: Loop-dependent vector operations ========== */
    {
        /* Process vectors in a loop with data-dependent masks */
        int32x16_t accum = {0};
        
        for (int iter = 0; iter < 4; iter++) {
            /* Load vectors from different offsets */
            int offset = iter * 16;
            int32x16_t loop_vec_a = *((int32x16_t*)&array_a[offset]);
            int32x16_t loop_vec_b = *((int32x16_t*)&array_b[offset]);
            
            /* Compute mask based on loop iteration and data */
            int32x16_t loop_mask;
            int32_t *mask_ptr = (int32_t*)&loop_mask;
            for (int i = 0; i < 16; i++) {
                /* Data-dependent mask calculation */
                volatile int temp = array_a[offset + i] & 0xF;
                mask_ptr[i] = (temp + i + iter) % 32;
            }
            
            /* Shuffle operation inside loop */
            int32x16_t loop_result = __builtin_shuffle(loop_vec_a, loop_vec_b, loop_mask);
            
            /* Accumulate results */
            accum += loop_result;
        }
        
        checksum += horizontal_sum_int32x16(accum);
    }
    
    /* ========== KERNEL 6: Mixed type permutations ========== */
    {
        /* Mix different vector types and sizes */
        float64x8_t double_vec = *((float64x8_t*)&float_array[0]);
        float64x4_t double_half = *((float64x4_t*)&float_array[32]);
        
        /* Convert between types and shuffle */
        int32x8_t int_vec = __builtin_convertvector(double_half, int32x8_t);
        int32x16_t expanded_int = __builtin_shufflevector(
            int_vec, int_vec,
            0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7);
        
        /* Complex operation chain */
        int32x16_t mask3 = compute_complex_mask(control_var + 2);
        int32x16_t final_mixed = __builtin_shuffle(expanded_int, expanded_int + 
            (int32x16_t){1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}, mask3);
        
        checksum += horizontal_sum_int32x16(final_mixed);
    }
    
    /* Final checksum output to prevent dead code elimination */
    printf("Integer checksum: %d\n", checksum);
    printf("Float checksum: %f\n", float_checksum);
    
    return 0;
}
