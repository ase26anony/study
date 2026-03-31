#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Vector type definitions */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));
typedef int64_t int64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(int32_t *arr32, size_t n32, 
                       float *arrf32, size_t nf32,
                       double *arrf64, size_t nf64) {
    for (size_t i = 0; i < n32; i++) {
        arr32[i] = (int32_t)lcg_rand();
    }
    for (size_t i = 0; i < nf32; i++) {
        arrf32[i] = (float)lcg_rand() / 4294967296.0f;
    }
    for (size_t i = 0; i < nf64; i++) {
        arrf64[i] = (double)lcg_rand() / 4294967296.0;
    }
}

/* Complex mask computation to prevent constant propagation */
static int32x16_t compute_dynamic_mask(volatile int *control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask computation */
    int base = *control;
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (base + i * 3) % 32;
        if (mask_data[i] >= 16) {
            mask_data[i] = (mask_data[i] % 8) + 16;
        }
    }
    
    return mask;
}

int main(void) {
    /* Large arrays for vector operations */
    int32_t data32[256];
    float dataf32[256];
    double dataf64[128];
    
    /* Control variables to prevent optimization */
    volatile int control1 = 7;
    volatile int control2 = 13;
    volatile int control3 = 42;
    
    /* Initialize with pseudo-random data */
    init_arrays(data32, 256, dataf32, 256, dataf64, 128);
    
    /* Cast to vector types */
    int32x16_t *vec32 = (int32x16_t *)data32;
    float32x16_t *vecf32 = (float32x16_t *)dataf32;
    float64x8_t *vecf64 = (float64x8_t *)dataf64;
    
    /* Result vectors */
    int32x16_t result1, result2, result3;
    float32x16_t fresult1, fresult2;
    float64x8_t dresult1;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * KERNEL 1: Complex shuffle with computed mask (10+ operands)
     ******************************************************************/
    {
        /* Compute dynamic mask - prevents constant propagation */
        int32x16_t mask = compute_dynamic_mask(&control1);
        
        /* Complex shuffle operation that may require many operands */
        result1 = __builtin_shuffle(vec32[0], vec32[1], mask);
        
        /* Chain another shuffle with modified mask */
        int32x16_t mask2 = mask + (int32x16_t){1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
        result2 = __builtin_shuffle(vec32[2], result1, mask2);
        
        /* Mixed-type operation */
        int32x16_t temp_mask = mask & (int32x16_t){15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
        result3 = __builtin_shuffle(result1, result2, temp_mask);
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * KERNEL 2: Chain of shuffles accumulating operand count
     ******************************************************************/
    {
        /* Initial vectors */
        int32x16_t a = vec32[3];
        int32x16_t b = vec32[4];
        int32x16_t c = vec32[5];
        
        /* Sequence of shuffles - each may add to operand count */
        int32x16_t m1 = compute_dynamic_mask(&control2);
        int32x16_t t1 = __builtin_shuffle(a, b, m1);
        
        int32x16_t m2 = m1 ^ (int32x16_t){5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5};
        int32x16_t t2 = __builtin_shuffle(t1, c, m2);
        
        int32x16_t m3 = m1 + m2;
        int32x16_t t3 = __builtin_shuffle(t2, a, m3);
        
        /* Final shuffle combining all previous results */
        int32x16_t m4 = compute_dynamic_mask(&control3);
        result1 = __builtin_shuffle(t1, t3, m4);
        
        /* Store back to force RTL generation */
        vec32[6] = result1;
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * KERNEL 3: Conditional vector permutations
     ******************************************************************/
    {
        float32x16_t fa = vecf32[0];
        float32x16_t fb = vecf32[1];
        float32x16_t fc = vecf32[2];
        
        /* Create mask based on control variable */
        int32x16_t mask_cond;
        int32_t *mask_data = (int32_t*)&mask_cond;
        for (int i = 0; i < 16; i++) {
            mask_data[i] = (control1 + i) % 24;
        }
        
        /* Conditional shuffle selection */
        float32x16_t shuffle1 = __builtin_shuffle(fa, fb, mask_cond);
        float32x16_t shuffle2 = __builtin_shuffle(fb, fc, mask_cond + 
            (int32x16_t){8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8});
        
        /* Conditional selection between two shuffle results */
        float32x16_t cmp_mask = fa > fb;
        fresult1 = cmp_mask ? shuffle1 : shuffle2;
        
        /* Another conditional with different types */
        int32x16_t int_mask = (int32x16_t)(fa * 100.0f) % 16;
        fresult2 = __builtin_shuffle(fresult1, fc, int_mask);
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * KERNEL 4: Inline assembly with many vector operands
     ******************************************************************/
    {
        /* Use inline assembly with multiple vector constraints */
        int32x16_t asm_in1 = vec32[7];
        int32x16_t asm_in2 = vec32[8];
        int32x16_t asm_in3 = vec32[9];
        int32x16_t asm_out;
        
        /* Hypothetical multi-operand vector operation */
        asm volatile (
            /* Assembly template with many operands */
            "vmovdqa %1, %%ymm0\n\t"
            "vmovdqa %2, %%ymm1\n\t"
            "vmovdqa %3, %%ymm2\n\t"
            /* Complex operation that might require many operands */
            "vpshufd $0x1B, %%ymm0, %%ymm3\n\t"
            "vpshufd $0x39, %%ymm1, %%ymm4\n\t"
            "vpblendd $0xF0, %%ymm3, %%ymm4, %%ymm5\n\t"
            "vmovdqa %%ymm5, %0\n\t"
            : "=v"(asm_out)
            : "v"(asm_in1), "v"(asm_in2), "v"(asm_in3)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "memory"
        );
        
        vec32[10] = asm_out;
    }
    
    /******************************************************************
     * KERNEL 5: Architecture-specific builtins (x86)
     ******************************************************************/
#ifdef __x86_64__
    {
        int32x8_t sse_vec1 = ((int32x8_t *)data32)[0];
        int32x8_t sse_vec2 = ((int32x8_t *)data32)[1];
        
        /* Use x86-specific shuffle intrinsic */
        int32x8_t shuffled = __builtin_ia32_pshufd(sse_vec1, 0x1B);
        
        /* Chain with generic shuffle */
        int32x8_t mask = {7,6,5,4,3,2,1,0};
        int32x8_t final = __builtin_shuffle(shuffled, sse_vec2, mask);
        
        ((int32x8_t *)data32)[2] = final;
    }
#endif
    
    /******************************************************************
     * KERNEL 6: Loop-dependent vector operations
     ******************************************************************/
    {
        int32x16_t accum = {0};
        volatile int loop_control = control1;
        
        /* Loop with data-dependent shuffles */
        for (int i = 0; i < 8; i++) {
            /* Compute mask based on loop iteration and control */
            int32x16_t loop_mask;
            int32_t *mask_ptr = (int32_t*)&loop_mask;
            for (int j = 0; j < 16; j++) {
                mask_ptr[j] = (loop_control + i * 2 + j) % 32;
            }
            
            /* Shuffle with dynamic mask */
            int32x16_t shuffled = __builtin_shuffle(vec32[i], vec32[i+1], loop_mask);
            accum = accum + shuffled;
            
            /* Modify control for next iteration */
            loop_control = (loop_control * 3 + 1) & 0xFF;
        }
        
        vec32[15] = accum;
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * Final checksum computation to prevent dead code elimination
     ******************************************************************/
    {
        int64_t checksum = 0;
        
        /* Horizontal addition of result vectors */
        int32_t *r1 = (int32_t*)&result1;
        int32_t *r2 = (int32_t*)&result2;
        int32_t *r3 = (int32_t*)&result3;
        
        for (int i = 0; i < 16; i++) {
            checksum += r1[i] + r2[i] + r3[i];
        }
        
        /* Add float results */
        float *fr1 = (float*)&fresult1;
        float *fr2 = (float*)&fresult2;
        for (int i = 0; i < 16; i++) {
            checksum += (int64_t)(fr1[i] * 1000.0f);
            checksum += (int64_t)(fr2[i] * 1000.0f);
        }
        
        printf("Checksum: %ld\n", checksum);
    }
    
    return 0;
}
