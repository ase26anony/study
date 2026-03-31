#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int global_seed;
int global_int_array[512];
float global_float_array[512];
int accumulator_int[512];
float accumulator_float[512];

/* Vector type definitions */
typedef int v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef float v8sf __attribute__((vector_size(32)));    /* 256-bit float */
typedef int v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef float v16sf __attribute__((vector_size(64)));   /* 512-bit float */
typedef int v4si __attribute__((vector_size(16)));      /* 128-bit integer */
typedef float v4sf __attribute__((vector_size(16)));    /* 128-bit float */

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    global_seed = seed;
    for (int i = 0; i < 512; i++) {
        global_int_array[i] = (i * 1103515245 + seed) % 1000;
        global_float_array[i] = ((i * 1103515245 + seed) % 1000) * 0.1f;
        accumulator_int[i] = 0;
        accumulator_float[i] = 0.0f;
    }
}

/* Function using __builtin_shuffle with many operands - targeting 10+ operand case */
#ifdef __AVX2__
void shuffle_10_operand_int(v16si* result, const int* data, volatile int mask_idx) {
    /* Load 4 vectors of data */
    v16si v1 = *(const v16si*)(data);
    v16si v2 = *(const v16si*)(data + 16);
    v16si v3 = *(const v16si*)(data + 32);
    v16si v4 = *(const v16si*)(data + 48);
    
    /* Create control mask with runtime-dependent indices */
    volatile int mask_vals[16];
    for (int i = 0; i < 16; i++) {
        mask_vals[i] = (mask_idx + i * 3) % 64;
    }
    
    /* Complex shuffle pattern requiring many operands */
    v16si mask = *(v16si*)mask_vals;
    
    /* This shuffle with 3 input vectors and mask should require many operand slots */
    v16si shuffled = __builtin_shuffle(v1, v2, v3, mask);
    
    /* Additional operation to ensure the result is used */
    *result = shuffled + v4;
}

void shuffle_11_operand_float(v16sf* result, const float* data, volatile int pattern) {
    /* Load multiple vectors */
    v16sf f1 = *(const v16sf*)(data);
    v16sf f2 = *(const v16sf*)(data + 16);
    v16sf f3 = *(const v16sf*)(data + 32);
    v16sf f4 = *(const v16sf*)(data + 48);
    
    /* Runtime-dependent mask generation */
    volatile int mask_arr[16];
    for (int i = 0; i < 16; i++) {
        mask_arr[i] = (pattern + i * 7) % 48;
    }
    
    v16si mask = *(v16si*)mask_arr;
    
    /* Complex shuffle operation - aiming for 11 operand expansion */
    v16sf temp1 = __builtin_shuffle(f1, f2, f3, mask);
    v16sf temp2 = __builtin_shuffle(f2, f3, f4, mask);
    
    /* Mixed operation to stress different optab paths */
    *result = temp1 * temp2 + f1;
}
#endif

#ifdef __AVX512F__
/* Even larger vector operations for AVX-512 */
typedef int v32si __attribute__((vector_size(128)));
typedef float v32sf __attribute__((vector_size(128)));

void avx512_shuffle_complex(v32si* result, const int* data, volatile int mode) {
    v32si v1 = *(const v32si*)(data);
    v32si v2 = *(const v32si*)(data + 32);
    
    volatile int mask32[32];
    for (int i = 0; i < 32; i++) {
        mask32[i] = (mode + i * 5) % 64;
    }
    
    v32si mask = *(v32si*)mask32;
    
    /* Large shuffle that may require many operand slots */
    v32si shuffled = __builtin_shuffle(v1, v2, mask);
    
    /* Store intermediate result to memory to prevent elimination */
    volatile v32si temp = shuffled;
    *result = temp;
}
#endif

/* Function with nested control flow around vector operations */
void conditional_shuffle_operations(int* acc, const int* data, volatile int condition) {
    v8si v1, v2, v3, result;
    
    /* Load data */
    v1 = *(const v8si*)(data);
    v2 = *(const v8si*)(data + 8);
    v3 = *(const v8si*)(data + 16);
    
    /* Complex control flow with vector operations */
    if (condition & 1) {
        volatile int mask1[8] = {0, 2, 4, 6, 1, 3, 5, 7};
        v8si mask = *(v8si*)mask1;
        
        switch (condition % 4) {
            case 0:
                result = __builtin_shuffle(v1, v2, mask);
                break;
            case 1:
                result = __builtin_shuffle(v2, v3, mask);
                break;
            case 2: {
                volatile int mask2[8] = {7, 6, 5, 4, 3, 2, 1, 0};
                v8si mask_rev = *(v8si*)mask2;
                result = __builtin_shuffle(v1, v3, mask_rev);
                break;
            }
            case 3:
                result = __builtin_shuffle(v1, v2, v3, *(v8si*)mask1);
                break;
        }
    } else {
        volatile int mask3[8] = {1, 0, 3, 2, 5, 4, 7, 6};
        v8si mask = *(v8si*)mask3;
        result = __builtin_shuffle(v1, mask);
    }
    
    /* Store result */
    *(v8si*)acc = result;
}

/* Mixed SIMD patterns with size conversions */
void mixed_size_shuffle(float* acc, const float* data, volatile int idx) {
    /* Work with 256-bit vectors */
    v8sf v256_1 = *(const v8sf*)(data);
    v8sf v256_2 = *(const v8sf*)(data + 8);
    
    /* Create runtime-dependent mask */
    volatile int mask8[8];
    for (int i = 0; i < 8; i++) {
        mask8[i] = (idx + i) % 16;
    }
    
    v8si mask = *(v8si*)mask8;
    
    /* Shuffle 256-bit vectors */
    v8sf shuffled256 = __builtin_shuffle(v256_1, v256_2, mask);
    
    /* Convert to 128-bit vectors for narrowing operation */
    v4sf v128_low = __builtin_convertvector(shuffled256, v4sf);
    v4sf v128_high = __builtin_convertvector(
        __builtin_shuffle(shuffled256, *(v8sf*)(data + 16), mask), 
        v4sf
    );
    
    /* Expand back to 256-bit */
    v8sf expanded = __builtin_shufflevector(v128_low, v128_high, 0, 1, 2, 3, 4, 5, 6, 7);
    
    /* Store result */
    *(v8sf*)acc = expanded;
}

/* Main test driver */
int main(int argc, char* argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    volatile int control = seed;
    int loop_count = 10;
    
    printf("Starting vector shuffle tests with seed: %d\n", seed);
    
    for (int iter = 0; iter < loop_count; iter++) {
        volatile int pattern = (control + iter * 17) % 256;
        
        /* Test different shuffle patterns based on iteration */
        if (iter % 3 == 0) {
            #ifdef __AVX2__
            v16si int_result;
            shuffle_10_operand_int(&int_result, global_int_array + iter * 4, pattern);
            /* Accumulate results */
            for (int i = 0; i < 16; i++) {
                accumulator_int[iter * 16 + i] += int_result[i];
            }
            #endif
        } 
        else if (iter % 3 == 1) {
            #ifdef __AVX2__
            v16sf float_result;
            shuffle_11_operand_float(&float_result, global_float_array + iter * 4, pattern);
            for (int i = 0; i < 16; i++) {
                accumulator_float[iter * 16 + i] += float_result[i];
            }
            #endif
        } 
        else {
            /* Conditional shuffle operations */
            conditional_shuffle_operations(
                accumulator_int + iter * 8,
                global_int_array + iter * 8,
                pattern
            );
            
            mixed_size_shuffle(
                accumulator_float + iter * 8,
                global_float_array + iter * 8,
                pattern
            );
        }
        
        #ifdef __AVX512F__
        if (iter % 4 == 0) {
            v32si avx512_result;
            avx512_shuffle_complex(&avx512_result, global_int_array + iter * 8, pattern);
            /* Use the result to prevent elimination */
            volatile int temp_sum = 0;
            for (int i = 0; i < 32; i++) {
                temp_sum += avx512_result[i];
            }
            accumulator_int[0] += temp_sum;
        }
        #endif
    }
    
    /* Compute final checksum */
    long long int_checksum = 0;
    float float_checksum = 0.0f;
    
    for (int i = 0; i < 512; i++) {
        int_checksum += accumulator_int[i];
        float_checksum += accumulator_float[i];
    }
    
    printf("Integer checksum: %lld\n", int_checksum);
    printf("Float checksum: %f\n", float_checksum);
    
    return 0;
}
