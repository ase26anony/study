#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Vector type definitions */
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

/* Complex mask computation - prevents constant propagation */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask generation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control + i * 3) % 32;
        if (mask_data[i] >= 16) {
            mask_data[i] = 31 - mask_data[i];
        }
    }
    
    return mask;
}

/* Another mask with different pattern */
static int32x16_t compute_alternate_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control * 7 + i * 5) % 32;
        mask_data[i] = mask_data[i] < 0 ? -mask_data[i] : mask_data[i];
    }
    
    return mask;
}

int main(void) {
    /* Large arrays for vector operations */
    int32_t array_a[256];
    int32_t array_b[256];
    int32_t array_c[256];
    float array_f[256];
    int64_t array_l[128];
    
    /* Initialize arrays */
    init_array(array_a, 256);
    init_array(array_b, 256);
    init_array(array_c, 256);
    
    for (int i = 0; i < 256; i++) {
        array_f[i] = (float)array_a[i] / 1000.0f;
    }
    for (int i = 0; i < 128; i++) {
        array_l[i] = (int64_t)array_a[i] * array_b[i];
    }
    
    /* Volatile control variables to prevent optimization */
    volatile int control1 = 7;
    volatile int control2 = 13;
    volatile int control3 = 42;
    
    /* Result storage */
    int32x16_t results[4];
    float32x8_t float_results[2];
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /*** KERNEL 1: Complex shuffle with computed mask ***/
    {
        /* Load vectors from different arrays */
        int32x16_t *vec_a = (int32x16_t*)&array_a[0];
        int32x16_t *vec_b = (int32x16_t*)&array_a[64];  /* Offset to different data */
        int32x16_t *vec_c = (int32x16_t*)&array_b[0];
        
        /* Compute dynamic mask */
        int32x16_t mask = compute_complex_mask(control1);
        
        /* Complex shuffle operation that may require many operands during expansion */
        int32x16_t temp1 = __builtin_shuffle(*vec_a, *vec_b, mask);
        
        /* Another shuffle with different mask */
        int32x16_t mask2 = compute_alternate_mask(control2);
        int32x16_t temp2 = __builtin_shuffle(*vec_c, temp1, mask2);
        
        /* Chain operations to increase operand count */
        int32x16_t mask3 = compute_complex_mask(control3);
        results[0] = __builtin_shuffle(temp1, temp2, mask3);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /*** KERNEL 2: Chain of shuffles with mixed types ***/
    {
        /* Mixed type vectors */
        float32x8_t *fvec_a = (float32x8_t*)&array_f[0];
        float32x8_t *fvec_b = (float32x8_t*)&array_f[64];
        int32x8_t *ivec_a = (int32x8_t*)&array_a[128];
        
        /* Convert and shuffle */
        int32x8_t iconv = __builtin_convertvector(*fvec_a, int32x8_t);
        
        /* Complex shuffle chain - each step adds more operands */
        int32x8_t shuffle_mask = {7, 6, 5, 4, 3, 2, 1, 0};
        int32x8_t temp1 = __builtin_shuffle(iconv, *ivec_a, shuffle_mask);
        
        /* Another shuffle with computed indices */
        int32x8_t mask2;
        int32_t *mask2_data = (int32_t*)&mask2;
        for (int i = 0; i < 8; i++) {
            mask2_data[i] = (control1 + i * control2) % 16;
        }
        
        /* This may trigger the 10-11 operand path when expanded */
        int32x8_t temp2 = __builtin_shufflevector(temp1, *ivec_a, 
            0, 8, 2, 10, 4, 12, 6, 14, 1, 9, 3, 11, 5, 13, 7, 15);
        
        /* Store result */
        memcpy(&results[1], &temp2, sizeof(temp2));
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /*** KERNEL 3: Conditional vector permutation ***/
    {
        int32x16_t *vec1 = (int32x16_t*)&array_a[32];
        int32x16_t *vec2 = (int32x16_t*)&array_b[32];
        int32x16_t *vec3 = (int32x16_t*)&array_c[32];
        
        /* Compute masks based on control variables */
        int32x16_t mask_a = compute_complex_mask(control1);
        int32x16_t mask_b = compute_alternate_mask(control2);
        
        /* Two different shuffle results */
        int32x16_t shuffle_a = __builtin_shuffle(*vec1, *vec2, mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(*vec2, *vec3, mask_b);
        
        /* Conditional selection between shuffle results */
        int32x16_t selector;
        int32_t *sel_data = (int32_t*)&selector;
        for (int i = 0; i < 16; i++) {
            sel_data[i] = (array_a[i] > array_b[i]) ? -1 : 0;
        }
        
        /* Conditional permutation - complex operation */
        results[2] = (selector != 0) ? shuffle_a : shuffle_b;
        
        /* Additional shuffle to chain operations */
        int32x16_t final_mask;
        int32_t *fmask_data = (int32_t*)&final_mask;
        for (int i = 0; i < 16; i++) {
            fmask_data[i] = (i * 5 + control3) % 32;
        }
        
        results[2] = __builtin_shuffle(results[2], *vec3, final_mask);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /*** KERNEL 4: Inline assembly with many vector operands ***/
    {
        /* Try to use inline assembly that may get expanded through optabs */
        int32x8_t asm_vec1, asm_vec2, asm_vec3, asm_vec4;
        int32x8_t *src1 = (int32x8_t*)&array_a[0];
        int32x8_t *src2 = (int32x8_t*)&array_a[32];
        int32x8_t *src3 = (int32x8_t*)&array_b[0];
        int32x8_t *src4 = (int32x8_t*)&array_b[32];
        
        /* Complex inline asm with many operands */
        asm volatile (
            /* Multiple vector operations in one asm block */
            "vmovdqa %[v1], %[out1]\n\t"
            "vmovdqa %[v2], %[out2]\n\t"
            "vpaddd %[v3], %[out1], %[out3]\n\t"
            "vpaddd %[v4], %[out2], %[out4]\n\t"
            : [out1] "=x" (asm_vec1),
              [out2] "=x" (asm_vec2),
              [out3] "=x" (asm_vec3),
              [out4] "=x" (asm_vec4)
            : [v1] "x" (*src1),
              [v2] "x" (*src2),
              [v3] "x" (*src3),
              [v4] "x" (*src4)
            : "memory"
        );
        
        /* Use the results to prevent optimization */
        memcpy(&results[3], &asm_vec1, sizeof(asm_vec1));
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
#ifdef __x86_64__
    /*** Target-specific builtins for x86 ***/
    {
        int32x8_t xmm_vec = *(int32x8_t*)&array_a[0];
        
        /* Use x86-specific shuffle intrinsic */
        int32x8_t shuffled = __builtin_ia32_pshufd(xmm_vec, 0x1B);
        
        /* Chain with generic shuffle */
        int32x8_t mask = {3, 2, 1, 0, 7, 6, 5, 4};
        int32x8_t final = __builtin_shuffle(shuffled, shuffled, mask);
        
        /* Mix with other operations */
        results[1] = __builtin_shufflevector(results[1], final, 
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    }
#endif
    
#ifdef __ARM_NEON
    /*** Target-specific builtins for ARM ***/
    {
        int32x4_t neon_vec = *(int32x4_t*)&array_a[0];
        
        /* Use ARM-specific reversal intrinsic */
        int32x4_t reversed = __builtin_neon_vrev64q_s32(neon_vec);
        
        /* Combine with generic operations */
        int32x8_t combined = __builtin_shufflevector(
            *(int32x8_t*)&reversed, 
            *(int32x8_t*)&array_b[0],
            0, 1, 2, 3, 4, 5, 6, 7);
    }
#endif
    
    /*** Loop-dependent vector operations ***/
    {
        int32x16_t accum = {0};
        
        /* Loop with data-dependent shuffles */
        for (int iter = 0; iter < 8; iter++) {
            volatile int dynamic_control = array_a[iter] & 0xF;
            
            int32x16_t *vec_a = (int32x16_t*)&array_a[iter * 16];
            int32x16_t *vec_b = (int32x16_t*)&array_b[iter * 16];
            
            /* Compute mask based on loop iteration and data */
            int32x16_t loop_mask = compute_complex_mask(dynamic_control + iter);
            
            /* Data-dependent shuffle in loop */
            int32x16_t shuffled = __builtin_shuffle(*vec_a, *vec_b, loop_mask);
            
            /* Accumulate results */
            accum += shuffled;
        }
        
        /* Mix with previous results */
        results[0] = __builtin_shuffle(results[0], accum, 
            compute_complex_mask(control1));
    }
    
    /*** Final checksum computation ***/
    int64_t checksum = 0;
    
    /* Horizontal addition of all result vectors */
    for (int i = 0; i < 4; i++) {
        int32_t *data = (int32_t*)&results[i];
        for (int j = 0; j < 16; j++) {
            checksum += data[j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
