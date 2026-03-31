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

/* Initialize array with pseudo-random data */
static void init_array(int32_t *arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = (int32_t)lcg_rand();
    }
}

/* Complex mask computation preventing constant propagation */
static int32x16_t compute_complex_mask(int32x16_t base, volatile int *control) {
    int32x16_t mask = base;
    /* Data-dependent mask modification */
    for (int i = 0; i < 16; i++) {
        mask[i] = (mask[i] + *control) & 0x1F;
    }
    return mask;
}

/* Horizontal sum for checksum */
static int32_t horizontal_sum(int32x16_t v) {
    int32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += v[i];
    }
    return sum;
}

int main(void) {
    /* Large arrays for vector operations */
    int32_t data_a[256];
    int32_t data_b[256];
    int32_t data_c[256];
    int32_t results[256];
    
    /* Volatile control variables to prevent optimization */
    volatile int control1 = 7;
    volatile int control2 = 13;
    volatile int control3 = 19;
    
    /* Initialize with pseudo-random data */
    init_array(data_a, 256);
    init_array(data_b, 256);
    init_array(data_c, 256);
    memset(results, 0, sizeof(results));
    
    /* Cast to vector types */
    int32x16_t *vec_a = (int32x16_t *)data_a;
    int32x16_t *vec_b = (int32x16_t *)data_b;
    int32x16_t *vec_c = (int32x16_t *)data_c;
    int32x16_t *vec_results = (int32x16_t *)results;
    
    int32_t checksum = 0;
    
    /* KERNEL 1: Complex shuffle with computed mask */
    for (int i = 0; i < 8; i++) {
        /* Create base mask with arithmetic */
        int32x16_t base_mask;
        for (int j = 0; j < 16; j++) {
            base_mask[j] = (j * control1 + i) & 0x1F;
        }
        
        /* Compute dynamic mask */
        int32x16_t mask = compute_complex_mask(base_mask, &control1);
        
        /* Complex shuffle with 3 vectors and computed mask */
        /* This may expand to many operands during RTL generation */
        int32x16_t temp1 = __builtin_shuffle(vec_a[i], vec_b[i], mask);
        
        /* Another shuffle with different mask */
        for (int j = 0; j < 16; j++) {
            mask[j] = (mask[j] + control2) & 0x1F;
        }
        
        int32x16_t temp2 = __builtin_shuffle(temp1, vec_c[i], mask);
        
        /* Store result */
        vec_results[i] = temp2;
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 2: Chain of shuffles accumulating operand count */
    for (int i = 0; i < 4; i++) {
        /* Initial shuffle */
        int32x16_t shuffle_mask1;
        for (int j = 0; j < 16; j++) {
            shuffle_mask1[j] = (j + control3 * i) & 0x1F;
        }
        
        int32x16_t chain1 = __builtin_shuffle(vec_a[i*2], vec_b[i*2], shuffle_mask1);
        
        /* Second shuffle using result as input */
        int32x16_t shuffle_mask2;
        for (int j = 0; j < 16; j++) {
            shuffle_mask2[j] = (shuffle_mask1[j] + 8) & 0x1F;
        }
        
        int32x16_t chain2 = __builtin_shuffle(chain1, vec_c[i*2], shuffle_mask2);
        
        /* Third shuffle with mixed sources */
        int32x16_t shuffle_mask3;
        for (int j = 0; j < 16; j++) {
            shuffle_mask3[j] = (shuffle_mask2[j] * 3 + j) & 0x1F;
        }
        
        int32x16_t chain3 = __builtin_shuffle(chain2, vec_a[i*2+1], shuffle_mask3);
        
        /* Complex operation combining multiple shuffles */
        int32x16_t final_result = chain1 + chain2 - chain3;
        
        vec_results[8 + i] = final_result;
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 3: Conditional vector permutation */
    for (int i = 0; i < 4; i++) {
        /* Create two different masks */
        int32x16_t mask_a, mask_b;
        for (int j = 0; j < 16; j++) {
            mask_a[j] = (j * 5 + control1) & 0x1F;
            mask_b[j] = (j * 7 + control2) & 0x1F;
        }
        
        /* Two different shuffle results */
        int32x16_t shuffle_a = __builtin_shuffle(vec_b[i+4], vec_c[i+4], mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(vec_a[i+4], vec_c[i+4], mask_b);
        
        /* Conditional selection between shuffle results */
        int32x16_t selector;
        for (int j = 0; j < 16; j++) {
            selector[j] = (vec_a[i+4][j] > 0) ? -1 : 0;
        }
        
        /* Conditional permutation - may require many operands */
        int32x16_t cond_result = (selector & shuffle_a) | (~selector & shuffle_b);
        
        vec_results[12 + i] = cond_result;
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 4: Mixed vector types and widths */
    {
        /* Use different vector types */
        float32x16_t *float_vec_a = (float32x16_t *)data_a;
        float32x16_t *float_vec_b = (float32x16_t *)data_b;
        
        /* Complex operation with type conversion */
        int32x16_t int_mask;
        for (int j = 0; j < 16; j++) {
            int_mask[j] = (j * 11 + control3) & 0x1F;
        }
        
        /* Shuffle with float vectors */
        float32x16_t float_shuffle = __builtin_shuffle(float_vec_a[0], float_vec_b[0], int_mask);
        
        /* Convert back to int and store */
        int32x16_t *int_shuffle = (int32x16_t *)&float_shuffle;
        vec_results[15] = *int_shuffle;
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
#ifdef __x86_64__
    /* KERNEL 5: x86-specific builtins with many operands */
    {
        /* Use SSE/AVX specific builtins if available */
        typedef int32_t v8si __attribute__((vector_size(32)));
        v8si xmm0 = {0,1,2,3,4,5,6,7};
        v8si xmm1 = {8,9,10,11,12,13,14,15};
        
        /* Complex inline asm with many operands */
        asm volatile(
            "vmovdqa %1, %%ymm0\n\t"
            "vmovdqa %2, %%ymm1\n\t"
            "vpermq $0x1B, %%ymm0, %%ymm2\n\t"
            "vpermq $0x39, %%ymm1, %%ymm3\n\t"
            "vpblendd $0xF0, %%ymm2, %%ymm3, %%ymm4\n\t"
            "vmovdqa %%ymm4, %0\n\t"
            : "=m"(vec_results[0])
            : "m"(xmm0), "m"(xmm1)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "memory"
        );
    }
#endif
    
#ifdef __ARM_NEON
    /* KERNEL 6: ARM-specific builtins */
    {
        /* Use ARM NEON builtins if available */
        typedef int32_t int32x4_t __attribute__((vector_size(16)));
        int32x4x4_t vec4 = {{
            {0,1,2,3},
            {4,5,6,7},
            {8,9,10,11},
            {12,13,14,15}
        }};
        
        /* Complex inline asm for ARM */
        asm volatile(
            "ld4 {v0.4s-v3.4s}, [%1]\n\t"
            "rev64 v4.4s, v0.4s\n\t"
            "rev64 v5.4s, v1.4s\n\t"
            "trn1 v6.4s, v4.4s, v5.4s\n\t"
            "trn2 v7.4s, v4.4s, v5.4s\n\t"
            "st2 {v6.4s-v7.4s}, [%0]\n\t"
            : "=r"(vec_results)
            : "r"(&vec4)
            : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "memory"
        );
    }
#endif
    
    /* Compute final checksum to prevent dead code elimination */
    for (int i = 0; i < 16; i++) {
        checksum += horizontal_sum(vec_results[i]);
    }
    
    /* Additional complex shuffle with __builtin_shufflevector */
    {
        int32x8_t v1 = {0,1,2,3,4,5,6,7};
        int32x8_t v2 = {8,9,10,11,12,13,14,15};
        
        /* __builtin_shufflevector can require many operands */
        int32x8_t shuffled = __builtin_shufflevector(
            v1, v2,
            0, 8, 1, 9, 2, 10, 3, 11,
            4, 12, 5, 13, 6, 14, 7, 15
        );
        
        /* Use the result */
        checksum += shuffled[0] + shuffled[7];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
