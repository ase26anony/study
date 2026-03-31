#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int global_seed;
int global_int_array[512];
float global_float_array[512];
int accumulator[512] = {0};

/* Vector type definitions using GCC extensions */
typedef int v8si __attribute__((vector_size(32)));      /* 8 ints - 256-bit */
typedef int v16si __attribute__((vector_size(64)));     /* 16 ints - 512-bit */
typedef float v8sf __attribute__((vector_size(32)));    /* 8 floats - 256-bit */
typedef float v16sf __attribute__((vector_size(64)));   /* 16 floats - 512-bit */
typedef double v4df __attribute__((vector_size(32)));   /* 4 doubles - 256-bit */
typedef double v8df __attribute__((vector_size(64)));   /* 8 doubles - 512-bit */

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    unsigned int r = seed;
    for (int i = 0; i < 512; i++) {
        /* Simple LCG */
        r = r * 1103515245 + 12345;
        global_int_array[i] = (int)(r % 1000);
        global_float_array[i] = (float)(r % 1000) * 0.1f;
    }
}

/* Function using __builtin_shuffle with many operands - targeting 10+ operands */
#ifdef __AVX2__
void shuffle_v16si_complex(v16si *result, const int *data, volatile int *mask_indices) {
    v16si a, b, c, d;
    
    /* Load data into vectors */
    a = *(const v16si*)(data);
    b = *(const v16si*)(data + 16);
    c = *(const v16si*)(data + 32);
    d = *(const v16si*)(data + 48);
    
    /* Create control mask from volatile indices to prevent constant folding */
    int mask[16];
    for (int i = 0; i < 16; i++) {
        mask[i] = mask_indices[i % 4] + i;
    }
    
    v16si mask_vec = *(v16si*)mask;
    
    /* Complex shuffle pattern that may require many operands during expansion */
    v16si shuffled;
    
    /* Use volatile condition to prevent optimization */
    volatile int choice = global_seed;
    
    if (choice & 1) {
        /* Pattern 1: Shuffle with 10+ operand requirement */
        shuffled = __builtin_shuffle(a, b, mask_vec);
    } else if (choice & 2) {
        /* Pattern 2: Another shuffle variant */
        shuffled = __builtin_shuffle(b, c, mask_vec);
    } else {
        /* Pattern 3: Complex shuffle mixing all vectors */
        /* This may expand to many operands */
        v16si temp1 = __builtin_shuffle(a, b, mask_vec);
        v16si temp2 = __builtin_shuffle(c, d, mask_vec);
        shuffled = __builtin_shuffle(temp1, temp2, mask_vec);
    }
    
    /* Perform arithmetic to ensure value is used */
    shuffled = shuffled + (v16si){1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    
    *result = shuffled;
}
#endif

#ifdef __AVX512F__
void shuffle_v16sf_mixed(v16sf *result, const float *data, volatile int *indices) {
    v16sf a, b;
    
    /* Load data */
    a = *(const v16sf*)(data);
    b = *(const v16sf*)(data + 16);
    
    /* Create complex mask using indices */
    int mask[16];
    for (int i = 0; i < 16; i++) {
        mask[i] = (indices[i % 8] + i * 3) % 32;
    }
    
    v16si mask_int_vec = *(v16si*)mask;
    
    /* Switch statement to create complex control flow */
    volatile int mode = global_seed % 4;
    v16sf shuffled;
    
    switch (mode) {
        case 0:
            /* Direct shuffle - may use many operands */
            shuffled = __builtin_shuffle(a, b, mask_int_vec);
            break;
        case 1:
            /* Nested shuffles */
            {
                v16sf temp = __builtin_shuffle(a, a, mask_int_vec);
                shuffled = __builtin_shuffle(temp, b, mask_int_vec);
            }
            break;
        case 2:
            /* Multiple shuffle operations in sequence */
            {
                int mask2[16];
                for (int i = 0; i < 16; i++) mask2[i] = mask[i] ^ 1;
                v16si mask2_vec = *(v16si*)mask2;
                
                v16sf temp1 = __builtin_shuffle(a, b, mask_int_vec);
                v16sf temp2 = __builtin_shuffle(b, a, mask2_vec);
                shuffled = __builtin_shuffle(temp1, temp2, mask_int_vec);
            }
            break;
        default:
            /* Complex pattern with arithmetic */
            shuffled = __builtin_shuffle(a + b, a - b, mask_int_vec);
            break;
    }
    
    *result = shuffled * 2.0f;
}
#endif

/* Function using __builtin_shufflevector with large vectors */
#ifdef __SSE2__
void shuffle_v8df_pattern(v8df *result, const double *data, volatile int *mask_idx) {
    v8df a, b;
    
    a = *(const v8df*)(data);
    b = *(const v8df*)(data + 8);
    
    /* Create variable indices to prevent constant folding */
    volatile int idx0 = mask_idx[0];
    volatile int idx1 = mask_idx[1];
    volatile int idx2 = mask_idx[2];
    volatile int idx3 = mask_idx[3];
    volatile int idx4 = mask_idx[4];
    volatile int idx5 = mask_idx[5];
    volatile int idx6 = mask_idx[6];
    volatile int idx7 = mask_idx[7];
    
    /* Loop with conditional shuffles */
    v8df shuffled;
    for (int iter = 0; iter < 3; iter++) {
        if (iter == 0) {
            /* __builtin_shufflevector with many indices - may require 10+ operands */
            shuffled = __builtin_shufflevector(a, b, 
                idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7,
                8+idx0, 8+idx1, 8+idx2, 8+idx3, 8+idx4, 8+idx5, 8+idx6, 8+idx7);
        } else if (iter == 1) {
            /* Different pattern */
            shuffled = __builtin_shufflevector(b, a,
                idx7, idx6, idx5, idx4, idx3, idx2, idx1, idx0,
                8+idx7, 8+idx6, 8+idx5, 8+idx4, 8+idx3, 8+idx2, 8+idx1, 8+idx0);
        } else {
            /* Mixed pattern */
            shuffled = __builtin_shufflevector(a, b,
                idx0%8, (idx1+1)%8, (idx2+2)%8, (idx3+3)%8,
                (idx4+4)%8, (idx5+5)%8, (idx6+6)%8, (idx7+7)%8,
                8+(idx0%8), 8+((idx1+1)%8), 8+((idx2+2)%8), 8+((idx3+3)%8),
                8+((idx4+4)%8), 8+((idx5+5)%8), 8+((idx6+6)%8), 8+((idx7+7)%8));
        }
        
        /* Use result to prevent elimination */
        *result = *result + shuffled;
    }
}
#endif

/* Mixed-size vector operations */
void mixed_size_shuffles(int *acc, const int *data, volatile int *indices) {
    /* Use different vector sizes in the same function */
    v8si v256_a, v256_b;
    v16si v512_temp;
    
    /* Load 256-bit vectors */
    v256_a = *(const v8si*)(data);
    v256_b = *(const v8si*)(data + 8);
    
    /* Create mask from volatile indices */
    int mask8[8];
    for (int i = 0; i < 8; i++) {
        mask8[i] = (indices[i] + i) % 16;
    }
    v8si mask8_vec = *(v8si*)mask8;
    
    /* Shuffle 256-bit vectors */
    v8si shuffled256 = __builtin_shuffle(v256_a, v256_b, mask8_vec);
    
    /* Expand to 512-bit (simulated) */
    v16si expanded;
    memcpy(&expanded, &shuffled256, 32);
    memcpy((char*)&expanded + 32, &shuffled256, 32);
    
    /* Store to accumulator */
    for (int i = 0; i < 16; i++) {
        acc[i] += expanded[i];
    }
}

int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    global_seed = seed;
    init_arrays(seed);
    
    /* Volatile mask indices to prevent constant folding */
    volatile int mask_indices[16];
    for (int i = 0; i < 16; i++) {
        mask_indices[i] = (seed + i * 7) % 32;
    }
    
    printf("Starting vector shuffle tests with seed %d\n", seed);
    
    /* Main test loop */
    for (int iter = 0; iter < 10; iter++) {
        int base_idx = (iter * 37) % 256;
        
        /* Call different shuffle functions based on iteration */
        if (iter % 3 == 0) {
#ifdef __AVX2__
            v16si int_result;
            shuffle_v16si_complex(&int_result, 
                                 &global_int_array[base_idx], 
                                 (int*)mask_indices);
            
            /* Accumulate results */
            for (int i = 0; i < 16; i++) {
                accumulator[base_idx + i] += int_result[i];
            }
#endif
        } else if (iter % 3 == 1) {
#ifdef __AVX512F__
            v16sf float_result;
            shuffle_v16sf_mixed(&float_result,
                               &global_float_array[base_idx],
                               (int*)mask_indices);
            
            /* Convert and accumulate */
            for (int i = 0; i < 16; i++) {
                accumulator[base_idx + i] += (int)float_result[i];
            }
#endif
        } else {
#ifdef __SSE2__
            v8df double_result = {0};
            shuffle_v8df_pattern(&double_result,
                                (double*)&global_float_array[base_idx],
                                (int*)mask_indices);
            
            /* Accumulate */
            for (int i = 0; i < 8; i++) {
                accumulator[base_idx + i] += (int)double_result[i];
            }
#endif
            
            /* Also call mixed-size function */
            mixed_size_shuffles(&accumulator[base_idx],
                               &global_int_array[base_idx],
                               (int*)mask_indices);
        }
        
        /* Modify mask indices for next iteration */
        for (int i = 0; i < 16; i++) {
            mask_indices[i] = (mask_indices[i] + 11) % 32;
        }
    }
    
    /* Compute final checksum */
    long long checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += accumulator[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}
