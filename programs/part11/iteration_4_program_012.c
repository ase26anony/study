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

/* Initialize array with pseudo-random data */
static void init_array(void *arr, size_t size) {
    uint32_t *ptr = (uint32_t *)arr;
    size_t count = size / sizeof(uint32_t);
    for (size_t i = 0; i < count; i++) {
        ptr[i] = lcg_rand();
    }
}

/* Complex mask computation - prevents constant propagation */
static int32x16_t compute_complex_mask(int32x16_t base, volatile int offset) {
    int32x16_t mask = base;
    /* Use volatile to prevent optimization */
    volatile int shift = offset & 7;
    
    /* Complex, non-constant mask computation */
    for (int i = 0; i < 16; i++) {
        mask[i] = (mask[i] + shift + i) & 0x1F;
    }
    
    return mask;
}

/* Horizontal sum of vector elements */
static int32_t horizontal_sum_i32(int32x16_t v) {
    int32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += v[i];
    }
    return sum;
}

static float horizontal_sum_f32(float32x16_t v) {
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += v[i];
    }
    return sum;
}

int main(void) {
    /* Large arrays for vector operations */
    int32_t data_i32[256];
    float data_f32[256];
    double data_f64[128];
    
    /* Initialize with pseudo-random data */
    init_array(data_i32, sizeof(data_i32));
    init_array(data_f32, sizeof(data_f32));
    init_array(data_f64, sizeof(data_f64));
    
    /* Cast to vector types */
    int32x16_t *vec_i32 = (int32x16_t *)data_i32;
    float32x16_t *vec_f32 = (float32x16_t *)data_f32;
    float64x8_t *vec_f64 = (float64x8_t *)data_f64;
    
    /* Result vectors */
    int32x16_t result_i32[4];
    float32x16_t result_f32[2];
    float64x8_t result_f64[2];
    
    /* Volatile control variables to prevent constant propagation */
    volatile int offset1 = 3;
    volatile int offset2 = 7;
    volatile int offset3 = 11;
    
    /* KERNEL 1: Complex shuffle with computed mask */
    {
        int32x16_t mask_base = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        int32x16_t mask = compute_complex_mask(mask_base, offset1);
        
        /* This shuffle with large vectors and computed mask may require
           many operands during RTL expansion */
        result_i32[0] = __builtin_shuffle(vec_i32[0], vec_i32[1], mask);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 2: Chain of shuffles - output of one feeds into next */
    {
        int32x16_t mask1 = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
        int32x16_t mask2 = {1,3,5,7,9,11,13,15,0,2,4,6,8,10,12,14};
        int32x16_t mask3 = {4,5,6,7,0,1,2,3,12,13,14,15,8,9,10,11};
        
        /* Chain of operations - each may add to operand count */
        int32x16_t temp1 = __builtin_shuffle(vec_i32[2], vec_i32[3], mask1);
        int32x16_t temp2 = __builtin_shuffle(temp1, vec_i32[4], mask2);
        result_i32[1] = __builtin_shuffle(temp2, vec_i32[5], mask3);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 3: Conditional vector permutation */
    {
        float32x16_t mask_f32 = {0,2,4,6,8,10,12,14,1,3,5,7,9,11,13,15};
        float32x16_t alt_mask_f32 = {15,13,11,9,7,5,3,1,14,12,10,8,6,4,2,0};
        
        /* Use volatile to force runtime evaluation */
        volatile int selector = offset2 & 1;
        
        float32x16_t shuffle1 = __builtin_shuffle(vec_f32[0], vec_f32[1], mask_f32);
        float32x16_t shuffle2 = __builtin_shuffle(vec_f32[0], vec_f32[1], alt_mask_f32);
        
        /* Conditional selection between two shuffle results */
        result_f32[0] = selector ? shuffle1 : shuffle2;
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 4: Mixed type operations */
    {
        /* Convert int to float and shuffle */
        float32x16_t converted = __builtin_convertvector(vec_i32[6], float32x16_t);
        
        /* Complex mask with arithmetic */
        int32x16_t dynamic_mask;
        for (int i = 0; i < 16; i++) {
            dynamic_mask[i] = (i + offset3) & 0xF;
        }
        
        /* Shuffle with converted vector */
        result_f32[1] = __builtin_shuffle(converted, vec_f32[2], dynamic_mask);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
#ifdef __x86_64__
    /* KERNEL 5: x86-specific builtins with many operands */
    {
        /* Use x86-specific shuffle builtins if available */
        int32x8_t x86_vec1 = ((int32x8_t *)data_i32)[0];
        int32x8_t x86_vec2 = ((int32x8_t *)data_i32)[1];
        
        /* __builtin_ia32_pshufd typically takes 2 operands but complex usage
           patterns may require more during expansion */
        result_i32[2] = __builtin_shufflevector(x86_vec1, x86_vec2, 
            0,2,4,6,8,10,12,14,1,3,5,7,9,11,13,15);
    }
#endif
    
#ifdef __ARM_NEON
    /* KERNEL 6: ARM-specific builtins */
    {
        /* ARM NEON reversal operations */
        int32x4_t neon_vec1 = ((int32x4_t *)data_i32)[0];
        int32x4_t neon_vec2 = ((int32x4_t *)data_i32)[1];
        int32x4_t neon_vec3 = ((int32x4_t *)data_i32)[2];
        int32x4_t neon_vec4 = ((int32x4_t *)data_i32)[3];
        
        /* Complex pattern combining multiple vectors */
        int32x16_t combined = __builtin_shufflevector(
            neon_vec1, neon_vec2, neon_vec3, neon_vec4,
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
            
        result_i32[3] = combined;
    }
#endif
    
    /* KERNEL 7: Inline assembly with many vector operands */
    {
        /* Hypothetical multi-operand vector operation using inline asm */
        int32x16_t asm_in1 = vec_i32[7];
        int32x16_t asm_in2 = vec_i32[8];
        int32x16_t asm_out;
        
        /* Inline assembly with vector constraints - may require many
           operands during expansion */
        asm volatile (
            /* Hypothetical 3-input vector operation */
            "vpaddd %0, %1, %2\n\t"
            : "=v"(asm_out)
            : "v"(asm_in1), "v"(asm_in2)
            : "memory"
        );
        
        /* Use the result */
        result_i32[0] = result_i32[0] + asm_out;
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 8: Loop-dependent vector operations */
    {
        /* Data-dependent shuffle in a loop */
        int32x16_t accum = {0};
        
        for (int iter = 0; iter < 4; iter++) {
            /* Compute mask based on loop iteration and volatile offset */
            int32x16_t loop_mask;
            for (int i = 0; i < 16; i++) {
                loop_mask[i] = (i + iter + offset1) & 0xF;
            }
            
            /* Shuffle operation that can't be optimized away */
            int32x16_t shuffled = __builtin_shuffle(
                vec_i32[iter], 
                vec_i32[iter + 1], 
                loop_mask
            );
            
            accum = accum + shuffled;
        }
        
        result_i32[1] = result_i32[1] + accum;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int32_t checksum_i32 = 0;
    float checksum_f32 = 0.0f;
    
    for (int i = 0; i < 4; i++) {
        checksum_i32 += horizontal_sum_i32(result_i32[i]);
    }
    
    for (int i = 0; i < 2; i++) {
        checksum_f32 += horizontal_sum_f32(result_f32[i]);
    }
    
    printf("Checksums: int32=%d, float32=%.2f\n", 
           checksum_i32, checksum_f32);
    
    return 0;
}
