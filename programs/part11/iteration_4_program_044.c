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

/* Barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Function to create non-constant mask vectors */
static int32x16_t create_complex_mask(volatile int seed) {
    int32x16_t mask = {0};
    int32_t *mask_ptr = (int32_t*)&mask;
    
    /* Data-dependent mask generation */
    for (int i = 0; i < 16; i++) {
        mask_ptr[i] = (seed + i * 3) % 32;
        if (mask_ptr[i] < 0) mask_ptr[i] += 32;
    }
    
    return mask;
}

int main(void) {
    /* Initialize large arrays with pseudo-random data */
    int32_t data_a[256];
    int32_t data_b[256];
    int32_t data_c[256];
    float float_data[256];
    
    for (int i = 0; i < 256; i++) {
        data_a[i] = (int32_t)lcg_rand();
        data_b[i] = (int32_t)lcg_rand();
        data_c[i] = (int32_t)lcg_rand();
        float_data[i] = (float)lcg_rand() / 1000.0f;
    }
    
    volatile int control_var = 42; /* Volatile to prevent constant propagation */
    int64_t checksum = 0;
    
    /* KERNEL 1: Complex shuffle with computed mask */
    {
        int32x16_t *vec_a = (int32x16_t*)&data_a[0];
        int32x16_t *vec_b = (int32x16_t*)&data_a[16]; /* Overlap for complexity */
        float32x16_t *vec_f = (float32x16_t*)&float_data[0];
        
        /* Create non-constant mask using volatile control variable */
        int32x16_t mask1 = create_complex_mask(control_var);
        int32x16_t mask2 = create_complex_mask(control_var + 1);
        
        /* Complex shuffle operation that may require many operands */
        int32x16_t result1 = __builtin_shuffle(*vec_a, *vec_b, mask1);
        float32x16_t result_f = __builtin_shuffle(*vec_f, *vec_f, mask2);
        
        /* Store results back */
        int32x16_t *dest = (int32x16_t*)&data_c[0];
        *dest = result1;
        
        /* Mix integer and float operations */
        int32x16_t int_result = __builtin_shuffle(result1, *vec_b, mask1);
        
        /* Accumulate for checksum */
        int32_t *rptr = (int32_t*)&int_result;
        for (int i = 0; i < 16; i++) {
            checksum += rptr[i];
        }
    }
    
    COMPILER_BARRIER();
    
    /* KERNEL 2: Chain of shuffles to increase operand count */
    {
        int32x8_t vec1 = *(int32x8_t*)&data_a[32];
        int32x8_t vec2 = *(int32x8_t*)&data_a[40];
        int32x8_t vec3 = *(int32x8_t*)&data_a[48];
        int32x8_t vec4 = *(int32x8_t*)&data_a[56];
        
        /* Create complex shuffle chain */
        int32x8_t mask_a = {7, 6, 5, 4, 3, 2, 1, 0};
        int32x8_t mask_b = {3, 2, 1, 0, 7, 6, 5, 4};
        
        /* Chain operations where output of one is input to next */
        int32x8_t temp1 = __builtin_shufflevector(vec1, vec2, 
            0, 1, 2, 3, 8, 9, 10, 11);
        int32x8_t temp2 = __builtin_shufflevector(vec3, vec4,
            4, 5, 6, 7, 12, 13, 14, 15);
        int32x8_t temp3 = __builtin_shuffle(temp1, temp2, mask_a);
        int32x8_t temp4 = __builtin_shuffle(temp3, vec1, mask_b);
        
        /* Another shuffle with mixed sources */
        int32x8_t final_result = __builtin_shufflevector(temp2, temp4,
            0, 8, 1, 9, 2, 10, 3, 11);
        
        /* Accumulate checksum */
        int32_t *rptr = (int32_t*)&final_result;
        for (int i = 0; i < 8; i++) {
            checksum += rptr[i];
        }
    }
    
    COMPILER_BARRIER();
    
    /* KERNEL 3: Conditional vector permutations */
    {
        int64x8_t vec_l1 = *(int64x8_t*)&data_a[64];
        int64x8_t vec_l2 = *(int64x8_t*)&data_a[72];
        float64x8_t vec_d1 = *(float64x8_t*)&float_data[64];
        float64x8_t vec_d2 = *(float64x8_t*)&float_data[72];
        
        /* Data-dependent mask selection */
        int64x8_t mask_sel;
        int64_t *msel_ptr = (int64_t*)&mask_sel;
        for (int i = 0; i < 8; i++) {
            msel_ptr[i] = (control_var + i) % 16;
        }
        
        /* Conditional shuffle selection */
        int64x8_t shuffle1 = __builtin_shuffle(vec_l1, vec_l2, mask_sel);
        int64x8_t shuffle2 = __builtin_shuffle(vec_l2, vec_l1, mask_sel);
        
        /* Use conditional operator on vectors */
        int64x8_t cond_mask = {0, -1, 0, -1, 0, -1, 0, -1};
        int64x8_t selected = (shuffle1 & cond_mask) | (shuffle2 & ~cond_mask);
        
        /* Mixed-type operations */
        int64x8_t converted = __builtin_convertvector(*((int32x16_t*)&data_a[80]), int64x8_t);
        int64x8_t final_vec = __builtin_shuffle(selected, converted, mask_sel);
        
        /* Accumulate checksum */
        int64_t *rptr = (int64_t*)&final_vec;
        for (int i = 0; i < 8; i++) {
            checksum += rptr[i];
        }
    }
    
    COMPILER_BARRIER();
    
    /* KERNEL 4: Inline assembly with many vector operands */
    {
        /* Use architecture-specific builtins when available */
        #ifdef __x86_64__
        {
            int32x8_t v1 = *(int32x8_t*)&data_a[96];
            int32x8_t v2 = *(int32x8_t*)&data_a[104];
            int32x8_t v3 = *(int32x8_t*)&data_a[112];
            int32x8_t v4 = *(int32x8_t*)&data_a[120];
            
            /* Complex inline assembly with many operands */
            int32x8_t out1, out2, out3, out4;
            
            asm volatile (
                /* Hypothetical multi-operand vector operation */
                "vmovdqa %[v1], %%ymm0\n\t"
                "vmovdqa %[v2], %%ymm1\n\t"
                "vmovdqa %[v3], %%ymm2\n\t"
                "vmovdqa %[v4], %%ymm3\n\t"
                /* Complex permutation sequence */
                "vpermq $0x1B, %%ymm0, %%ymm4\n\t"
                "vpermq $0x39, %%ymm1, %%ymm5\n\t"
                "vpblendd $0xF0, %%ymm4, %%ymm5, %%ymm6\n\t"
                "vperm2i128 $0x21, %%ymm2, %%ymm3, %%ymm7\n\t"
                : [out1] "=v" (out1), [out2] "=v" (out2),
                  [out3] "=v" (out3), [out4] "=v" (out4)
                : [v1] "v" (v1), [v2] "v" (v2),
                  [v3] "v" (v3), [v4] "v" (v4)
                : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "memory"
            );
            
            /* Use results */
            int32x8_t combined = __builtin_shuffle(out1, out2, 
                (int32x8_t){0, 1, 2, 3, 8, 9, 10, 11});
            
            int32_t *rptr = (int32_t*)&combined;
            for (int i = 0; i < 8; i++) {
                checksum += rptr[i];
            }
        }
        #elif defined(__aarch64__)
        {
            /* ARM NEON version */
            int32x4_t v1 = *(int32x4_t*)&data_a[96];
            int32x4_t v2 = *(int32x4_t*)&data_a[100];
            int32x4_t v3 = *(int32x4_t*)&data_a[104];
            int32x4_t v4 = *(int32x4_t*)&data_a[108];
            
            int32x4_t out1, out2;
            
            asm volatile (
                /* Multi-operand NEON operations */
                "mov v0.16b, %[v1].16b\n\t"
                "mov v1.16b, %[v2].16b\n\t"
                "mov v2.16b, %[v3].16b\n\t"
                "mov v3.16b, %[v4].16b\n\t"
                "rev64 v4.4s, v0.4s\n\t"
                "rev64 v5.4s, v1.4s\n\t"
                "zip1 v6.4s, v4.4s, v5.4s\n\t"
                "zip2 v7.4s, v4.4s, v5.4s\n\t"
                : [out1] "=w" (out1), [out2] "=w" (out2)
                : [v1] "w" (v1), [v2] "w" (v2), [v3] "w" (v3), [v4] "w" (v4)
                : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "memory"
            );
            
            int32_t *rptr = (int32_t*)&out1;
            for (int i = 0; i < 4; i++) {
                checksum += rptr[i];
            }
        }
        #endif
    }
    
    /* Final checksum computation with loop to ensure execution */
    {
        /* Horizontal addition simulation */
        int32x16_t final_vec = *(int32x16_t*)&data_c[0];
        int32_t *fptr = (int32_t*)&final_vec;
        
        for (int i = 0; i < 16; i += 4) {
            checksum += fptr[i] + fptr[i+1] + fptr[i+2] + fptr[i+3];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    /* Additional loop with data-dependent vector operations */
    for (int iter = 0; iter < 10; iter++) {
        int offset = iter * 16;
        int32x16_t v1 = *(int32x16_t*)&data_a[offset];
        int32x16_t v2 = *(int32x16_t*)&data_b[offset];
        
        /* Dynamic mask based on iteration */
        int32x16_t dyn_mask;
        int32_t *dm_ptr = (int32_t*)&dyn_mask;
        for (int i = 0; i < 16; i++) {
            dm_ptr[i] = (control_var + iter + i) % 32;
        }
        
        /* Complex shuffle that cannot be optimized away */
        int32x16_t result = __builtin_shuffle(v1, v2, dyn_mask);
        
        /* Store with barrier */
        COMPILER_BARRIER();
        *(int32x16_t*)&data_c[offset] = result;
        COMPILER_BARRIER();
    }
    
    /* Final print to prevent dead code elimination */
    printf("Final result stored in data_c array\n");
    
    return 0;
}
