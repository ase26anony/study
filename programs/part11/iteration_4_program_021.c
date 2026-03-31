#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef int64_t int64x8_t __attribute__((vector_size(64)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(int32_t *arr32, size_t n32, 
                       int64_t *arr64, size_t n64,
                       float *arrf32, size_t nf32,
                       double *arrf64, size_t nf64) {
    for (size_t i = 0; i < n32; i++) arr32[i] = (int32_t)lcg_rand();
    for (size_t i = 0; i < n64; i++) arr64[i] = (int64_t)lcg_rand() << 32 | lcg_rand();
    for (size_t i = 0; i < nf32; i++) arrf32[i] = (float)(lcg_rand() % 1000) / 100.0f;
    for (size_t i = 0; i < nf64; i++) arrf64[i] = (double)(lcg_rand() % 1000) / 100.0;
}

/* Complex mask computation - prevents constant propagation */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask generation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control + i * 3) % 32;
        if (i % 4 == 0) mask_data[i] ^= control;
        if (i % 3 == 0) mask_data[i] += (control >> 2);
    }
    
    /* Additional non-linear transformation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (mask_data[i] * 1103515245u + 12345) % 32;
    }
    
    return mask;
}

/* Another mask with different pattern */
static int32x16_t compute_alternate_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control * i + 7) % 32;
        mask_data[i] ^= (i << 3);
        mask_data[i] = (mask_data[i] & 31) | ((i & 1) ? 0x80000000 : 0);
    }
    
    return mask;
}

int main(void) {
    /* Large arrays to hold vector data */
    int32_t arr32[256];
    int64_t arr64[128];
    float arrf32[256];
    double arrf64[128];
    
    /* Initialize with pseudo-random data */
    init_arrays(arr32, 256, arr64, 128, arrf32, 256, arrf64, 128);
    
    /* Cast to vector types */
    int32x16_t *vec32 = (int32x16_t*)arr32;
    int64x8_t *vec64 = (int64x8_t*)arr64;
    float32x16_t *vecf32 = (float32x16_t*)arrf32;
    float64x8_t *vecf64 = (float64x8_t*)arrf64;
    
    /* Result vectors */
    int32x16_t result1, result2, result3;
    float32x16_t fresult1, fresult2;
    int64x8_t result64;
    
    /* Volatile control variables to prevent optimization */
    volatile int control1 = 7;
    volatile int control2 = 13;
    volatile int control3 = 42;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /*** KERNEL 1: Complex shuffle with computed mask ***/
    {
        /* Compute dynamic mask */
        int32x16_t mask1 = compute_complex_mask(control1);
        
        /* Complex shuffle operation - may require many operands during expansion */
        result1 = __builtin_shuffle(vec32[0], vec32[1], mask1);
        
        /* Additional shuffle with different sources */
        int32x16_t mask2 = compute_alternate_mask(control2);
        result2 = __builtin_shuffle(vec32[2], vec32[3], mask2);
        
        /* Chain shuffles */
        int32x16_t temp = __builtin_shuffle(result1, result2, mask1);
        result3 = __builtin_shuffle(temp, vec32[4], mask2);
    }
    
    asm volatile("" ::: "memory");
    
    /*** KERNEL 2: Mixed-type permutations and chained operations ***/
    {
        /* Create mask from float data (type conversion adds complexity) */
        int32x16_t int_mask;
        int32_t *int_mask_data = (int32_t*)&int_mask;
        float *fdata = (float*)&vecf32[0];
        
        for (int i = 0; i < 16; i++) {
            int_mask_data[i] = (int)(fdata[i] * 100.0f) % 32;
            int_mask_data[i] ^= control3;
        }
        
        /* Shuffle with mixed sources */
        fresult1 = __builtin_shuffle(vecf32[1], vecf32[2], 
                                    *(float32x16_t*)&int_mask);
        
        /* Another shuffle chain */
        float32x16_t ftemp = __builtin_shuffle(fresult1, vecf32[3], 
                                              *(float32x16_t*)&int_mask);
        
        /* Conditional permutation */
        if (control1 > 5) {
            fresult2 = __builtin_shuffle(ftemp, vecf32[4], 
                                        *(float32x16_t*)&int_mask);
        } else {
            fresult2 = __builtin_shuffle(vecf32[4], ftemp, 
                                        *(float32x16_t*)&int_mask);
        }
    }
    
    asm volatile("" ::: "memory");
    
    /*** KERNEL 3: Loop-dependent vector operations ***/
    {
        int32x16_t accum = {0};
        int32x16_t mask_accum = {0};
        
        /* Loop with data-dependent masks */
        for (volatile int iter = 0; iter < 4; iter++) {
            /* Compute mask based on iteration */
            int32x16_t loop_mask = compute_complex_mask(control1 + iter);
            
            /* Shuffle operation inside loop - prevents simplification */
            int32x16_t shuffled = __builtin_shuffle(
                vec32[iter % 8], 
                vec32[(iter + 1) % 8], 
                loop_mask
            );
            
            /* Accumulate results */
            accum += shuffled;
            mask_accum += loop_mask;
            
            /* Use result in next iteration */
            if (iter < 3) {
                vec32[(iter + 2) % 8] = __builtin_shuffle(
                    accum, 
                    vec32[(iter + 2) % 8], 
                    mask_accum
                );
            }
        }
        result1 = accum;
    }
    
    asm volatile("" ::: "memory");
    
    /*** KERNEL 4: Inline assembly with many operands ***/
    {
        /* Inline asm with vector constraints - may require many operands */
        asm volatile (
            /* Hypothetical multi-operand vector operation */
            "# Complex vector operation requiring many operands\n\t"
            : "=v" (result64), "=v" (result2)
            : "v" (vec64[0]), "v" (vec64[1]), "v" (vec64[2]), 
              "v" (vec64[3]), "v" (vec32[0]), "v" (vec32[1]),
              "r" (control1), "r" (control2)
            : "memory"
        );
    }
    
#ifdef __x86_64__
    /*** Target-specific builtins for x86 ***/
    {
        /* Use x86-specific shuffle intrinsics */
        asm volatile(
            "# x86-specific vector operations\n\t"
            : 
            : 
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "memory"
        );
    }
#elif defined(__aarch64__)
    /*** Target-specific builtins for ARM ***/
    {
        /* ARM-specific operations */
        asm volatile(
            "# ARM-specific vector operations\n\t"
            : 
            : 
            : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
              "v16", "v17", "v18", "v19", "memory"
        );
    }
#endif
    
    asm volatile("" ::: "memory");
    
    /*** Final checksum computation to prevent dead code elimination ***/
    {
        int64_t checksum = 0;
        
        /* Horizontal addition of result vectors */
        int32_t *r1 = (int32_t*)&result1;
        int32_t *r2 = (int32_t*)&result2;
        int32_t *r3 = (int32_t*)&result3;
        
        for (int i = 0; i < 16; i++) {
            checksum += r1[i];
            checksum += r2[i];
            checksum += r3[i];
        }
        
        float *fr1 = (float*)&fresult1;
        float *fr2 = (float*)&fresult2;
        for (int i = 0; i < 16; i++) {
            checksum += (int64_t)(fr1[i] * 1000.0f);
            checksum += (int64_t)(fr2[i] * 1000.0f);
        }
        
        int64_t *r64 = (int64_t*)&result64;
        for (int i = 0; i < 8; i++) {
            checksum += r64[i];
        }
        
        printf("Checksum: %ld\n", (long)checksum);
    }
    
    return 0;
}
