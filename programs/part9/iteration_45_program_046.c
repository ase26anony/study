#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
#define ARRAY_SIZE 512
static int32_t global_int_array[ARRAY_SIZE];
static float global_float_array[ARRAY_SIZE];
static volatile int32_t volatile_int_array[ARRAY_SIZE];
static volatile float volatile_float_array[ARRAY_SIZE];

/* Accumulator arrays for results */
static int32_t int_accumulator[ARRAY_SIZE] = {0};
static float float_accumulator[ARRAY_SIZE] = {0.0f};

/* Vector type definitions using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef float v8sf __attribute__((vector_size(32)));        /* 256-bit float */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */
typedef int32_t v4si __attribute__((vector_size(16)));      /* 128-bit integer */
typedef float v4sf __attribute__((vector_size(16)));        /* 128-bit float */

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    srand(seed);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = rand() % 1000;
        global_float_array[i] = (float)(rand() % 1000) / 10.0f;
        volatile_int_array[i] = global_int_array[i];
        volatile_float_array[i] = global_float_array[i];
    }
}

/* Function 1: Complex shuffle with 10+ operands using __builtin_shuffle */
#ifdef __AVX2__
static void shuffle_10_operand_int(v16si* result, const int32_t* data, int mask_seed) {
    /* Load data into vectors - using volatile to prevent constant folding */
    volatile v16si v1 = *(const v16si*)(data);
    volatile v16si v2 = *(const v16si*)(data + 16);
    volatile v16si v3 = *(const v16si*)(data + 32);
    volatile v16si v4 = *(const v16si*)(data + 48);
    
    /* Create control mask with runtime-dependent indices */
    int32_t mask_arr[16];
    for (int i = 0; i < 16; i++) {
        mask_arr[i] = (mask_seed + i * 3) % 64;  /* Spread across all 4 vectors */
    }
    volatile v16si mask = *(v16si*)mask_arr;
    
    /* Complex shuffle operation that may require 10+ operand expansion */
    v16si temp1 = __builtin_shuffle(v1, v2, mask);
    v16si temp2 = __builtin_shuffle(v3, v4, mask);
    
    /* Another shuffle combining results - potentially hitting 11 operands */
    int32_t mask2_arr[16];
    for (int i = 0; i < 16; i++) {
        mask2_arr[i] = (mask_seed + i * 7) % 32;
    }
    volatile v16si mask2 = *(v16si*)mask2_arr;
    
    v16si final_result = __builtin_shuffle(temp1, temp2, mask2);
    
    /* Store to volatile memory to prevent DCE */
    *(volatile v16si*)volatile_int_array = final_result;
    
    *result = final_result;
}
#endif

/* Function 2: Mixed float/int shuffles with many operands */
#ifdef __AVX512F__
static void shuffle_11_operand_mixed(v16sf* float_result, v16si* int_result, 
                                     const float* fdata, const int32_t* idata, 
                                     int mask_seed) {
    /* Load float vectors */
    volatile v16sf fv1 = *(const v16sf*)(fdata);
    volatile v16sf fv2 = *(const v16sf*)(fdata + 16);
    volatile v16sf fv3 = *(const v16sf*)(fdata + 32);
    
    /* Load int vectors */
    volatile v16si iv1 = *(const v16si*)(idata);
    volatile v16si iv2 = *(const v16si*)(idata + 16);
    
    /* Create complex masks */
    int32_t float_mask_arr[16];
    int32_t int_mask_arr[16];
    for (int i = 0; i < 16; i++) {
        float_mask_arr[i] = (mask_seed + i * 5) % 48;
        int_mask_arr[i] = (mask_seed + i * 11) % 32;
    }
    
    volatile v16si float_mask = *(v16si*)float_mask_arr;
    volatile v16si int_mask = *(v16si*)int_mask_arr;
    
    /* Series of shuffles that may require many operands */
    v16sf f_shuffled1 = __builtin_shuffle(fv1, fv2, float_mask);
    v16sf f_shuffled2 = __builtin_shuffle(fv3, f_shuffled1, float_mask);
    
    v16si i_shuffled1 = __builtin_shuffle(iv1, iv2, int_mask);
    
    /* Store intermediate results to volatile */
    *(volatile v16sf*)volatile_float_array = f_shuffled2;
    *(volatile v16si*)(volatile_int_array + 32) = i_shuffled1;
    
    /* Additional shuffle with converted types */
    v16si converted = __builtin_convertvector(f_shuffled2, v16si);
    
    /* Final complex shuffle that may hit 11 operands */
    int32_t final_mask_arr[16];
    for (int i = 0; i < 16; i++) {
        final_mask_arr[i] = (mask_seed + i * 13) % 64;
    }
    volatile v16si final_mask = *(v16si*)final_mask_arr;
    
    v16si final_int = __builtin_shuffle(i_shuffled1, converted, final_mask);
    v16sf final_float = __builtin_shuffle(f_shuffled1, f_shuffled2, final_mask);
    
    *float_result = final_float;
    *int_result = final_int;
}
#endif

/* Function 3: Narrowing/expanding shuffles with control flow */
#ifdef __SSE2__
static v8si narrowing_shuffle_with_control_flow(const int32_t* data, int pattern) {
    volatile v8si v1 = *(const v8si*)(data);
    volatile v8si v2 = *(const v8si*)(data + 8);
    
    /* Control flow that depends on volatile input */
    int32_t mask_arr[8];
    if (pattern % 3 == 0) {
        for (int i = 0; i < 8; i++) mask_arr[i] = (i + pattern) % 16;
    } else if (pattern % 3 == 1) {
        for (int i = 0; i < 8; i++) mask_arr[i] = (i * 2 + pattern) % 16;
    } else {
        for (int i = 0; i < 8; i++) mask_arr[i] = (i * 3 + pattern) % 16;
    }
    
    volatile v8si mask = *(v8si*)mask_arr;
    
    /* Switch statement with different shuffle patterns */
    v8si result;
    switch (pattern % 4) {
        case 0:
            result = __builtin_shuffle(v1, v2, mask);
            break;
        case 1: {
            /* More complex shuffle pattern */
            int32_t mask2_arr[8];
            for (int i = 0; i < 8; i++) mask2_arr[i] = (mask_arr[i] + 8) % 16;
            volatile v8si mask2 = *(v8si*)mask2_arr;
            v8si temp = __builtin_shuffle(v1, v2, mask);
            result = __builtin_shuffle(temp, v1, mask2);
            break;
        }
        case 2:
            result = v1 + v2;  /* Simple operation */
            break;
        case 3:
            result = __builtin_shuffle(v2, v1, mask);
            break;
    }
    
    return result;
}
#endif

/* Function 4: __builtin_shufflevector with many arguments */
static void shufflevector_many_args(v8sf* result, const float* data, int seed) {
    /* Load multiple vectors */
    volatile v8sf v1 = *(const v8sf*)(data);
    volatile v8sf v2 = *(const v8sf*)(data + 8);
    volatile v8sf v3 = *(const v8sf*)(data + 16);
    volatile v8sf v4 = *(const v8sf*)(data + 24);
    
    /* __builtin_shufflevector can take many arguments */
    /* This creates a 16-element vector from 4 8-element vectors */
    typedef float v16sf_small __attribute__((vector_size(64)));
    
    /* Use shufflevector with 10+ arguments (4 input vectors + 16 indices) */
    v16sf_small large_vec = __builtin_shufflevector(v1, v2, v3, v4,
        0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15);
    
    /* Extract back to 8-element vector */
    *result = *(v8sf*)&large_vec;
}

/* Main test function with loops and control flow */
int main(int argc, char* argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    int iterations = 10;
    uint64_t int_checksum = 0;
    float float_checksum = 0.0f;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Use volatile variable for loop-dependent masks */
        volatile int mask_seed = iter * 17;
        
#ifdef __AVX2__
        /* Test 10+ operand integer shuffles */
        v16si int_result;
        shuffle_10_operand_int(&int_result, global_int_array + iter * 4, mask_seed);
        
        /* Accumulate results */
        for (int i = 0; i < 16; i++) {
            int idx = (iter * 16 + i) % ARRAY_SIZE;
            int_accumulator[idx] += int_result[i];
        }
#endif
        
#ifdef __AVX512F__
        /* Test 11+ operand mixed shuffles */
        v16sf float_result;
        v16si mixed_int_result;
        shuffle_11_operand_mixed(&float_result, &mixed_int_result,
                                global_float_array + iter * 8,
                                global_int_array + iter * 8,
                                mask_seed);
        
        /* Accumulate both float and int results */
        for (int i = 0; i < 16; i++) {
            int idx = (iter * 16 + i) % ARRAY_SIZE;
            float_accumulator[idx] += float_result[i];
            int_accumulator[idx] += mixed_int_result[i];
        }
#endif
        
#ifdef __SSE2__
        /* Test narrowing shuffles with control flow */
        v8si narrow_result = narrowing_shuffle_with_control_flow(
            global_int_array + iter * 16, mask_seed);
        
        for (int i = 0; i < 8; i++) {
            int idx = (iter * 8 + i) % ARRAY_SIZE;
            int_accumulator[idx] += narrow_result[i];
        }
#endif
        
        /* Test shufflevector with many arguments */
        v8sf shufflevec_result;
        shufflevector_many_args(&shufflevec_result, 
                               global_float_array + iter * 32, 
                               mask_seed);
        
        for (int i = 0; i < 8; i++) {
            int idx = (iter * 8 + i) % ARRAY_SIZE;
            float_accumulator[idx] += shufflevec_result[i];
        }
    }
    
    /* Compute final checksums */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_checksum += int_accumulator[i];
        float_checksum += float_accumulator[i];
    }
    
    printf("Integer checksum: %lu\n", int_checksum);
    printf("Float checksum: %f\n", float_checksum);
    
    return 0;
}
