#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
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

/* Complex mask computation that prevents constant propagation */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control & (1 << (i % 8))) ? (i + 16) : (i % 16);
        /* Add some arithmetic to prevent simplification */
        mask_data[i] += (lcg_rand() % 3) - 1;
    }
    
    return mask;
}

int main(void) {
    /* Allocate and initialize data arrays */
    const size_t ARRAY_SIZE = 256;
    int32_t *data_a = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *data_b = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *data_c = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *result = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    
    init_array(data_a, ARRAY_SIZE);
    init_array(data_b, ARRAY_SIZE);
    init_array(data_c, ARRAY_SIZE);
    
    volatile int control_var = 0;
    int64_t checksum = 0;
    
    /* KERNEL 1: Complex shuffle with computed mask */
    for (int iter = 0; iter < 100; iter++) {
        control_var = iter & 0xFF;
        
        for (size_t i = 0; i < ARRAY_SIZE - 32; i += 16) {
            /* Load vectors */
            int32x16_t vec_a = *((int32x16_t*)(data_a + i));
            int32x16_t vec_b = *((int32x16_t*)(data_b + i));
            
            /* Compute complex mask that can't be constant folded */
            int32x16_t mask = compute_complex_mask(control_var);
            
            /* This shuffle with large vectors and computed mask may require
               many operands during RTL expansion */
            int32x16_t shuffled = __builtin_shuffle(vec_a, vec_b, mask);
            
            /* Store result */
            *((int32x16_t*)(result + i)) = shuffled;
        }
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 2: Chain of shuffles to accumulate operand count */
    for (int iter = 0; iter < 50; iter++) {
        control_var = lcg_rand() & 0xFF;
        
        for (size_t i = 0; i < ARRAY_SIZE - 64; i += 16) {
            /* Load multiple vectors */
            int32x16_t v1 = *((int32x16_t*)(data_a + i));
            int32x16_t v2 = *((int32x16_t*)(data_b + i));
            int32x16_t v3 = *((int32x16_t*)(data_c + i));
            
            /* Create complex mask using arithmetic */
            int32x16_t mask1, mask2;
            int32_t *m1 = (int32_t*)&mask1;
            int32_t *m2 = (int32_t*)&mask2;
            
            for (int j = 0; j < 16; j++) {
                m1[j] = (j * 3 + control_var) % 32;
                m2[j] = (j * 5 + (control_var >> 2)) % 32;
            }
            
            /* Chain shuffles - output of first is input to second */
            int32x16_t step1 = __builtin_shuffle(v1, v2, mask1);
            int32x16_t step2 = __builtin_shuffle(step1, v3, mask2);
            
            /* Third shuffle with mixed sources */
            int32x16_t mask3 = {0, 16, 1, 17, 2, 18, 3, 19,
                                4, 20, 5, 21, 6, 22, 7, 23};
            int32x16_t final_result = __builtin_shuffle(step2, v1, mask3);
            
            *((int32x16_t*)(result + i)) = final_result;
        }
        
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 3: Conditional vector permutations */
    for (int iter = 0; iter < 30; iter++) {
        control_var = iter * 7;
        
        for (size_t i = 0; i < ARRAY_SIZE - 32; i += 8) {
            /* Use different vector types */
            int32x8_t vec_int = *((int32x8_t*)(data_a + i));
            float32x8_t vec_float = *((float32x8_t*)(data_b + i));
            
            /* Create masks */
            int32x8_t mask_a = {7, 6, 5, 4, 3, 2, 1, 0};
            int32x8_t mask_b = {0, 2, 4, 6, 1, 3, 5, 7};
            
            /* Conditional shuffle selection */
            int32x8_t shuffled_int = __builtin_shuffle(vec_int, mask_a);
            int32x8_t shuffled_int2 = __builtin_shuffle(vec_int, mask_b);
            
            /* Use conditional operator on vectors - may generate complex expansion */
            int32x8_t selected = (control_var & 0x10) ? shuffled_int : shuffled_int2;
            
            /* Convert and store */
            *((int32x8_t*)(result + i)) = selected;
        }
        
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 4: Inline assembly with many vector operands */
#ifdef __x86_64__
    for (int iter = 0; iter < 20; iter++) {
        for (size_t i = 0; i < ARRAY_SIZE - 32; i += 8) {
            int32x8_t v1, v2, v3, v4, vout;
            
            v1 = *((int32x8_t*)(data_a + i));
            v2 = *((int32x8_t*)(data_b + i));
            v3 = *((int32x8_t*)(data_c + i));
            v4 = *((int32x8_t*)(result + i));
            
            /* Inline assembly with multiple vector operands */
            asm volatile (
                "vpaddd %[vout], %[v1], %[v2]\n\t"
                "vpshufd %[vout], %[vout], 0x1B\n\t"
                "vpaddd %[vout], %[vout], %[v3]\n\t"
                "vpsubd %[vout], %[vout], %[v4]"
                : [vout] "=x" (vout)
                : [v1] "x" (v1), [v2] "x" (v2), [v3] "x" (v3), [v4] "x" (v4)
                : "memory"
            );
            
            *((int32x8_t*)(result + i)) = vout;
        }
        
        asm volatile("" ::: "memory");
    }
#endif
    
    /* KERNEL 5: Mixed-width vector operations */
    for (int iter = 0; iter < 25; iter++) {
        control_var = lcg_rand();
        
        for (size_t i = 0; i < ARRAY_SIZE - 16; i += 8) {
            /* Use 64-bit vectors */
            int64x8_t vec64_a = *((int64x8_t*)(data_a + i));
            int64x8_t vec64_b = *((int64x8_t*)(data_b + i));
            
            /* Create complex 64-bit mask */
            int64x8_t mask64;
            int64_t *m64 = (int64_t*)&mask64;
            for (int j = 0; j < 8; j++) {
                m64[j] = (j + (control_var & 7)) % 8;
                m64[j] |= ((j + (control_var >> 3)) % 8) << 32;
            }
            
            /* Shuffle with 64-bit vectors */
            int64x8_t shuffled64 = __builtin_shuffle(vec64_a, vec64_b, mask64);
            
            /* Convert back to 32-bit and store */
            *((int64x8_t*)(result + i)) = shuffled64;
        }
        
        asm volatile("" ::: "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        checksum += result[i];
    }
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int64_t final_checksum = checksum;
    printf("Checksum: %ld\n", (long)final_checksum);
    
    /* Cleanup */
    free(data_a);
    free(data_b);
    free(data_c);
    free(result);
    
    return 0;
}
