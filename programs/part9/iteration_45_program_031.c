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
    global_seed = seed;
    for (int i = 0; i < 512; i++) {
        global_int_array[i] = (i * 1103515245 + seed) % 1000;
        global_float_array[i] = ((i * 1103515245 + seed) % 1000) * 0.1f;
    }
}

/* Function 1: Complex shuffle with 10+ operands using int32x16_t equivalent */
__attribute__((noinline))
void shuffle_int_16lane(v16si *result, int offset, volatile int mask_idx) {
    v16si a, b, c, d;
    
    /* Load data from global arrays */
    for (int i = 0; i < 16; i++) {
        ((int*)&a)[i] = global_int_array[offset + i];
        ((int*)&b)[i] = global_int_array[offset + 16 + i];
        ((int*)&c)[i] = global_int_array[offset + 32 + i];
        ((int*)&d)[i] = global_int_array[offset + 48 + i];
    }
    
    /* Create control mask with runtime-dependent indices */
    int mask_arr[16];
    for (int i = 0; i < 16; i++) {
        mask_arr[i] = (i + mask_idx) % 32;
    }
    v16si mask = *(v16si*)mask_arr;
    
    /* Complex nested control flow to stress expander */
    if (mask_idx & 1) {
        /* First shuffle pattern - requires 10 operands during expansion */
        v16si temp1 = __builtin_shuffle(a, b, mask);
        v16si temp2 = __builtin_shuffle(c, d, mask);
        
        /* Second shuffle with different pattern */
        int mask2_arr[16];
        for (int i = 0; i < 16; i++) {
            mask2_arr[i] = (mask_arr[i] + 8) % 32;
        }
        v16si mask2 = *(v16si*)mask2_arr;
        
        /* Combined shuffle - this may expand to many operands */
        *result = __builtin_shuffle(temp1, temp2, mask2);
    } else {
        /* Alternative path with shufflevector */
        switch (mask_idx % 4) {
            case 0: {
                /* shufflevector with many arguments */
                *result = __builtin_shufflevector(a, b, c, d,
                    0, 16, 1, 17, 2, 18, 3, 19,
                    4, 20, 5, 21, 6, 22, 7, 23);
                break;
            }
            case 1: {
                /* Different pattern */
                *result = __builtin_shufflevector(a, b, c, d,
                    8, 24, 9, 25, 10, 26, 11, 27,
                    12, 28, 13, 29, 14, 30, 15, 31);
                break;
            }
            default: {
                /* Dynamic pattern based on mask_idx */
                int pattern[16];
                for (int i = 0; i < 16; i++) {
                    pattern[i] = (i * mask_idx) % 32;
                }
                *result = __builtin_shuffle(a, b, c, d, *(v16si*)pattern);
            }
        }
    }
    
    /* Prevent dead code elimination */
    volatile v16si vtemp = *result;
    (void)vtemp;
}

/* Function 2: Float shuffle with mixed operations */
__attribute__((noinline))
void shuffle_float_8lane(v8sf *result, int offset, volatile int pattern) {
    v8sf a, b, c, d;
    
    /* Load float data */
    for (int i = 0; i < 8; i++) {
        ((float*)&a)[i] = global_float_array[offset + i];
        ((float*)&b)[i] = global_float_array[offset + 8 + i];
        ((float*)&c)[i] = global_float_array[offset + 16 + i];
        ((float*)&d)[i] = global_float_array[offset + 24 + i];
    }
    
    /* Complex control flow with multiple shuffle patterns */
    if (pattern > 100) {
        /* Pattern A: Interleave shuffles */
        v8sf mask1 = {7, 6, 5, 4, 3, 2, 1, 0};
        v8sf temp1 = __builtin_shuffle(a, b, mask1);
        v8sf temp2 = __builtin_shuffle(c, d, mask1);
        
        v8sf mask2 = {0, 8, 1, 9, 2, 10, 3, 11};
        *result = __builtin_shuffle(temp1, temp2, mask2);
    } else {
        /* Pattern B: Complex shufflevector chain */
        v8sf temp = __builtin_shufflevector(a, b, c, d,
            0, 8, 1, 9, 2, 10, 3, 11);
        
        /* Additional arithmetic to create more complex RTL */
        *result = temp + (v8sf){1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    }
}

/* Function 3: Double precision shuffle with narrowing/expanding */
#ifdef __AVX2__
__attribute__((noinline))
void shuffle_double_4lane(v4df *result, int offset, volatile int mode) {
    v4df a, b;
    
    /* Load double data */
    for (int i = 0; i < 4; i++) {
        ((double*)&a)[i] = (double)global_float_array[offset + i * 2];
        ((double*)&b)[i] = (double)global_float_array[offset + i * 2 + 8];
    }
    
    /* Switch with multiple shuffle patterns */
    switch (mode % 5) {
        case 0: {
            /* Simple reverse */
            v4df mask = {3, 2, 1, 0};
            *result = __builtin_shuffle(a, b, mask);
            break;
        }
        case 1: {
            /* Interleave */
            v4df mask = {0, 4, 1, 5};
            *result = __builtin_shuffle(a, b, mask);
            break;
        }
        case 2: {
            /* Complex pattern that may require many operands */
            v4df temp1 = __builtin_shuffle(a, a, (v4df){1, 0, 3, 2});
            v4df temp2 = __builtin_shuffle(b, b, (v4df){3, 2, 1, 0});
            v4df mask = {0, 4, 2, 6};
            *result = __builtin_shuffle(temp1, temp2, mask);
            break;
        }
        default: {
            /* Dynamic pattern */
            int idx = mode & 3;
            v4df masks[4] = {
                {0, 1, 2, 3},
                {3, 2, 1, 0},
                {0, 4, 1, 5},
                {2, 6, 3, 7}
            };
            *result = __builtin_shuffle(a, b, masks[idx]);
        }
    }
}
#endif

/* Function 4: Mixed-type operations with large vectors */
#ifdef __AVX512F__
__attribute__((noinline))
void shuffle_mixed_512bit(v16si *int_result, v16sf *float_result, 
                          int offset, volatile int selector) {
    v16si int_vec;
    v16sf float_vec;
    
    /* Load mixed data */
    for (int i = 0; i < 16; i++) {
        ((int*)&int_vec)[i] = global_int_array[offset + i];
        ((float*)&float_vec)[i] = global_float_array[offset + i];
    }
    
    /* Complex shuffle patterns in loop */
    for (int i = 0; i < 3; i++) {
        if ((selector >> i) & 1) {
            /* Integer shuffle with many operands */
            int mask_arr[16];
            for (int j = 0; j < 16; j++) {
                mask_arr[j] = (j + i + selector) % 16;
            }
            v16si mask = *(v16si*)mask_arr;
            int_vec = __builtin_shuffle(int_vec, mask);
            
            /* Float shuffle with different pattern */
            float maskf_arr[16];
            for (int j = 0; j < 16; j++) {
                maskf_arr[j] = (float)((j * 3 + i) % 16);
            }
            v16sf maskf = *(v16sf*)maskf_arr;
            float_vec = __builtin_shuffle(float_vec, maskf);
        }
    }
    
    *int_result = int_vec;
    *float_result = float_vec;
}
#endif

/* Main test driver */
int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    volatile int runtime_var = seed;
    int checksum = 0;
    
    /* Main test loop */
    for (int iter = 0; iter < 10; iter++) {
        int offset = (iter * 37) % 256;
        
        /* Test 1: Large integer vector shuffle */
        v16si int_result;
        shuffle_int_16lane(&int_result, offset, runtime_var + iter);
        
        /* Accumulate results */
        for (int i = 0; i < 16; i++) {
            accumulator[offset + i] += ((int*)&int_result)[i];
        }
        
        /* Test 2: Float vector shuffle */
        v8sf float_result;
        shuffle_float_8lane(&float_result, offset, runtime_var ^ iter);
        
        /* Convert and accumulate float results */
        for (int i = 0; i < 8; i++) {
            accumulator[offset + 100 + i] += (int)((float*)&float_result)[i];
        }
        
#ifdef __AVX2__
        /* Test 3: Double precision shuffle */
        v4df double_result;
        shuffle_double_4lane(&double_result, offset, runtime_var * iter);
        
        for (int i = 0; i < 4; i++) {
            accumulator[offset + 200 + i] += (int)((double*)&double_result)[i];
        }
#endif
        
#ifdef __AVX512F__
        /* Test 4: Mixed 512-bit operations */
        v16si int_result512;
        v16sf float_result512;
        shuffle_mixed_512bit(&int_result512, &float_result512, 
                           offset, runtime_var + iter * 7);
        
        for (int i = 0; i < 16; i++) {
            accumulator[offset + 300 + i] += ((int*)&int_result512)[i];
            accumulator[offset + 400 + i] += (int)((float*)&float_result512)[i];
        }
#endif
        
        /* Modify runtime variable to affect control flow */
        runtime_var = (runtime_var * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Compute final checksum */
    for (int i = 0; i < 512; i++) {
        checksum = (checksum * 31 + accumulator[i]) & 0x7fffffff;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
