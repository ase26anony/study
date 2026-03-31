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
static void init_array(int32_t *arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = (int32_t)(lcg_rand() % 1000);
    }
}

/* Compute checksum to prevent dead code elimination */
static int64_t compute_checksum(int32x16_t vec) {
    int64_t sum = 0;
    int32_t *ptr = (int32_t *)&vec;
    for (int i = 0; i < 16; i++) {
        sum += ptr[i];
    }
    return sum;
}

int main(void) {
    /* Allocate and initialize data arrays */
    int32_t data_a[256];
    int32_t data_b[256];
    int32_t data_c[256];
    int32_t data_d[256];
    int32_t mask_data[256];
    
    init_array(data_a, 256);
    init_array(data_b, 256);
    init_array(data_c, 256);
    init_array(data_d, 256);
    init_array(mask_data, 256);
    
    /* Volatile variable to prevent constant propagation */
    volatile int dynamic_offset = 0;
    
    /* Cast array segments to vector types */
    int32x16_t *vec_a = (int32x16_t *)data_a;
    int32x16_t *vec_b = (int32x16_t *)data_b;
    int32x16_t *vec_c = (int32x16_t *)data_c;
    int32x16_t *vec_d = (int32x16_t *)data_d;
    int32x16_t *mask_vec = (int32x16_t *)mask_data;
    
    /* Result vectors */
    int32x16_t result1, result2, result3, result4;
    int64_t checksum = 0;
    
    /* KERNEL 1: Complex shuffle with computed mask vector */
    for (int iter = 0; iter < 100; iter++) {
        /* Dynamic mask calculation - prevents compile-time simplification */
        int32x16_t computed_mask;
        for (int i = 0; i < 16; i++) {
            ((int32_t *)&computed_mask)[i] = 
                (mask_data[i + dynamic_offset] & 31) | 
                ((i < 8) ? 0 : 16);  /* Mix elements from both vectors */
        }
        
        /* This shuffle with 3 vector operands expands to many RTL operands */
        result1 = __builtin_shuffle(vec_a[iter % 4], vec_b[iter % 4], computed_mask);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 2: Chain of shuffles accumulating operand count */
    for (int iter = 0; iter < 50; iter++) {
        /* First shuffle */
        int32x16_t temp1 = __builtin_shuffle(
            vec_a[iter % 4], 
            vec_b[iter % 4],
            (int32x16_t){0,1,2,3,16,17,18,19,4,5,6,7,20,21,22,23}
        );
        
        /* Second shuffle using result of first */
        int32x16_t temp2 = __builtin_shuffle(
            temp1,
            vec_c[iter % 4],
            (int32x16_t){16,17,18,19,0,1,2,3,20,21,22,23,4,5,6,7}
        );
        
        /* Third shuffle chaining more vectors */
        result2 = __builtin_shuffle(
            temp2,
            vec_d[iter % 4],
            mask_vec[iter % 4]  /* Non-constant mask */
        );
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 3: Conditional vector permutation */
    for (int iter = 0; iter < 50; iter++) {
        /* Create two different shuffle results */
        int32x16_t shuffle_a = __builtin_shuffle(
            vec_a[iter % 4], vec_b[iter % 4],
            (int32x16_t){0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23}
        );
        
        int32x16_t shuffle_b = __builtin_shuffle(
            vec_c[iter % 4], vec_d[iter % 4],
            (int32x16_t){23,22,21,20,19,18,17,16,7,6,5,4,3,2,1,0}
        );
        
        /* Conditional selection between shuffle results */
        int32x16_t condition = vec_a[iter % 4] > vec_b[iter % 4];
        result3 = __builtin_shuffle(
            shuffle_a, shuffle_b,
            (condition != (int32x16_t){0}) ? 
                (int32x16_t){0,1,2,3,16,17,18,19,4,5,6,7,20,21,22,23} :
                (int32x16_t){16,17,18,19,0,1,2,3,20,21,22,23,4,5,6,7}
        );
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 4: Mixed vector types and widths */
    {
        /* Use different vector types */
        float32x8_t *fvec_a = (float32x8_t *)data_a;
        float32x8_t *fvec_b = (float32x8_t *)data_b;
        
        /* Shuffle between different vector types (requires conversion) */
        int32x8_t int_shuffle = __builtin_shuffle(
            (int32x8_t)fvec_a[0], (int32x8_t)fvec_b[0],
            (int32x8_t){0,8,1,9,2,10,3,11}
        );
        
        /* Chain with another operation */
        result4 = __builtin_shufflevector(
            (int32x16_t){0},  /* Padding */
            (int32x16_t)int_shuffle,
            16,17,18,19,20,21,22,23,  /* First 8 from padding (zeros) */
            0,1,2,3,4,5,6,7           /* Last 8 from int_shuffle */
        );
    }
    
    /* KERNEL 5: Inline assembly with many vector operands */
#ifdef __x86_64__
    for (int iter = 0; iter < 10; iter++) {
        /* Inline asm with multiple vector operands */
        asm volatile (
            "vmovdqa %[vec1], %%ymm0\n\t"
            "vmovdqa %[vec2], %%ymm1\n\t"
            "vmovdqa %[vec3], %%ymm2\n\t"
            "vmovdqa %[vec4], %%ymm3\n\t"
            "vpermq $0x1B, %%ymm0, %%ymm4\n\t"
            "vpermq $0x1B, %%ymm1, %%ymm5\n\t"
            "vpblendd $0xF0, %%ymm4, %%ymm5, %%ymm6\n\t"
            "vmovdqa %%ymm6, %[result]\n\t"
            : [result] "=v" (result1)
            : [vec1] "v" (vec_a[iter % 4]),
              [vec2] "v" (vec_b[iter % 4]),
              [vec3] "v" (vec_c[iter % 4]),
              [vec4] "v" (vec_d[iter % 4])
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "memory"
        );
    }
#endif
    
    /* KERNEL 6: Architecture-specific builtins */
#ifdef __SSE4_2__
    for (int iter = 0; iter < 10; iter++) {
        /* Use x86-specific shuffle intrinsic */
        int32x8_t sse_vec1 = (int32x8_t)vec_a[iter % 4];
        int32x8_t sse_vec2 = (int32x8_t)vec_b[iter % 4];
        
        /* __builtin_ia32_pshufd takes immediate mask + vector */
        int32x8_t shuffled = __builtin_ia32_pshufd(sse_vec1, 0x1B);
        
        /* Chain with another operation */
        int32x8_t blended = __builtin_ia32_pblendd128(shuffled, sse_vec2, 0xF0);
        
        /* Expand back to larger vector */
        result2 = __builtin_shufflevector(
            (int32x16_t){0},
            (int32x16_t)blended,
            16,17,18,19,20,21,22,23,8,9,10,11,12,13,14,15
        );
    }
#endif
    
    /* KERNEL 7: Loop-dependent vector operations with volatile control */
    volatile int control = 42;
    for (int iter = 0; iter < 100; iter++) {
        /* Data-dependent mask calculation */
        int32x16_t dynamic_mask;
        for (int i = 0; i < 16; i++) {
            ((int32_t *)&dynamic_mask)[i] = 
                (data_a[iter * 16 + i] > control) ? i : (31 - i);
        }
        
        /* Complex shuffle with data-dependent mask */
        result3 = __builtin_shuffle(
            vec_a[iter % 4],
            vec_b[iter % 4],
            dynamic_mask
        );
        
        /* Modify control variable */
        control = (control * 13 + 7) & 0xFF;
    }
    
    /* Compute final checksum to prevent optimization */
    checksum += compute_checksum(result1);
    checksum += compute_checksum(result2);
    checksum += compute_checksum(result3);
    checksum += compute_checksum(result4);
    
    printf("Checksum: %ld\n", (long)checksum);
    
    return 0;
}
