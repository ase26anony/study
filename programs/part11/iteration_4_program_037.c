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

/* Initialize array with pseudo-random data */
static void init_array(int32_t *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr[i] = (int32_t)lcg_rand();
    }
}

/* Complex mask computation - prevents constant propagation */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_ptr = (int32_t*)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_ptr[i] = (control & (1 << (i % 8))) ? 
                     (i + control) % 16 : 
                     (16 - i + control) % 16;
    }
    
    return mask;
}

/* Another mask with different pattern */
static int32x16_t compute_alternate_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_ptr = (int32_t*)&mask;
    
    for (int i = 0; i < 16; i++) {
        mask_ptr[i] = (control * i) % 32;
        if (mask_ptr[i] >= 16) mask_ptr[i] = 31 - mask_ptr[i];
    }
    
    return mask;
}

int main(void) {
    /* Large arrays for vector operations */
    int32_t data_a[256];
    int32_t data_b[256];
    int32_t data_c[256];
    float float_data[256];
    
    /* Initialize with pseudo-random data */
    init_array(data_a, 256);
    init_array(data_b, 256);
    init_array(data_c, 256);
    
    for (int i = 0; i < 256; i++) {
        float_data[i] = (float)lcg_rand() / 1000.0f;
    }
    
    /* Volatile control variables to prevent optimization */
    volatile int control_var = 42;
    volatile int loop_counter = 0;
    
    /* Result storage */
    int32x16_t results[4] = {0};
    float32x16_t float_results[2] = {0};
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * KERNEL 1: Complex shuffle with computed mask
     * This should generate many operands during RTL expansion
     ******************************************************************/
    for (int iter = 0; iter < 100; iter++) {
        loop_counter = iter;
        
        /* Load vectors from different array positions */
        int32x16_t *vec_a = (int32x16_t*)&data_a[iter * 4];
        int32x16_t *vec_b = (int32x16_t*)&data_b[iter * 4];
        int32x16_t *vec_c = (int32x16_t*)&data_c[iter * 4];
        
        /* Compute data-dependent mask */
        int32x16_t mask1 = compute_complex_mask(control_var + iter);
        int32x16_t mask2 = compute_alternate_mask(control_var - iter);
        
        /* Complex shuffle chain - forces many operands */
        int32x16_t temp1 = __builtin_shuffle(*vec_a, *vec_b, mask1);
        int32x16_t temp2 = __builtin_shuffle(*vec_b, *vec_c, mask2);
        
        /* Another shuffle combining results */
        int32x16_t combined_mask;
        int32_t *cm_ptr = (int32_t*)&combined_mask;
        for (int i = 0; i < 16; i++) {
            cm_ptr[i] = (iter + i * 3) % 32;
        }
        
        /* This shuffle with 3 vectors and computed mask may require many operands */
        int32x16_t temp3 = __builtin_shuffle(temp1, temp2, combined_mask);
        
        /* Store result */
        results[iter % 4] = temp3;
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /******************************************************************
     * KERNEL 2: Chain of shuffles with mixed types
     * Increases operand count through chaining
     ******************************************************************/
    for (int iter = 0; iter < 50; iter++) {
        /* Mix float and int vectors */
        float32x16_t *fvec_a = (float32x16_t*)&float_data[iter * 8];
        float32x16_t *fvec_b = (float32x16_t*)&float_data[iter * 8 + 16];
        
        /* Convert to int for shuffle, then back */
        int32x16_t ivec_a = *(int32x16_t*)fvec_a;
        int32x16_t ivec_b = *(int32x16_t*)fvec_b;
        
        /* Create complex shuffle pattern */
        int32x16_t shuffle_pattern;
        int32_t *sp_ptr = (int32_t*)&shuffle_pattern;
        for (int i = 0; i < 16; i++) {
            sp_ptr[i] = (control_var * i + iter) % 32;
        }
        
        /* Chain of operations */
        int32x16_t shuffled1 = __builtin_shuffle(ivec_a, ivec_b, shuffle_pattern);
        
        /* Modify pattern and shuffle again */
        for (int i = 0; i < 16; i++) {
            sp_ptr[i] = (sp_ptr[i] + 7) % 32;
        }
        
        int32x16_t shuffled2 = __builtin_shuffle(shuffled1, ivec_a, shuffle_pattern);
        
        /* Another shuffle with the result */
        for (int i = 0; i < 16; i++) {
            sp_ptr[i] = (sp_ptr[i] * 2) % 32;
        }
        
        int32x16_t shuffled3 = __builtin_shuffle(shuffled2, ivec_b, shuffle_pattern);
        
        /* Convert back to float */
        float_results[iter % 2] = *(float32x16_t*)&shuffled3;
        
        asm volatile("" ::: "memory");
    }
    
    /******************************************************************
     * KERNEL 3: Conditional vector permutations
     * Uses ?: operator on vectors with shuffle results
     ******************************************************************/
    {
        int32x16_t vec1 = *(int32x16_t*)&data_a[0];
        int32x16_t vec2 = *(int32x16_t*)&data_b[0];
        int32x16_t vec3 = *(int32x16_t*)&data_c[0];
        
        /* Two different mask computations */
        int32x16_t mask_a = compute_complex_mask(control_var);
        int32x16_t mask_b = compute_alternate_mask(control_var + 1);
        
        /* Compute two different shuffle results */
        int32x16_t shuffle_a = __builtin_shuffle(vec1, vec2, mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(vec2, vec3, mask_b);
        
        /* Create selection mask based on data */
        int32x16_t select_mask;
        int32_t *sel_ptr = (int32_t*)&select_mask;
        for (int i = 0; i < 16; i++) {
            int32_t val = ((int32_t*)&vec1)[i];
            sel_ptr[i] = (val & 1) ? -1 : 0;
        }
        
        /* Conditional selection between shuffle results */
        int32x16_t result = (select_mask != 0) ? shuffle_a : shuffle_b;
        
        /* Use result */
        results[0] = result;
    }
    
    /******************************************************************
     * KERNEL 4: Inline assembly with many vector operands
     * Directly specifies many input/output operands
     ******************************************************************/
    {
        int32x16_t asm_in1 = *(int32x16_t*)&data_a[32];
        int32x16_t asm_in2 = *(int32x16_t*)&data_b[32];
        int32x16_t asm_in3 = *(int32x16_t*)&data_c[32];
        int32x16_t asm_out1, asm_out2;
        
        /* Inline asm with many vector operands */
        asm volatile(
            /* Hypothetical multi-operand vector operation */
            "/* Multi-vector operation placeholder %0 %1 %2 %3 %4 */\n"
            : "=v"(asm_out1), "=v"(asm_out2)
            : "v"(asm_in1), "v"(asm_in2), "v"(asm_in3)
            : "memory"
        );
        
        results[1] = asm_out1;
        results[2] = asm_out2;
    }
    
#ifdef __x86_64__
    /******************************************************************
     * Target-specific builtins for x86
     ******************************************************************/
    {
        int32x8_t x86_vec1 = *(int32x8_t*)&data_a[64];
        int32x8_t x86_vec2 = *(int32x8_t*)&data_b[64];
        
        /* Use x86-specific shuffle intrinsic */
        int32x8_t shuffled = __builtin_ia32_pshufd(x86_vec1, 0x1B);
        
        /* Chain with another operation */
        int32x8_t result = shuffled + x86_vec2;
        
        /* Store partial result */
        memcpy(&results[3], &result, sizeof(result));
    }
#endif
    
#ifdef __ARM_NEON
    /******************************************************************
     * Target-specific builtins for ARM
     ******************************************************************/
    {
        int32x4_t neon_vec1 = *(int32x4_t*)&data_a[128];
        int32x4_t neon_vec2 = *(int32x4_t*)&data_b[128];
        
        /* Use ARM-specific reversal intrinsic */
        int32x4_t reversed = __builtin_neon_vrev64q_s32(neon_vec1);
        
        /* Combine with other vector */
        int32x4_t result = reversed + neon_vec2;
        
        /* Store partial result */
        memcpy(&results[3], &result, sizeof(result));
    }
#endif
    
    /******************************************************************
     * Final checksum computation to prevent dead code elimination
     ******************************************************************/
    int64_t checksum = 0;
    
    /* Horizontal addition of all result vectors */
    for (int i = 0; i < 4; i++) {
        int32_t *rptr = (int32_t*)&results[i];
        for (int j = 0; j < 16; j++) {
            checksum += rptr[j];
        }
    }
    
    for (int i = 0; i < 2; i++) {
        float *fptr = (float*)&float_results[i];
        for (int j = 0; j < 16; j++) {
            checksum += (int64_t)fptr[j];
        }
    }
    
    printf("Checksum: %ld\n", (long)checksum);
    
    return 0;
}
