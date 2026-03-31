#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int global_seed;
int global_int_array[512] __attribute__((aligned(64)));
float global_float_array[512] __attribute__((aligned(64)));
int accumulator[512] __attribute__((aligned(64)));

/* Vector type definitions */
typedef int v4si __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef int v16si __attribute__((vector_size(64)));
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef double v8df __attribute__((vector_size(64)));

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    unsigned int lcg = seed;
    for (int i = 0; i < 512; i++) {
        lcg = lcg * 1103515245 + 12345;
        global_int_array[i] = (int)(lcg >> 16) & 0x7FFF;
        global_float_array[i] = (float)((lcg >> 16) & 0xFF) * 0.1f;
        accumulator[i] = 0;
    }
}

/* Function using __builtin_shuffle with many operands */
#ifdef __AVX2__
void shuffle_10_operand_int(v16si *result, const int *data, volatile int *mask_indices) {
    /* Load 4 vectors (64 bytes each = 256 bytes total) */
    v16si v0 = *(const v16si*)(data + 0);
    v16si v1 = *(const v16si*)(data + 16);
    v16si v2 = *(const v16si*)(data + 32);
    v16si v3 = *(const v16si*)(data + 48);
    
    /* Create control mask from volatile indices to prevent constant folding */
    v16si control_mask;
    for (int i = 0; i < 16; i++) {
        control_mask[i] = mask_indices[i % 4] + i;
    }
    
    /* Complex shuffle pattern that might require 10+ operand expansion */
    v16si shuffled;
    
    /* Nested control flow to stress expander */
    if (mask_indices[0] & 1) {
        /* First shuffle pattern */
        shuffled = __builtin_shuffle(v0, v1, v2, v3, control_mask);
    } else {
        /* Alternative shuffle pattern */
        v16si temp = __builtin_shuffle(v0, v1, control_mask);
        shuffled = __builtin_shuffle(temp, v2, v3, control_mask);
    }
    
    /* Additional operation on result */
    *result = shuffled + (v16si){1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
}

void shuffle_11_operand_float(v16sf *result, const float *data, volatile int *mask_indices) {
    /* Load vectors */
    v16sf f0 = *(const v16sf*)(data + 0);
    v16sf f1 = *(const v16sf*)(data + 16);
    v16sf f2 = *(const v16sf*)(data + 32);
    v16sf f3 = *(const v16sf*)(data + 48);
    
    /* Create control mask */
    v16si int_mask;
    for (int i = 0; i < 16; i++) {
        int_mask[i] = (mask_indices[i % 4] * i) & 0xF;
    }
    
    /* Switch statement with different shuffle patterns */
    switch (mask_indices[0] & 3) {
        case 0: {
            /* Pattern requiring many operands */
            v16sf temp = __builtin_shuffle(f0, f1, f2, int_mask);
            *result = __builtin_shuffle(temp, f3, int_mask);
            break;
        }
        case 1: {
            /* Direct 11-operand-like pattern */
            v16sf temp0 = __builtin_shuffle(f0, f1, int_mask);
            v16sf temp1 = __builtin_shuffle(f2, f3, int_mask);
            *result = __builtin_shuffle(temp0, temp1, f0, f1, int_mask);
            break;
        }
        case 2: {
            /* Another complex pattern */
            *result = __builtin_shuffle(f0, f1, f2, f3, f0, f1, int_mask);
            break;
        }
        default: {
            /* Mix integer and float masks */
            v16sf temp = __builtin_shuffle(f0, f1, f2, f3, int_mask);
            *result = temp * (v16sf){2.0f};
        }
    }
}
#endif

#ifdef __SSE2__
void mixed_size_shuffle(v8si *result, const int *data, volatile int mask_idx) {
    /* Work with 256-bit vectors */
    v8si v0 = *(const v8si*)(data + 0);
    v8si v1 = *(const v8si*)(data + 8);
    
    /* Control mask from volatile */
    v8si mask;
    for (int i = 0; i < 8; i++) {
        mask[i] = (mask_idx + i) & 0x7;
    }
    
    /* Loop with conditional shuffles */
    for (int i = 0; i < 3; i++) {
        if ((mask_idx >> i) & 1) {
            v0 = __builtin_shuffle(v0, v1, mask);
        } else {
            v1 = __builtin_shuffle(v1, v0, mask);
        }
        /* Modify mask each iteration */
        mask = mask + (v8si){1};
    }
    
    /* Final shuffle combining both vectors */
    *result = __builtin_shuffle(v0, v1, v0, v1, mask);
}
#endif

#ifdef __AVX512F__
void avx512_complex_shuffle(v16si *result, const int *data, volatile int *indices) {
    v16si v0 = *(const v16si*)(data + 0);
    v16si v1 = *(const v16si*)(data + 16);
    v16si v2 = *(const v16si*)(data + 32);
    v16si v3 = *(const v16si*)(data + 48);
    
    /* Multiple control masks */
    v16si mask1, mask2;
    for (int i = 0; i < 16; i++) {
        mask1[i] = (indices[0] + i) & 0xF;
        mask2[i] = (indices[1] * i) & 0xF;
    }
    
    /* Series of shuffles that could expand to many operands */
    v16si step1 = __builtin_shuffle(v0, v1, mask1);
    v16si step2 = __builtin_shuffle(v2, v3, mask2);
    
    /* Complex final shuffle with many input vectors */
    v16si final_mask;
    for (int i = 0; i < 16; i++) {
        final_mask[i] = (indices[2] + i * 3) & 0x1F;
    }
    
    /* This pattern might trigger the 11-operand case */
    *result = __builtin_shuffle(step1, step2, v0, v1, v2, v3, 
                                step1, step2, final_mask);
}
#endif

/* Main test function */
int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    global_seed = seed;
    
    volatile int mask_indices[4];
    for (int i = 0; i < 4; i++) {
        mask_indices[i] = (seed + i * 17) & 0xF;
    }
    
    printf("Testing vector shuffles with seed: %d\n", seed);
    
    /* Main test loop */
    for (int iter = 0; iter < 10; iter++) {
        int base_idx = (iter * 37) & 0x1FF; /* Wrap within array bounds */
        
#ifdef __AVX2__
        /* Test 10+ operand shuffles */
        v16si int_result;
        shuffle_10_operand_int(&int_result, 
                               &global_int_array[base_idx], 
                               mask_indices);
        
        /* Store to accumulator with volatile write */
        volatile v16si *volatile_ptr = &int_result;
        memcpy(&accumulator[base_idx], (void*)volatile_ptr, sizeof(v16si));
        
        v16sf float_result;
        shuffle_11_operand_float(&float_result,
                                 &global_float_array[base_idx],
                                 mask_indices);
        
        /* Convert float result to int and accumulate */
        for (int i = 0; i < 16; i++) {
            accumulator[base_idx + i] += (int)float_result[i];
        }
#endif

#ifdef __SSE2__
        v8si mixed_result;
        mixed_size_shuffle(&mixed_result,
                          &global_int_array[base_idx + 64],
                          mask_indices[0]);
        
        /* Accumulate */
        for (int i = 0; i < 8; i++) {
            accumulator[base_idx + 64 + i] += mixed_result[i];
        }
#endif

#ifdef __AVX512F__
        v16si avx512_result;
        avx512_complex_shuffle(&avx512_result,
                              &global_int_array[base_idx + 128],
                              mask_indices);
        
        /* XOR with accumulator */
        for (int i = 0; i < 16; i++) {
            accumulator[base_idx + 128 + i] ^= avx512_result[i];
        }
#endif
        
        /* Modify mask indices for next iteration */
        for (int i = 0; i < 4; i++) {
            mask_indices[i] = (mask_indices[i] + iter + 1) & 0xF;
        }
    }
    
    /* Compute final checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += (unsigned int)accumulator[i];
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    /* Additional test with inline assembly barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    return 0;
}
