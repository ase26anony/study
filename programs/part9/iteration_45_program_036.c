#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
#define ARRAY_SIZE 512
static int32_t global_int_array[ARRAY_SIZE];
static float global_float_array[ARRAY_SIZE];
static volatile int32_t volatile_buffer[ARRAY_SIZE];
static volatile float volatile_float_buffer[ARRAY_SIZE];

/* Vector type definitions using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef float v8sf __attribute__((vector_size(32)));        /* 256-bit float */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */
typedef double v4df __attribute__((vector_size(32)));       /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));       /* 512-bit double */

/* Accumulator arrays for results */
static v8si int_accumulator_256[4];
static v16si int_accumulator_512[4];
static v8sf float_accumulator_256[4];
static v16sf float_accumulator_512[4];

/* Initialize arrays with deterministic pseudo-random values */
void initialize_arrays(int seed) {
    srand(seed);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = rand() % 1000;
        global_float_array[i] = (rand() % 1000) / 10.0f;
    }
}

/* Function that uses __builtin_shuffle with many operands (10+ total) */
#ifdef __AVX2__
static v8si shuffle_10_operand_int256(v8si a, v8si b, v8si c, v8si d, 
                                      volatile int* mask_ptr) {
    /* Create a control mask from volatile memory to prevent constant folding */
    v8si mask;
    for (int i = 0; i < 8; i++) {
        ((int*)&mask)[i] = mask_ptr[i] % 16;  /* 0-15 indices for 16 elements */
    }
    
    /* This shuffle will be expanded with many operands */
    v8si result = __builtin_shuffle(a, b, mask);
    
    /* Use result in computation to prevent elimination */
    return result + c - d;
}

static v8sf shuffle_11_operand_float256(v8sf a, v8sf b, v8sf c, v8sf d,
                                        volatile int* mask_ptr) {
    /* Create two masks for complex shuffling */
    v8si mask1, mask2;
    for (int i = 0; i < 8; i++) {
        ((int*)&mask1)[i] = mask_ptr[i] % 16;
        ((int*)&mask2)[i] = mask_ptr[i + 8] % 16;
    }
    
    /* Nested shuffles that may require many operands during expansion */
    v8sf temp1 = __builtin_shuffle(a, b, mask1);
    v8sf temp2 = __builtin_shuffle(c, d, mask2);
    
    /* Final shuffle combining results - potentially 11+ operands */
    v8si combined_mask;
    for (int i = 0; i < 8; i++) {
        ((int*)&combined_mask)[i] = (mask_ptr[i] + mask_ptr[i+8]) % 16;
    }
    
    return __builtin_shuffle(temp1, temp2, combined_mask);
}
#endif

#ifdef __AVX512F__
/* 512-bit vector operations that require many operands */
static v16si shuffle_many_operands_int512(v16si a, v16si b, v16si c, 
                                          volatile int* mask_ptr) {
    /* Create control mask with 16 elements */
    v16si mask;
    for (int i = 0; i < 16; i++) {
        ((int*)&mask)[i] = mask_ptr[i] % 32;  /* 0-31 indices for 32 elements */
    }
    
    /* Complex shuffle pattern that may expand to many operands */
    v16si shuffled = __builtin_shuffle(a, b, mask);
    
    /* Additional operation to use the result */
    return shuffled * c;
}

/* Function using __builtin_shufflevector with explicit indices */
static v8df shufflevector_many_args(v8df a, v8df b, volatile int* idx_ptr) {
    /* Use volatile indices to prevent constant folding */
    /* This creates a call with many explicit index arguments */
    return __builtin_shufflevector(a, b, 
        idx_ptr[0] % 16, idx_ptr[1] % 16, idx_ptr[2] % 16, idx_ptr[3] % 16,
        idx_ptr[4] % 16, idx_ptr[5] % 16, idx_ptr[6] % 16, idx_ptr[7] % 16,
        idx_ptr[8] % 16, idx_ptr[9] % 16, idx_ptr[10] % 16, idx_ptr[11] % 16,
        idx_ptr[12] % 16, idx_ptr[13] % 16, idx_ptr[14] % 16, idx_ptr[15] % 16);
}
#endif

/* Mixed SIMD patterns with control flow */
static void process_vector_patterns(int pattern_id, volatile int* mask, int iter) {
    /* Complex control flow to stress the expander */
    switch (pattern_id % 4) {
        case 0: {
#ifdef __AVX2__
            /* Load data into vectors */
            v8si va = *(v8si*)&global_int_array[iter * 8];
            v8si vb = *(v8si*)&global_int_array[iter * 8 + 8];
            v8si vc = *(v8si*)&global_int_array[iter * 8 + 16];
            v8si vd = *(v8si*)&global_int_array[iter * 8 + 24];
            
            /* Perform shuffle with many operands */
            v8si result = shuffle_10_operand_int256(va, vb, vc, vd, mask);
            
            /* Store to volatile buffer to prevent elimination */
            *(v8si*)&volatile_buffer[iter * 8] = result;
            
            /* Accumulate result */
            int_accumulator_256[pattern_id % 4] += result;
#endif
            break;
        }
        
        case 1: {
#ifdef __AVX2__
            /* Float vector processing */
            v8sf fa = *(v8sf*)&global_float_array[iter * 8];
            v8sf fb = *(v8sf*)&global_float_array[iter * 8 + 8];
            v8sf fc = *(v8sf*)&global_float_array[iter * 8 + 16];
            v8sf fd = *(v8sf*)&global_float_array[iter * 8 + 24];
            
            v8sf fresult = shuffle_11_operand_float256(fa, fb, fc, fd, mask);
            
            *(v8sf*)&volatile_float_buffer[iter * 8] = fresult;
            float_accumulator_256[pattern_id % 4] += fresult;
#endif
            break;
        }
        
        case 2: {
#ifdef __AVX512F__
            /* 512-bit integer operations */
            v16si va512 = *(v16si*)&global_int_array[iter * 16];
            v16si vb512 = *(v16si*)&global_int_array[iter * 16 + 16];
            v16si vc512 = *(v16si*)&global_int_array[iter * 16 + 32];
            
            v16si result512 = shuffle_many_operands_int512(va512, vb512, vc512, mask);
            
            /* Store through volatile pointer */
            for (int i = 0; i < 16; i++) {
                volatile_buffer[iter * 16 + i] = ((int32_t*)&result512)[i];
            }
            
            int_accumulator_512[pattern_id % 4] += result512;
#endif
            break;
        }
        
        case 3: {
#ifdef __AVX512F__
            /* 512-bit double operations */
            v8df da = *(v8df*)&global_float_array[iter * 8];
            v8df db = *(v8df*)&global_float_array[iter * 8 + 8];
            
            v8df dresult = shufflevector_many_args(da, db, mask);
            
            /* Store result */
            for (int i = 0; i < 8; i++) {
                volatile_float_buffer[iter * 8 + i] = ((double*)&dresult)[i];
            }
            
            /* Convert and accumulate in float accumulator */
            for (int i = 0; i < 8; i++) {
                ((float*)&float_accumulator_512[pattern_id % 4])[i] += ((double*)&dresult)[i];
            }
#endif
            break;
        }
    }
}

/* Main function with complex control flow */
int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    initialize_arrays(seed);
    
    /* Initialize accumulators */
    memset(int_accumulator_256, 0, sizeof(int_accumulator_256));
    memset(int_accumulator_512, 0, sizeof(int_accumulator_512));
    memset(float_accumulator_256, 0, sizeof(float_accumulator_256));
    memset(float_accumulator_512, 0, sizeof(float_accumulator_512));
    
    /* Create volatile mask array with runtime-dependent values */
    volatile int mask[32];
    for (int i = 0; i < 32; i++) {
        mask[i] = (seed + i * 17) % 64;  /* Deterministic but runtime value */
    }
    
    /* Main processing loop with nested control flow */
    for (int iter = 0; iter < 10; iter++) {
        /* Vary pattern based on iteration and mask values */
        int pattern_selector = mask[iter % 32] % 8;
        
        if (pattern_selector < 4) {
            /* Process with current pattern */
            process_vector_patterns(pattern_selector, mask, iter);
        } else if (pattern_selector < 6) {
            /* Alternative path with different mask offset */
            process_vector_patterns(pattern_selector - 2, &mask[16], iter);
        } else {
            /* Third path with modified mask */
            volatile int alt_mask[32];
            for (int i = 0; i < 32; i++) {
                alt_mask[i] = (mask[i] + iter) % 64;
            }
            process_vector_patterns(pattern_selector - 4, alt_mask, iter);
        }
        
        /* Modify mask based on iteration to create varying patterns */
        if (iter % 3 == 0) {
            for (int i = 0; i < 32; i++) {
                mask[i] = (mask[i] + 1) % 64;
            }
        }
    }
    
    /* Compute checksum from all accumulators */
    int64_t int_checksum = 0;
    float float_checksum = 0.0f;
    
    /* Sum integer accumulators */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            int_checksum += ((int32_t*)&int_accumulator_256[i])[j];
        }
    }
    
#ifdef __AVX512F__
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            int_checksum += ((int32_t*)&int_accumulator_512[i])[j];
        }
    }
#endif
    
    /* Sum float accumulators */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            float_checksum += ((float*)&float_accumulator_256[i])[j];
        }
    }
    
#ifdef __AVX512F__
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            float_checksum += ((float*)&float_accumulator_512[i])[j];
        }
    }
#endif
    
    /* Also include volatile buffers in checksum */
    for (int i = 0; i < ARRAY_SIZE && i < 100; i++) {
        int_checksum += volatile_buffer[i];
        float_checksum += volatile_float_buffer[i];
    }
    
    printf("Integer checksum: %lld\n", (long long)int_checksum);
    printf("Float checksum: %f\n", float_checksum);
    
    return 0;
}
