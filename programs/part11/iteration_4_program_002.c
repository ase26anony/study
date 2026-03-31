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

/* Initialize array with pseudo-random values */
static void init_array(int32_t *arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = (int32_t)lcg_rand();
    }
}

/* Complex mask computation - prevents constant propagation */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask generation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control + i * 3) % 32;
        mask_data[i] = mask_data[i] < 0 ? mask_data[i] + 32 : mask_data[i];
    }
    
    return mask;
}

/* Another mask with different pattern */
static int32x16_t compute_alternate_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control * 7 + i * 5) % 32;
        mask_data[i] = mask_data[i] < 16 ? mask_data[i] + 16 : mask_data[i] - 16;
    }
    
    return mask;
}

int main(void) {
    /* Large arrays to hold vector data */
    int32_t array_a[256] __attribute__((aligned(64)));
    int32_t array_b[256] __attribute__((aligned(64)));
    int32_t array_c[256] __attribute__((aligned(64)));
    int32_t result[256] __attribute__((aligned(64)));
    
    volatile int control_var = 42; /* Volatile to prevent optimization */
    
    /* Initialize with pseudo-random data */
    init_array(array_a, 256);
    init_array(array_b, 256);
    init_array(array_c, 256);
    
    /* Cast to various vector types */
    int32x16_t *vec_a = (int32x16_t*)array_a;
    int32x16_t *vec_b = (int32x16_t*)array_b;
    int32x16_t *vec_c = (int32x16_t*)array_c;
    int32x16_t *vec_result = (int32x16_t*)result;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 1: Complex shuffle with computed mask */
    printf("Running Kernel 1: Complex shuffle with computed mask\n");
    for (int i = 0; i < 8; i++) {
        /* Compute mask based on loop index and volatile control */
        int32x16_t mask = compute_complex_mask(control_var + i);
        
        /* Complex shuffle operation - should expand to many operands */
        vec_result[i] = __builtin_shuffle(vec_a[i], vec_b[i], mask);
        
        /* Additional shuffle with different sources */
        int32x16_t alt_mask = compute_alternate_mask(control_var - i);
        vec_result[i+8] = __builtin_shuffle(vec_result[i], vec_c[i], alt_mask);
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 2: Chain of shuffles accumulating operand count */
    printf("Running Kernel 2: Chain of shuffles\n");
    for (int i = 0; i < 4; i++) {
        /* Start with initial vectors */
        int32x16_t v1 = vec_a[i*2];
        int32x16_t v2 = vec_b[i*2];
        int32x16_t v3 = vec_c[i*2];
        int32x16_t v4 = vec_a[i*2+1];
        
        /* Chain multiple shuffle operations */
        int32x16_t mask1 = compute_complex_mask(i * 11);
        int32x16_t mask2 = compute_alternate_mask(i * 13);
        int32x16_t mask3 = compute_complex_mask(i * 17);
        
        /* Chain that could require many operands during expansion */
        int32x16_t temp1 = __builtin_shuffle(v1, v2, mask1);
        int32x16_t temp2 = __builtin_shuffle(v3, v4, mask2);
        int32x16_t temp3 = __builtin_shuffle(temp1, temp2, mask3);
        
        /* Additional shuffle with all previous results */
        int32x16_t mask4 = compute_alternate_mask(i * 19);
        vec_result[i+16] = __builtin_shuffle(temp1, temp3, mask4);
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 3: Conditional vector permutation */
    printf("Running Kernel 3: Conditional vector permutation\n");
    for (int i = 0; i < 4; i++) {
        int32x16_t mask_a = compute_complex_mask(i * 23);
        int32x16_t mask_b = compute_alternate_mask(i * 29);
        
        /* Conditional selection between two different shuffle results */
        int32x16_t shuffle_a = __builtin_shuffle(vec_a[i], vec_b[i], mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(vec_b[i], vec_c[i], mask_b);
        
        /* Use conditional operator - may generate complex expansion */
        vec_result[i+20] = (control_var & (1 << i)) ? shuffle_a : shuffle_b;
        
        /* Mixed type operations */
        float32x16_t *float_vec_a = (float32x16_t*)array_a;
        float32x16_t *float_vec_b = (float32x16_t*)array_b;
        float32x16_t float_temp = __builtin_shuffle(float_vec_a[i], float_vec_b[i], mask_a);
        
        /* Convert and store */
        int32x16_t *converted = (int32x16_t*)&float_temp;
        vec_result[i+24] = vec_result[i+24] + *converted;
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 4: Inline assembly with many operands */
    printf("Running Kernel 4: Inline assembly\n");
    
#ifdef __x86_64__
    /* x86-specific vector operations */
    for (int i = 0; i < 2; i++) {
        int32x8_t vx = ((int32x8_t*)array_a)[i];
        int32x8_t vy = ((int32x8_t*)array_b)[i];
        int32x8_t vz = ((int32x8_t*)array_c)[i];
        int32x8_t vw = ((int32x8_t*)array_a)[i+2];
        
        /* Inline asm with multiple vector operands */
        asm volatile (
            "vmovdqa %1, %%ymm0\n\t"
            "vmovdqa %2, %%ymm1\n\t"
            "vmovdqa %3, %%ymm2\n\t"
            "vmovdqa %4, %%ymm3\n\t"
            "vpermq $0x1B, %%ymm0, %%ymm4\n\t"
            "vpermq $0x39, %%ymm1, %%ymm5\n\t"
            "vpblendd $0xF0, %%ymm4, %%ymm5, %%ymm6\n\t"
            "vmovdqa %%ymm6, %0\n\t"
            : "=m" (vec_result[i+28])
            : "m" (vx), "m" (vy), "m" (vz), "m" (vw)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "memory"
        );
    }
#endif
    
#ifdef __aarch64__
    /* ARM-specific operations */
    for (int i = 0; i < 2; i++) {
        int32x8_t va = ((int32x8_t*)array_a)[i];
        int32x8_t vb = ((int32x8_t*)array_b)[i];
        
        asm volatile (
            "ld1 {v0.4s, v1.4s}, [%1]\n\t"
            "ld1 {v2.4s, v3.4s}, [%2]\n\t"
            "rev64 v4.4s, v0.4s\n\t"
            "rev64 v5.4s, v2.4s\n\t"
            "zip1 v6.4s, v4.4s, v5.4s\n\t"
            "zip2 v7.4s, v4.4s, v5.4s\n\t"
            "st1 {v6.4s, v7.4s}, [%0]\n\t"
            : "=r" (&vec_result[i+30])
            : "r" (&va), "r" (&vb)
            : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "memory"
        );
    }
#endif
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 5: Mixed-width vector operations */
    printf("Running Kernel 5: Mixed-width operations\n");
    {
        /* Use vector_size 32 and 64 types together */
        int32x8_t medium_vec[4];
        for (int i = 0; i < 4; i++) {
            medium_vec[i] = ((int32x8_t*)array_a)[i * 2];
        }
        
        /* Combine medium vectors into large vectors */
        for (int i = 0; i < 2; i++) {
            /* This may require complex expansion with many operands */
            int32x16_t combined = __builtin_shufflevector(
                medium_vec[i*2], medium_vec[i*2+1],
                0, 1, 2, 3, 4, 5, 6, 7,
                8, 9, 10, 11, 12, 13, 14, 15
            );
            
            /* Additional manipulation */
            int32x16_t mask = compute_complex_mask(i * 31);
            vec_result[i+32] = __builtin_shuffle(combined, vec_b[i+32], mask);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    printf("Computing checksum\n");
    int64_t checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += result[i];
    }
    
    /* Also checksum the vector data directly */
    for (int i = 0; i < 16; i++) {
        int32_t *data = (int32_t*)&vec_result[i];
        for (int j = 0; j < 16; j++) {
            checksum += data[j];
        }
    }
    
    printf("Final checksum: %ld\n", (long)checksum);
    
    return 0;
}
