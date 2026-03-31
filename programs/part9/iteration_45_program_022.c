#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
#define ARRAY_SIZE 512
static int32_t global_ints[ARRAY_SIZE];
static double global_floats[ARRAY_SIZE];
static volatile int32_t volatile_buffer[ARRAY_SIZE];
static volatile double volatile_double_buffer[ARRAY_SIZE];

/* Vector type definitions using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef double v4df __attribute__((vector_size(32)));       /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));       /* 512-bit double */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */

/* Accumulator arrays */
static v8si int_accum_256[4];
static v16si int_accum_512[4];
static v4df float_accum_256[4];
static v8df double_accum_512[4];

/* Initialize global arrays with deterministic pseudo-random values */
void init_arrays(unsigned int seed) {
    srand(seed);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_ints[i] = (int32_t)(rand() % 1000);
        global_floats[i] = (double)(rand() % 1000) / 10.0;
    }
    
    /* Initialize accumulators to zero */
    memset(int_accum_256, 0, sizeof(int_accum_256));
    memset(int_accum_512, 0, sizeof(int_accum_512));
    memset(float_accum_256, 0, sizeof(float_accum_256));
    memset(double_accum_512, 0, sizeof(double_accum_512));
}

/* Function that uses __builtin_shuffle with many operands */
#ifdef __AVX2__
static void shuffle_10_operand_int(v16si *acc, const int32_t *data, volatile int *mask_indices) {
    /* Load 4 vectors of 512-bit integers (16 ints each) */
    v16si v1 = *(const v16si *)(data);
    v16si v2 = *(const v16si *)(data + 16);
    v16si v3 = *(const v16si *)(data + 32);
    v16si v4 = *(const v16si *)(data + 48);
    
    /* Create control mask from volatile indices to prevent constant folding */
    int idx0 = mask_indices[0] % 16;
    int idx1 = mask_indices[1] % 16;
    int idx2 = mask_indices[2] % 16;
    int idx3 = mask_indices[3] % 16;
    int idx4 = mask_indices[4] % 16;
    int idx5 = mask_indices[5] % 16;
    int idx6 = mask_indices[6] % 16;
    int idx7 = mask_indices[7] % 16;
    int idx8 = mask_indices[8] % 16;
    int idx9 = mask_indices[9] % 16;
    
    /* This shuffle uses 11 operands total: 4 input vectors + 7-element mask */
    /* The mask indices are runtime values to prevent optimization */
    v16si shuffled = __builtin_shufflevector(v1, v2, v3, v4,
        idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7,
        idx8, idx9, 10, 11, 12, 13, 14, 15);
    
    /* Perform arithmetic to ensure result is used */
    *acc = *acc + shuffled;
    
    /* Volatile store to prevent dead code elimination */
    volatile_buffer[0] = shuffled[0];
}
#endif

#ifdef __AVX512F__
static void shuffle_11_operand_float(v8df *acc, const double *data, volatile int *mask_indices) {
    /* Load vectors */
    v8df v1 = *(const v8df *)(data);
    v8df v2 = *(const v8df *)(data + 8);
    v8df v3 = *(const v8df *)(data + 16);
    v8df v4 = *(const v8df *)(data + 24);
    
    /* Get mask indices from volatile source */
    int idx0 = mask_indices[0] % 8;
    int idx1 = mask_indices[1] % 8;
    int idx2 = mask_indices[2] % 8;
    int idx3 = mask_indices[3] % 8;
    int idx4 = mask_indices[4] % 8;
    int idx5 = mask_indices[5] % 8;
    int idx6 = mask_indices[6] % 8;
    int idx7 = mask_indices[7] % 8;
    int idx8 = mask_indices[8] % 8;
    int idx9 = mask_indices[9] % 8;
    int idx10 = mask_indices[10] % 8;
    
    /* This uses 12 operands total: 4 input vectors + 8-element mask */
    v8df shuffled = __builtin_shufflevector(v1, v2, v3, v4,
        idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7);
    
    /* Complex control flow with switch */
    switch (mask_indices[0] % 4) {
        case 0:
            /* Another shuffle with mixed types */
            v4df narrow = __builtin_shufflevector(shuffled, shuffled,
                idx8 % 4, idx9 % 4, idx10 % 4, (idx8 + idx9) % 4);
            float_accum_256[0] = float_accum_256[0] + *(v4df*)&narrow;
            break;
        case 1:
            *acc = *acc + shuffled * 2.0;
            break;
        case 2:
            *acc = *acc + shuffled + 1.0;
            break;
        default:
            *acc = *acc - shuffled;
            break;
    }
    
    volatile_double_buffer[0] = shuffled[0];
}
#endif

#ifdef __SSE2__
static void mixed_size_shuffle(v8si *acc, const int32_t *data, volatile int *mask_indices) {
    /* Work with 256-bit vectors */
    v8si v1 = *(const v8si *)(data);
    v8si v2 = *(const v8si *)(data + 8);
    v8si v3 = *(const v8si *)(data + 16);
    v8si v4 = *(const v8si *)(data + 24);
    
    int idx0 = mask_indices[0] % 8;
    int idx1 = mask_indices[1] % 8;
    int idx2 = mask_indices[2] % 8;
    int idx3 = mask_indices[3] % 8;
    int idx4 = mask_indices[4] % 8;
    int idx5 = mask_indices[5] % 8;
    int idx6 = mask_indices[6] % 8;
    int idx7 = mask_indices[7] % 8;
    
    /* Nested if-else chain with shuffles */
    if (mask_indices[0] & 1) {
        /* Shuffle with 10 operands: 2 vectors + 8-element mask */
        v8si shuffled = __builtin_shuffle(v1, v2, 
            (v8si){idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7});
        *acc = *acc + shuffled;
    } else if (mask_indices[1] & 1) {
        /* Different shuffle pattern */
        v8si shuffled = __builtin_shuffle(v3, v4,
            (v8si){idx7, idx6, idx5, idx4, idx3, idx2, idx1, idx0});
        *acc = *acc - shuffled;
    } else {
        /* Complex pattern with multiple shuffles */
        v8si tmp1 = __builtin_shuffle(v1, v2,
            (v8si){idx0, idx1, idx2, idx3, 4, 5, 6, 7});
        v8si tmp2 = __builtin_shuffle(v3, v4,
            (v8si){0, 1, 2, 3, idx4, idx5, idx6, idx7});
        *acc = *acc + tmp1 * tmp2;
    }
    
    volatile_buffer[1] = (*acc)[0];
}
#endif

/* Generic function that works on any architecture */
static void generic_vector_ops(int iter, volatile int *mask_indices) {
    /* Use different data slices each iteration */
    const int32_t *int_data = global_ints + (iter * 64) % (ARRAY_SIZE - 128);
    const double *float_data = global_floats + (iter * 64) % (ARRAY_SIZE - 128);
    
    /* Update mask indices based on iteration */
    for (int i = 0; i < 16; i++) {
        mask_indices[i] = (mask_indices[i] + iter * 7) % 32;
    }
    
    /* Architecture-specific operations */
#ifdef __AVX512F__
    shuffle_11_operand_float(&double_accum_512[iter % 4], float_data, mask_indices);
#endif
    
#ifdef __AVX2__
    shuffle_10_operand_int(&int_accum_512[iter % 4], int_data, mask_indices);
#endif
    
#ifdef __SSE2__
    mixed_size_shuffle(&int_accum_256[iter % 4], int_data, mask_indices);
#endif
    
    /* Mixed SIMD pattern: convert between sizes */
    if (iter % 3 == 0) {
        /* Load 256-bit, narrow to 128-bit conceptually */
        v8si wide = *(const v8si *)int_data;
        /* Use shuffle to extract halves */
        v8si shuffled = __builtin_shuffle(wide, wide,
            (v8si){0, 1, 2, 3, 4, 5, 6, 7});
        int_accum_256[(iter + 1) % 4] = int_accum_256[(iter + 1) % 4] + shuffled;
    }
}

/* Compute checksum from all accumulators */
int64_t compute_checksum(void) {
    int64_t checksum = 0;
    
    /* Sum integer accumulators */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += int_accum_256[i][j];
        }
    }
    
#ifdef __AVX2__
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            checksum += int_accum_512[i][j];
        }
    }
#endif
    
    /* Sum float accumulators (convert to int for checksum) */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            checksum += (int64_t)float_accum_256[i][j];
        }
    }
    
#ifdef __AVX512F__
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += (int64_t)double_accum_512[i][j];
        }
    }
#endif
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Use command-line seed or default */
    unsigned int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    /* Volatile mask indices to prevent constant folding */
    volatile int mask_indices[16];
    for (int i = 0; i < 16; i++) {
        mask_indices[i] = (i * 13 + seed) % 32;
    }
    
    /* Main loop with complex control flow */
    printf("Starting vector operations...\n");
    
    for (int iter = 0; iter < 10; iter++) {
        /* Complex control flow around vector operations */
        if (iter % 2 == 0) {
            generic_vector_ops(iter, (int*)mask_indices);
        } else if (iter % 3 == 0) {
            /* Alternative path */
            for (int j = 0; j < 2; j++) {
                generic_vector_ops(iter + j, (int*)mask_indices);
            }
        } else {
            switch (iter % 4) {
                case 0:
                    generic_vector_ops(iter, (int*)mask_indices);
                    break;
                case 1:
                    generic_vector_ops(iter * 2, (int*)mask_indices);
                    break;
                case 2:
                    generic_vector_ops(iter * 3, (int*)mask_indices);
                    break;
                default:
                    generic_vector_ops(iter * 4, (int*)mask_indices);
                    break;
            }
        }
        
        /* Update volatile indices */
        for (int i = 0; i < 16; i++) {
            mask_indices[i] = (mask_indices[i] + 1) % 32;
        }
    }
    
    /* Compute and print final checksum */
    int64_t checksum = compute_checksum();
    printf("Final checksum: %ld\n", (long)checksum);
    
    return 0;
}
