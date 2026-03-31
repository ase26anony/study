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
        arr1[i] = (int32_t)lcg_rand();
        arr2[i] = (int32_t)lcg_rand();
        arr3[i] = (int32_t)lcg_rand();
    }
}

/* Complex mask computation preventing constant propagation */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control + i * 3) % 32;
        if (i % 4 == 0) mask_data[i] = (mask_data[i] + control * 2) % 32;
        if (i % 3 == 0) mask_data[i] = mask_data[i] ^ control;
    }
    
    return mask;
}

/* Another mask with different pattern */
static int32x16_t compute_alternate_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control * 7 + i * 5) % 32;
        mask_data[i] = mask_data[i] < 16 ? mask_data[i] : 31 - mask_data[i];
    }
    
    return mask;
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    int32_t *array1 = malloc(ARRAY_SIZE * sizeof(int32_t));
    int32_t *array2 = malloc(ARRAY_SIZE * sizeof(int32_t));
    int32_t *array3 = malloc(ARRAY_SIZE * sizeof(int32_t));
    
    init_arrays(array1, array2, array3, ARRAY_SIZE);
    
    /* Use volatile variables to prevent compile-time optimization */
    volatile int control_var = 7;
    volatile int loop_counter = 0;
    
    /* Result storage */
    int32x16_t results[4] = {0};
    float64x8_t float_results[2] = {0};
    
    /* Kernel 1: Complex shuffle with two source vectors and computed mask */
    for (int iter = 0; iter < 100; iter++) {
        control_var = (control_var * 13 + 17) % 101;
        
        /* Load vectors from different array positions */
        int32x16_t vec_a = *((int32x16_t*)&array1[iter * 4]);
        int32x16_t vec_b = *((int32x16_t*)&array2[iter * 4]);
        int32x16_t vec_c = *((int32x16_t*)&array3[iter * 4]);
        
        /* Compute dynamic mask */
        int32x16_t mask1 = compute_complex_mask(control_var);
        
        /* Complex shuffle operation - may require many operands during expansion */
        int32x16_t shuffled1 = __builtin_shuffle(vec_a, vec_b, mask1);
        
        /* Chain another shuffle with different mask */
        int32x16_t mask2 = compute_alternate_mask(control_var + 1);
        int32x16_t shuffled2 = __builtin_shuffle(shuffled1, vec_c, mask2);
        
        /* Store result */
        results[0] = shuffled2;
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Kernel 2: Chain of shuffles accumulating operand count */
        float32x16_t float_vec_a = *((float32x16_t*)&array1[iter * 4]);
        float32x16_t float_vec_b = *((float32x16_t*)&array2[iter * 4]);
        
        /* Convert to different vector type for mixed-type operations */
        int32x16_t int_vec = *((int32x16_t*)&float_vec_a);
        
        /* Multiple chained permutations */
        int32x16_t temp1 = __builtin_shuffle(int_vec, vec_b, mask1);
        int32x16_t temp2 = __builtin_shuffle(temp1, vec_c, mask2);
        int32x16_t temp3 = __builtin_shuffle(temp2, shuffled1, mask1);
        
        /* Mixed-type shuffle using shufflevector */
        int32x16_t mixed_shuffle = __builtin_shufflevector(
            temp1, temp3, 
            0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30
        );
        
        results[1] = mixed_shuffle;
        
        /* Kernel 3: Conditional vector permutation */
        int32x16_t mask3 = compute_complex_mask(control_var * 2);
        int32x16_t alt_shuffle1 = __builtin_shuffle(vec_a, vec_b, mask3);
        int32x16_t alt_shuffle2 = __builtin_shuffle(vec_b, vec_c, mask2);
        
        /* Conditional selection between shuffle results */
        int32x16_t conditional_result = (control_var % 2) ? alt_shuffle1 : alt_shuffle2;
        
        /* Additional shuffle on conditional result */
        results[2] = __builtin_shuffle(conditional_result, mixed_shuffle, mask1);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Kernel 4: Inline assembly with many vector operands */
        #ifdef __x86_64__
        /* x86-specific vector operations */
        asm volatile(
            "vmovdqa %[vec1], %%ymm0\n\t"
            "vmovdqa %[vec2], %%ymm1\n\t"
            "vmovdqa %[vec3], %%ymm2\n\t"
            "vpermq $0x1B, %%ymm0, %%ymm3\n\t"
            "vpermq $0x4E, %%ymm1, %%ymm4\n\t"
            "vpblendd $0xF0, %%ymm3, %%ymm4, %%ymm5\n\t"
            "vmovdqa %%ymm5, %[result]\n\t"
            : [result] "=m" (results[3])
            : [vec1] "m" (vec_a),
              [vec2] "m" (vec_b),
              [vec3] "m" (vec_c)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "memory"
        );
        #elif defined(__aarch64__)
        /* ARM-specific vector operations */
        asm volatile(
            "ld1 {v0.4s, v1.4s, v2.4s, v3.4s}, [%[vec1]]\n\t"
            "ld1 {v4.4s, v5.4s, v6.4s, v7.4s}, [%[vec2]]\n\t"
            "trn1 v8.4s, v0.4s, v4.4s\n\t"
            "trn2 v9.4s, v1.4s, v5.4s\n\t"
            "zip1 v10.4s, v8.4s, v9.4s\n\t"
            "st1 {v10.4s, v11.4s, v12.4s, v13.4s}, [%[result]]\n\t"
            : 
            : [vec1] "r" (&vec_a),
              [vec2] "r" (&vec_b),
              [result] "r" (&results[3])
            : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
              "v8", "v9", "v10", "v11", "v12", "v13", "memory"
        );
        #endif
        
        loop_counter++;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < 4; i++) {
        int32_t *data = (int32_t*)&results[i];
        for (int j = 0; j < 16; j++) {
            checksum += data[j];
        }
    }
    
    printf("Checksum: %ld\n", (long)checksum);
    printf("Loop iterations: %d\n", loop_counter);
    
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
