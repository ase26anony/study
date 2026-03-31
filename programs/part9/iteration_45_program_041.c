#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int global_seed;
int global_int_array[512];
float global_float_array[512];
int accumulator[512] = {0};

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    unsigned int r = seed;
    for (int i = 0; i < 512; i++) {
        /* Simple LCG */
        r = r * 1103515245 + 12345;
        global_int_array[i] = (int)(r >> 16) & 0x7FFF;
        global_float_array[i] = (float)((r >> 16) & 0xFF) / 256.0f;
    }
}

/* Vector types using GCC extensions */
typedef int v8si __attribute__((vector_size(32)));      /* 8x int32 (256-bit) */
typedef float v8sf __attribute__((vector_size(32)));    /* 8x float (256-bit) */
typedef int v16si __attribute__((vector_size(64)));     /* 16x int32 (512-bit) */
typedef float v16sf __attribute__((vector_size(64)));   /* 16x float (512-bit) */
typedef int v4si __attribute__((vector_size(16)));      /* 4x int32 (128-bit) */
typedef float v4sf __attribute__((vector_size(16)));    /* 4x float (128-bit) */

/* Function 1: Complex shuffle with 10+ operands using __builtin_shuffle */
#ifdef __AVX2__
v8si shuffle_10_operand_int(v8si a, v8si b, v8si mask) {
    volatile int idx = global_seed & 7;
    
    /* Complex control flow to prevent optimization */
    if (idx > 4) {
        /* This shuffle uses 10 operands: a, b, and mask elements 0-7 */
        v8si result = __builtin_shuffle(a, b, 
            (v8si){mask[0] & 15, mask[1] & 15, mask[2] & 15, mask[3] & 15,
                   mask[4] & 15, mask[5] & 15, mask[6] & 15, mask[7] & 15});
        return result + (v8si){1, 2, 3, 4, 5, 6, 7, 8};
    } else {
        /* Alternative shuffle pattern */
        v8si result = __builtin_shuffle(a, b,
            (v8si){mask[7] & 15, mask[6] & 15, mask[5] & 15, mask[4] & 15,
                   mask[3] & 15, mask[2] & 15, mask[1] & 15, mask[0] & 15});
        return result * (v8si){2, 2, 2, 2, 2, 2, 2, 2};
    }
}
#endif

#ifdef __AVX512F__
/* Function 2: 11-operand shuffle with float vectors */
v16sf shuffle_11_operand_float(v16sf a, v16sf b, v16si mask) {
    volatile int pattern = global_seed % 3;
    
    switch (pattern) {
        case 0:
            /* 11 operands: a, b, and mask elements 0-9 */
            return __builtin_shuffle(a, b,
                (v16si){mask[0] & 31, mask[1] & 31, mask[2] & 31, mask[3] & 31,
                        mask[4] & 31, mask[5] & 31, mask[6] & 31, mask[7] & 31,
                        mask[8] & 31, mask[9] & 31, mask[10] & 31, mask[11] & 31,
                        mask[12] & 31, mask[13] & 31, mask[14] & 31, mask[15] & 31});
        case 1:
            /* Another 11-operand pattern */
            return __builtin_shuffle(b, a,
                (v16si){mask[15] & 31, mask[14] & 31, mask[13] & 31, mask[12] & 31,
                        mask[11] & 31, mask[10] & 31, mask[9] & 31, mask[8] & 31,
                        mask[7] & 31, mask[6] & 31, mask[5] & 31, mask[4] & 31,
                        mask[3] & 31, mask[2] & 31, mask[1] & 31, mask[0] & 31});
        default:
            /* Mixed pattern */
            return __builtin_shuffle(a, b,
                (v16si){0, 16, 1, 17, 2, 18, 3, 19,
                        4, 20, 5, 21, 6, 22, 7, 23});
    }
}
#endif

/* Function 3: Mixed-size shuffles using __builtin_shufflevector */
#ifdef __SSE2__
v4si mixed_size_shuffle(v8si large_vec, v4si small_vec, int* indices) {
    volatile int mode = global_seed & 1;
    
    if (mode) {
        /* Narrow 256-bit to 128-bit with complex control */
        v4si narrowed = __builtin_shufflevector(large_vec, large_vec,
            indices[0] & 7, indices[1] & 7, indices[2] & 7, indices[3] & 7);
        
        /* Expand back with shuffle - total 10+ operands */
        v8si expanded = __builtin_shufflevector(narrowed, narrowed,
            indices[4] & 3, indices[5] & 3, indices[6] & 3, indices[7] & 3,
            indices[8] & 3, indices[9] & 3, indices[10] & 3, indices[11] & 3);
        
        /* Extract portion for return */
        return (v4si){expanded[0], expanded[1], expanded[2], expanded[3]};
    } else {
        /* Different pattern */
        v4si result = __builtin_shufflevector(small_vec, small_vec,
            indices[0] & 3, indices[1] & 3, indices[2] & 3, indices[3] & 3);
        return result * (v4si){3, 3, 3, 3};
    }
}
#endif

/* Function 4: Complex shuffle chain with arithmetic */
#ifdef __AVX2__
void process_vector_chain(int* data, float* fdata, int iter) {
    v8si int_vec1, int_vec2, mask_int;
    v8sf float_vec1, float_vec2;
    
    /* Load data with volatile pointer to prevent optimization */
    volatile int* vdata = data + iter * 8;
    volatile float* vfdata = fdata + iter * 8;
    
    /* Load vectors */
    for (int i = 0; i < 8; i++) {
        int_vec1[i] = vdata[i];
        int_vec2[i] = vdata[i + 8];
        float_vec1[i] = vfdata[i];
        float_vec2[i] = vfdata[i + 8];
        mask_int[i] = (iter + i) & 7;
    }
    
    /* Complex control flow with nested shuffles */
    for (int j = 0; j < 4; j++) {
        volatile int inner_ctrl = (iter + j) & 3;
        
        if (inner_ctrl == 0) {
            /* Shuffle with 10+ operands */
            v8si shuffled = __builtin_shuffle(int_vec1, int_vec2,
                (v8si){mask_int[0], mask_int[1], mask_int[2], mask_int[3],
                       mask_int[4], mask_int[5], mask_int[6], mask_int[7]});
            
            /* Arithmetic operation */
            int_vec1 = shuffled + (v8si){j, j+1, j+2, j+3, j+4, j+5, j+6, j+7};
        } else if (inner_ctrl == 1) {
            /* Float shuffle */
            v8sf temp = __builtin_shuffle(float_vec1, float_vec2,
                (v8si){7, 6, 5, 4, 3, 2, 1, 0});
            float_vec1 = temp * (v8sf){2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
        }
        
        /* Store intermediate results to prevent elimination */
        for (int k = 0; k < 8; k++) {
            accumulator[iter * 8 + k] += int_vec1[k];
        }
    }
}
#endif

int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    global_seed = seed;
    init_arrays(seed);
    
    int shuffle_indices[16];
    for (int i = 0; i < 16; i++) {
        shuffle_indices[i] = (seed + i * 3) & 15;
    }
    
    printf("Starting vector shuffle tests with seed %d\n", seed);
    
    /* Main processing loop */
    for (int iter = 0; iter < 10; iter++) {
        volatile int loop_var = iter;
        
#ifdef __AVX2__
        /* Process integer vectors */
        process_vector_chain(global_int_array, global_float_array, iter);
#endif
        
#ifdef __SSE2__
        /* Mixed-size operations */
        if (loop_var & 1) {
            v8si large_vec;
            v4si small_vec;
            
            for (int i = 0; i < 8; i++) {
                large_vec[i] = global_int_array[iter * 8 + i];
            }
            for (int i = 0; i < 4; i++) {
                small_vec[i] = global_int_array[iter * 4 + i + 256];
            }
            
            v4si result = mixed_size_shuffle(large_vec, small_vec, shuffle_indices);
            
            /* Accumulate results */
            for (int i = 0; i < 4; i++) {
                accumulator[iter * 4 + i + 128] += result[i];
            }
        }
#endif
        
#ifdef __AVX512F__
        /* 512-bit vector operations */
        if ((loop_var % 3) == 0) {
            v16sf vec_a, vec_b;
            v16si shuffle_mask;
            
            for (int i = 0; i < 16; i++) {
                vec_a[i] = global_float_array[iter * 16 + i];
                vec_b[i] = global_float_array[iter * 16 + i + 128];
                shuffle_mask[i] = shuffle_indices[i];
            }
            
            v16sf shuffled = shuffle_11_operand_float(vec_a, vec_b, shuffle_mask);
            
            /* Convert and accumulate */
            for (int i = 0; i < 16; i++) {
                accumulator[iter * 4 + i] += (int)(shuffled[i] * 100.0f);
            }
        }
#endif
    }
    
    /* Compute final checksum */
    long long checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += accumulator[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}
