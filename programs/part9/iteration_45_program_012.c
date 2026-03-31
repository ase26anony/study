#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int32_t global_int_data[512];
volatile float global_float_data[512];
int32_t accumulator_int[512];
float accumulator_float[512];

/* Initialize with deterministic pseudo-random sequence */
void init_data(int seed) {
    srand(seed);
    for (int i = 0; i < 512; i++) {
        global_int_data[i] = (rand() % 1000) - 500;
        global_float_data[i] = (rand() % 1000) / 100.0f - 5.0f;
        accumulator_int[i] = 0;
        accumulator_float[i] = 0.0f;
    }
}

/* Vector types using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 8x int32 (256-bit) */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 16x int32 (512-bit) */
typedef float v8sf __attribute__((vector_size(32)));        /* 8x float (256-bit) */
typedef float v16sf __attribute__((vector_size(64)));       /* 16x float (512-bit) */
typedef int64_t v4di __attribute__((vector_size(32)));      /* 4x int64 (256-bit) */
typedef double v4df __attribute__((vector_size(32)));       /* 4x double (256-bit) */
typedef double v8df __attribute__((vector_size(64)));       /* 8x double (512-bit) */

/* Function to create volatile mask indices */
volatile int get_mask_index(int i, int mod) {
    static volatile int counter = 0;
    counter = (counter + 1) % 256;
    return (i + counter) % mod;
}

/* Complex shuffle with 10+ operands - integer version */
void shuffle_int_10_operand(int32_t* src, int32_t* dst, int start_idx, volatile int mask_seed) {
    /* Load data into large vectors */
    v16si vec1 = *(v16si*)&src[start_idx];
    v16si vec2 = *(v16si*)&src[start_idx + 16];
    
    /* Create control mask with volatile elements to prevent constant folding */
    int32_t mask_arr[16];
    for (int i = 0; i < 16; i++) {
        mask_arr[i] = get_mask_index(i + mask_seed, 32);
    }
    v16si mask = *(v16si*)mask_arr;
    
    /* Complex control flow to stress expander */
    if (mask_seed % 3 == 0) {
        /* This __builtin_shuffle has 3 operands: vec1, vec2, mask = 10 total slots */
        v16si result = __builtin_shuffle(vec1, vec2, mask);
        
        /* Additional operation to use result */
        result = result + (v16si){1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
        
        /* Store back */
        *(v16si*)&dst[start_idx] = result;
    } else if (mask_seed % 3 == 1) {
        /* Alternative path with different shuffle pattern */
        v8si vec1_256 = *(v8si*)&src[start_idx];
        v8si vec2_256 = *(v8si*)&src[start_idx + 8];
        int32_t mask_arr_8[8];
        for (int i = 0; i < 8; i++) {
            mask_arr_8[i] = get_mask_index(i + mask_seed, 16);
        }
        v8si mask_256 = *(v8si*)mask_arr_8;
        
        v8si result = __builtin_shuffle(vec1_256, vec2_256, mask_256);
        result = result * (v8si){2,2,2,2,2,2,2,2};
        
        /* Expand back to 512-bit */
        v16si expanded = __builtin_shufflevector(result, result, 
            0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7);
        *(v16si*)&dst[start_idx] = expanded;
    } else {
        /* Third path with even more complex operation */
        v16si temp = vec1;
        for (int i = 0; i < 4; i++) {
            int32_t small_mask[16];
            for (int j = 0; j < 16; j++) {
                small_mask[j] = get_mask_index(j + i + mask_seed, 32);
            }
            v16si rot_mask = *(v16si*)small_mask;
            temp = __builtin_shuffle(temp, rot_mask);
        }
        *(v16si*)&dst[start_idx] = temp;
    }
}

/* Float version with 11+ operands */
void shuffle_float_11_operand(float* src, float* dst, int start_idx, volatile int mask_seed) {
    v16sf vec1 = *(v16sf*)&src[start_idx];
    v16sf vec2 = *(v16sf*)&src[start_idx + 16];
    
    /* Create two different masks */
    int32_t mask1_arr[16], mask2_arr[16];
    for (int i = 0; i < 16; i++) {
        mask1_arr[i] = get_mask_index(i + mask_seed, 32);
        mask2_arr[i] = get_mask_index(i + mask_seed + 7, 32);
    }
    v16si mask1 = *(v16si*)mask1_arr;
    v16si mask2 = *(v16si*)mask2_arr;
    
    /* Switch statement to create complex control flow */
    switch (mask_seed % 4) {
        case 0: {
            /* Double shuffle with arithmetic - could require many operand slots */
            v16sf shuffled1 = __builtin_shuffle(vec1, vec2, mask1);
            v16sf shuffled2 = __builtin_shuffle(vec1, vec2, mask2);
            v16sf result = shuffled1 * shuffled2 + vec1;
            *(v16sf*)&dst[start_idx] = result;
            break;
        }
        case 1: {
            /* Nested shuffles */
            v8sf vec1_256 = *(v8sf*)&src[start_idx];
            v8sf vec2_256 = *(v8sf*)&src[start_idx + 8];
            int32_t mask_arr_8[8];
            for (int i = 0; i < 8; i++) {
                mask_arr_8[i] = get_mask_index(i + mask_seed, 16);
            }
            v8si mask_256 = *(v8si*)mask_arr_8;
            
            v8sf temp = __builtin_shuffle(vec1_256, vec2_256, mask_256);
            
            /* Expand with shufflevector - 10 explicit indices + 2 vectors = 12 operands */
            v16sf expanded = __builtin_shufflevector(temp, temp,
                0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7);
            *(v16sf*)&dst[start_idx] = expanded;
            break;
        }
        case 2: {
            /* Mixed-type operation */
            v8df double_vec1 = *(v8df*)&src[start_idx];  /* Reinterpret as double */
            v8df double_vec2 = *(v8df*)&src[start_idx + 8];
            
            int64_t dbl_mask_arr[8];
            for (int i = 0; i < 8; i++) {
                dbl_mask_arr[i] = get_mask_index(i + mask_seed, 16);
            }
            v4di dbl_mask = *(v4di*)dbl_mask_arr;
            
            /* Shuffle with conversion */
            v4df shuffled_dbl = __builtin_shuffle(double_vec1, double_vec2, dbl_mask);
            
            /* Convert back and store */
            v8sf result = *(v8sf*)&shuffled_dbl;
            v16sf expanded = __builtin_shufflevector(result, result,
                0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7);
            *(v16sf*)&dst[start_idx] = expanded;
            break;
        }
        default: {
            /* Direct shuffle with volatile intermediate */
            volatile v16sf vol_vec = vec1;
            v16sf result = __builtin_shuffle(vol_vec, mask1);
            *(v16sf*)&dst[start_idx] = result;
            break;
        }
    }
}

/* Double precision version for AVX-512 */
#ifdef __AVX512F__
void shuffle_double_avx512(double* src, double* dst, int start_idx, volatile int mask_seed) {
    v8df vec1 = *(v8df*)&src[start_idx];
    v8df vec2 = *(v8df*)&src[start_idx + 8];
    
    int64_t mask_arr[8];
    for (int i = 0; i < 8; i++) {
        mask_arr[i] = get_mask_index(i + mask_seed, 16);
    }
    v8di mask = *(v8di*)mask_arr;
    
    /* Complex shuffle pattern that may require many operand slots */
    v8df result;
    if (mask_seed % 2 == 0) {
        result = __builtin_shuffle(vec1, vec2, mask);
    } else {
        /* Alternative: shufflevector with many explicit indices */
        result = __builtin_shufflevector(vec1, vec2,
            0,9,2,11,4,13,6,15);
    }
    
    /* Additional arithmetic to ensure the result is used */
    result = result * (v8df){1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5};
    
    *(v8df*)&dst[start_idx] = result;
}
#endif

/* SSE2 version for baseline */
#ifdef __SSE2__
void shuffle_sse2(int32_t* src, int32_t* dst, int start_idx, volatile int mask_seed) {
    typedef int32_t v4si __attribute__((vector_size(16)));
    v4si vec1 = *(v4si*)&src[start_idx];
    v4si vec2 = *(v4si*)&src[start_idx + 4];
    
    int32_t mask_arr[4];
    for (int i = 0; i < 4; i++) {
        mask_arr[i] = get_mask_index(i + mask_seed, 8);
    }
    v4si mask = *(v4si*)mask_arr;
    
    v4si result = __builtin_shuffle(vec1, vec2, mask);
    
    /* Chain multiple shuffles */
    for (int i = 0; i < 3; i++) {
        int32_t rot_mask[4];
        for (int j = 0; j < 4; j++) {
            rot_mask[j] = (mask_arr[j] + i) % 8;
        }
        v4si new_mask = *(v4si*)rot_mask;
        result = __builtin_shuffle(result, new_mask);
    }
    
    *(v4si*)&dst[start_idx] = result;
}
#endif

/* Main test function */
int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_data(seed);
    
    /* Perform multiple iterations with different shuffle patterns */
    for (int iter = 0; iter < 10; iter++) {
        volatile int mask_seed = iter * 17;
        
        /* Process integer data */
        for (int i = 0; i < 480; i += 32) {  /* 512-32 to avoid overflow */
            shuffle_int_10_operand((int32_t*)global_int_data, accumulator_int, i, mask_seed + i);
        }
        
        /* Process float data */
        for (int i = 0; i < 480; i += 32) {
            shuffle_float_11_operand((float*)global_float_data, accumulator_float, i, mask_seed + i + 1);
        }
        
        /* Architecture-specific paths */
#ifdef __SSE2__
        for (int i = 0; i < 496; i += 16) {
            shuffle_sse2((int32_t*)global_int_data, accumulator_int, i, mask_seed + i + 2);
        }
#endif
        
#ifdef __AVX512F__
        /* Process doubles if AVX-512 is available */
        double double_src[512];
        double double_dst[512];
        for (int i = 0; i < 512; i++) {
            double_src[i] = global_float_data[i];
            double_dst[i] = 0.0;
        }
        
        for (int i = 0; i < 496; i += 16) {
            shuffle_double_avx512(double_src, double_dst, i, mask_seed + i + 3);
        }
        
        /* Accumulate results back */
        for (int i = 0; i < 512; i++) {
            accumulator_float[i] += (float)double_dst[i];
        }
#endif
    }
    
    /* Compute checksums to prevent dead code elimination */
    int64_t int_sum = 0;
    double float_sum = 0.0;
    
    for (int i = 0; i < 512; i++) {
        int_sum += accumulator_int[i];
        float_sum += accumulator_float[i];
        
        /* Volatile store to memory barrier */
        if (i % 64 == 0) {
            volatile int32_t barrier = accumulator_int[i];
            (void)barrier;
        }
    }
    
    printf("Integer checksum: %ld\n", int_sum);
    printf("Float checksum: %f\n", float_sum);
    
    return 0;
}
