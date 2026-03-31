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
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control & 1) ? (i * 3 + 1) % 32 : (i * 5 + 2) % 32;
        if (i % 4 == 0) mask_data[i] ^= control;
    }
    
    return mask;
}

/* Another mask with different pattern */
static int32x16_t compute_alternate_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control & 2) ? (i * 7 + 3) % 32 : (i * 11 + 5) % 32;
        mask_data[i] += (control >> 2) & 0xF;
    }
    
    return mask;
}

int main(void) {
    /* Allocate and initialize data */
    #define ARRAY_SIZE 256
    int32_t data_a[ARRAY_SIZE];
    int32_t data_b[ARRAY_SIZE];
    int32_t data_c[ARRAY_SIZE];
    int32_t results[ARRAY_SIZE];
    
    init_array(data_a, ARRAY_SIZE);
    init_array(data_b, ARRAY_SIZE);
    init_array(data_c, ARRAY_SIZE);
    memset(results, 0, sizeof(results));
    
    volatile int control_var = 42; /* Prevents compile-time optimization */
    uint64_t checksum = 0;
    
    /* Kernel 1: Complex shuffle with computed mask */
    for (int iter = 0; iter < 100; iter++) {
        control_var ^= iter; /* Change control each iteration */
        
        for (int i = 0; i < ARRAY_SIZE - 31; i += 16) {
            /* Load vectors */
            int32x16_t vec_a = *(int32x16_t*)&data_a[i];
            int32x16_t vec_b = *(int32x16_t*)&data_b[i];
            
            /* Compute dynamic mask */
            int32x16_t mask = compute_complex_mask(control_var + i);
            
            /* Complex shuffle operation - may expand to many operands */
            int32x16_t shuffled = __builtin_shuffle(vec_a, vec_b, mask);
            
            /* Store result */
            *(int32x16_t*)&results[i] = shuffled;
            
            /* Compiler barrier */
            asm volatile("" ::: "memory");
        }
    }
    
    /* Kernel 2: Chain of shuffles accumulating operand count */
    for (int iter = 0; iter < 50; iter++) {
        control_var += iter * 3;
        
        for (int i = 0; i < ARRAY_SIZE - 47; i += 16) {
            /* Load multiple vectors */
            int32x16_t v1 = *(int32x16_t*)&data_a[i];
            int32x16_t v2 = *(int32x16_t*)&data_b[i];
            int32x16_t v3 = *(int32x16_t*)&data_c[i];
            
            /* First shuffle */
            int32x16_t mask1 = compute_complex_mask(control_var + i);
            int32x16_t s1 = __builtin_shuffle(v1, v2, mask1);
            
            /* Second shuffle using result of first */
            int32x16_t mask2 = compute_alternate_mask(control_var ^ i);
            int32x16_t s2 = __builtin_shuffle(s1, v3, mask2);
            
            /* Third shuffle chaining more results */
            int32x16_t mask3;
            int32_t *mask3_data = (int32_t*)&mask3;
            for (int j = 0; j < 16; j++) {
                mask3_data[j] = (control_var + j) % 48;
            }
            
            /* This complex chain may require many operands during expansion */
            int32x16_t final_result = __builtin_shuffle(s1, s2, mask3);
            
            /* Mix with another operation */
            final_result = final_result + (s1 >> 1);
            
            *(int32x16_t*)&results[i] = final_result;
            
            asm volatile("" ::: "memory");
        }
    }
    
    /* Kernel 3: Conditional vector permutation */
    for (int iter = 0; iter < 30; iter++) {
        control_var = lcg_rand() & 0xFF;
        
        for (int i = 0; i < ARRAY_SIZE - 31; i += 16) {
            int32x16_t va = *(int32x16_t*)&data_a[i];
            int32x16_t vb = *(int32x16_t*)&data_b[i];
            int32x16_t vc = *(int32x16_t*)&data_c[i];
            
            /* Compute two different masks */
            int32x16_t mask_a = compute_complex_mask(control_var);
            int32x16_t mask_b = compute_alternate_mask(control_var ^ 0x55);
            
            /* Two different shuffle results */
            int32x16_t shuffle_a = __builtin_shuffle(va, vb, mask_a);
            int32x16_t shuffle_b = __builtin_shuffle(vb, vc, mask_b);
            
            /* Conditional selection between shuffle results */
            int32x16_t selector;
            int32_t *sel_data = (int32_t*)&selector;
            for (int j = 0; j < 16; j++) {
                sel_data[j] = (control_var > 128) ? -1 : 0;
            }
            
            /* Conditional move - creates complex operand graph */
            int32x16_t result = (selector != 0) ? shuffle_a : shuffle_b;
            
            /* Additional shuffle on the result */
            int32x16_t final_mask;
            int32_t *fm_data = (int32_t*)&final_mask;
            for (int j = 0; j < 16; j++) {
                fm_data[j] = (j * 13 + control_var) % 32;
            }
            
            int32x16_t final = __builtin_shuffle(result, va, final_mask);
            *(int32x16_t*)&results[i] = final;
            
            asm volatile("" ::: "memory");
        }
    }
    
    /* Kernel 4: Inline assembly with many vector operands */
    #ifdef __x86_64__
    for (int i = 0; i < ARRAY_SIZE - 15; i += 8) {
        int32x8_t v1 = *(int32x8_t*)&data_a[i];
        int32x8_t v2 = *(int32x8_t*)&data_b[i];
        int32x8_t v3 = *(int32x8_t*)&data_c[i];
        int32x8_t v4 = *(int32x8_t*)&data_a[i+8];
        
        int32x8_t out1, out2;
        
        /* Inline asm with multiple vector operands */
        asm volatile (
            "vmovdqa %[v1], %%ymm0\n\t"
            "vmovdqa %[v2], %%ymm1\n\t"
            "vmovdqa %[v3], %%ymm2\n\t"
            "vmovdqa %[v4], %%ymm3\n\t"
            "vpermq $0x1B, %%ymm0, %%ymm4\n\t"
            "vpermq $0x39, %%ymm1, %%ymm5\n\t"
            "vpblendd $0xF0, %%ymm4, %%ymm5, %%ymm6\n\t"
            "vpshufd $0x1B, %%ymm2, %%ymm7\n\t"
            "vpaddd %%ymm6, %%ymm7, %%ymm0\n\t"
            "vmovdqa %%ymm0, %[out1]\n\t"
            "vpsrld $1, %%ymm0, %[out2]\n\t"
            : [out1] "=v" (out1), [out2] "=v" (out2)
            : [v1] "v" (v1), [v2] "v" (v2), [v3] "v" (v3), [v4] "v" (v4)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "memory"
        );
        
        /* Use the results */
        *(int32x8_t*)&results[i] = out1 + out2;
        
        asm volatile("" ::: "memory");
    }
    #endif
    
    /* Kernel 5: Mixed float/int vector operations */
    {
        float32x16_t fvec_a, fvec_b;
        int32x16_t ivec_a, ivec_b;
        
        /* Initialize float vectors */
        float *fdata = (float*)data_a;
        for (int i = 0; i < 64; i++) {
            ((float*)&fvec_a)[i % 16] = (float)fdata[i];
            ((float*)&fvec_b)[i % 16] = (float)fdata[i + 16];
        }
        
        /* Mixed type shuffle pattern */
        int32x16_t int_mask;
        int32_t *imask = (int32_t*)&int_mask;
        for (int i = 0; i < 16; i++) {
            imask[i] = (control_var + i * 7) % 32;
        }
        
        /* This may create complex operand requirements */
        ivec_a = *(int32x16_t*)&fvec_a;
        ivec_b = *(int32x16_t*)&fvec_b;
        
        int32x16_t mixed_shuffle = __builtin_shuffle(ivec_a, ivec_b, int_mask);
        
        /* Convert back and forth */
        float32x16_t float_result = *(float32x16_t*)&mixed_shuffle;
        
        /* Store through integer pointer to avoid strict aliasing */
        memcpy(&results[0], &float_result, sizeof(float_result));
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (uint64_t)results[i];
        if (i % 37 == 0) checksum ^= results[i] * 0x5A5A5A5A;
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
