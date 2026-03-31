#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int global_seed;
int global_int_array[512] __attribute__((aligned(64)));
float global_float_array[512] __attribute__((aligned(64)));
int accumulator_int[512] __attribute__((aligned(64)));
float accumulator_float[512] __attribute__((aligned(64)));

/* Vector type definitions */
typedef int v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef float v8sf __attribute__((vector_size(32)));    /* 256-bit float */
typedef float v16sf __attribute__((aligned(64)));       /* 512-bit float */
typedef double v4df __attribute__((vector_size(32)));   /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));   /* 512-bit double */

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

/* Function using __builtin_shuffle with many operands (10+ total) */
#ifdef __AVX2__
void shuffle_10_operand_int(v16si* result, const v16si* a, const v16si* b, 
                           volatile int mask_val) {
    /* Create control mask from runtime value */
    int mask_arr[16];
    for (int i = 0; i < 16; i++) {
        mask_arr[i] = (mask_val + i) % 32;  /* Indices into combined 32-element vector */
    }
    
    /* Load mask as vector - volatile to prevent constant folding */
    volatile int* volatile_mask = mask_arr;
    v16si mask = *(const v16si*)volatile_mask;
    
    /* Complex control flow to stress expander */
    if (mask_val % 3 == 0) {
        /* This shuffle uses 10 operands: 2 input vectors + 8-element mask */
        *result = __builtin_shuffle(*a, *b, 
            mask[0], mask[1], mask[2], mask[3],
            mask[4], mask[5], mask[6], mask[7],
            mask[8], mask[9], mask[10], mask[11],
            mask[12], mask[13], mask[14], mask[15]);
    } else if (mask_val % 3 == 1) {
        /* Alternative shuffle pattern */
        *result = __builtin_shuffle(*a, *b,
            mask[15], mask[14], mask[13], mask[12],
            mask[11], mask[10], mask[9], mask[8],
            mask[7], mask[6], mask[5], mask[4],
            mask[3], mask[2], mask[1], mask[0]);
    } else {
        /* Third pattern - all even indices */
        *result = __builtin_shuffle(*a, *b,
            0, 2, 4, 6, 8, 10, 12, 14,
            16, 18, 20, 22, 24, 26, 28, 30);
    }
}
#endif

#ifdef __AVX512F__
void shuffle_11_operand_float(v16sf* result, const v16sf* a, const v16sf* b,
                             const v16sf* c, volatile int mask_val) {
    /* Create two different masks from runtime values */
    int mask1_arr[16], mask2_arr[16];
    for (int i = 0; i < 16; i++) {
        mask1_arr[i] = (mask_val * 7 + i * 3) % 48;
        mask2_arr[i] = (mask_val * 11 + i * 5) % 48;
    }
    
    volatile int* volatile_mask1 = mask1_arr;
    volatile int* volatile_mask2 = mask2_arr;
    v16si mask1 = *(const v16si*)volatile_mask1;
    v16si mask2 = *(const v16si*)volatile_mask2;
    
    /* Switch statement with multiple shuffle patterns */
    switch (mask_val % 4) {
        case 0: {
            /* Complex shuffle requiring 11+ operands */
            v16sf temp = __builtin_shuffle(*a, *b,
                mask1[0], mask1[1], mask1[2], mask1[3],
                mask1[4], mask1[5], mask1[6], mask1[7],
                mask1[8], mask1[9], mask1[10], mask1[11],
                mask1[12], mask1[13], mask1[14], mask1[15]);
            
            *result = __builtin_shuffle(temp, *c,
                mask2[0], mask2[1], mask2[2], mask2[3],
                mask2[4], mask2[5], mask2[6], mask2[7],
                mask2[8], mask2[9], mask2[10], mask2[11],
                mask2[12], mask2[13], mask2[14], mask2[15]);
            break;
        }
        case 1: {
            /* Direct 3-input shuffle simulation */
            v16sf temp1 = __builtin_shuffle(*a, *b,
                0, 2, 4, 6, 8, 10, 12, 14,
                16, 18, 20, 22, 24, 26, 28, 30);
            v16sf temp2 = __builtin_shuffle(*b, *c,
                1, 3, 5, 7, 9, 11, 13, 15,
                17, 19, 21, 23, 25, 27, 29, 31);
            
            *result = temp1 + temp2;  /* Arithmetic to prevent elimination */
            break;
        }
        case 2: {
            /* Nested shuffles in loop */
            v16sf temp = *a;
            for (int i = 0; i < 3; i++) {
                temp = __builtin_shuffle(temp, *b,
                    mask1[(i*5)%16], mask1[(i*5+1)%16], mask1[(i*5+2)%16], mask1[(i*5+3)%16],
                    mask1[(i*5+4)%16], mask1[(i*5+5)%16], mask1[(i*5+6)%16], mask1[(i*5+7)%16],
                    mask1[(i*5+8)%16], mask1[(i*5+9)%16], mask1[(i*5+10)%16], mask1[(i*5+11)%16],
                    mask1[(i*5+12)%16], mask1[(i*5+13)%16], mask1[(i*5+14)%16], mask1[(i*5+15)%16]);
            }
            *result = temp;
            break;
        }
        default: {
            /* Simple pattern */
            *result = __builtin_shuffle(*a, *b,
                15, 14, 13, 12, 11, 10, 9, 8,
                7, 6, 5, 4, 3, 2, 1, 0);
            break;
        }
    }
}
#endif

/* Mixed SIMD patterns with narrowing/expanding */
#ifdef __AVX2__
void mixed_simd_patterns(int idx, volatile int mask_val) {
    /* Load 512-bit data as two 256-bit vectors */
    v8si* int_data = (v8si*)&global_int_array[idx * 16];
    v8sf* float_data = (v8sf*)&global_float_array[idx * 16];
    
    v8si vec1 = int_data[0];
    v8si vec2 = int_data[1];
    v8sf fvec1 = float_data[0];
    v8sf fvec2 = float_data[1];
    
    /* Create mask from runtime value */
    int mask_arr[8];
    for (int i = 0; i < 8; i++) {
        mask_arr[i] = (mask_val * 3 + i * 7) % 16;
    }
    volatile int* volatile_mask = mask_arr;
    v8si mask = *(const v8si*)volatile_mask;
    
    /* Shuffle with 10 operands: 2 vectors + 8-element mask */
    v8si shuffled_int;
    if (mask_val % 2 == 0) {
        shuffled_int = __builtin_shuffle(vec1, vec2,
            mask[0], mask[1], mask[2], mask[3],
            mask[4], mask[5], mask[6], mask[7]);
    } else {
        shuffled_int = __builtin_shuffle(vec2, vec1,
            mask[7], mask[6], mask[5], mask[4],
            mask[3], mask[2], mask[1], mask[0]);
    }
    
    /* Convert and mix with float operations */
    v8sf shuffled_float = __builtin_convertvector(shuffled_int, v8sf);
    v8sf result_float = fvec1 + shuffled_float * fvec2;
    
    /* Store to accumulator with volatile write */
    volatile v8sf* volatile_store = (v8sf*)&accumulator_float[idx * 8];
    *volatile_store = *volatile_store + result_float;
}
#endif

/* Architecture-neutral function using GCC vector extensions */
void generic_vector_shuffle(int idx, volatile int pattern) {
    /* Use smaller vectors that work on most architectures */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si* int_data = (v4si*)&global_int_array[idx * 4];
    v4sf* float_data = (v4sf*)&global_float_array[idx * 4];
    
    v4si vec1 = int_data[0];
    v4si vec2 = int_data[1];
    v4sf fvec1 = float_data[0];
    v4sf fvec2 = float_data[1];
    
    /* Create mask from pattern */
    int mask_arr[4];
    for (int i = 0; i < 4; i++) {
        mask_arr[i] = (pattern + i) % 8;
    }
    
    /* Multiple shuffle patterns in control flow */
    v4si shuffled;
    switch (pattern % 3) {
        case 0:
            shuffled = __builtin_shuffle(vec1, vec2, 
                mask_arr[0], mask_arr[1], mask_arr[2], mask_arr[3]);
            break;
        case 1:
            shuffled = __builtin_shuffle(vec2, vec1,
                mask_arr[3], mask_arr[2], mask_arr[1], mask_arr[0]);
            break;
        case 2:
            shuffled = __builtin_shuffle(vec1, vec2,
                0, 2, 4, 6);
            break;
    }
    
    /* Arithmetic to prevent dead code elimination */
    v4si result = vec1 + shuffled * vec2;
    
    /* Store result */
    v4si* acc = (v4si*)&accumulator_int[idx * 4];
    *acc = *acc + result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    init_arrays(seed);
    
    volatile int runtime_mask = seed;
    
    /* Main loop with different shuffle operations */
    for (int iter = 0; iter < 10; iter++) {
        runtime_mask = (runtime_mask * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call architecture-specific functions */
#ifdef __AVX512F__
        if (iter % 3 == 0) {
            v16si int_vec1 = *(v16si*)&global_int_array[iter * 32];
            v16si int_vec2 = *(v16si*)&global_int_array[iter * 32 + 16];
            v16si int_vec3 = *(v16si*)&global_int_array[iter * 32 + 32];
            v16si int_result;
            
            shuffle_10_operand_int(&int_result, &int_vec1, &int_vec2, runtime_mask);
            
            /* Store with volatile to prevent optimization */
            volatile v16si* volatile_store = (v16si*)&accumulator_int[iter * 32];
            *volatile_store = *volatile_store + int_result;
        }
        
        if (iter % 3 == 1) {
            v16sf float_vec1 = *(v16sf*)&global_float_array[iter * 32];
            v16sf float_vec2 = *(v16sf*)&global_float_array[iter * 32 + 16];
            v16sf float_vec3 = *(v16sf*)&global_float_array[iter * 32 + 32];
            v16sf float_result;
            
            shuffle_11_operand_float(&float_result, &float_vec1, &float_vec2, 
                                   &float_vec3, runtime_mask);
            
            volatile v16sf* volatile_store = (v16sf*)&accumulator_float[iter * 32];
            *volatile_store = *volatile_store + float_result;
        }
#endif

#ifdef __AVX2__
        if (iter % 3 == 2) {
            mixed_simd_patterns(iter % 16, runtime_mask);
        }
#endif
        
        /* Always call generic function */
        generic_vector_shuffle(iter % 32, runtime_mask);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long int_checksum = 0;
    float float_checksum = 0.0f;
    
    for (int i = 0; i < 512; i++) {
        int_checksum += accumulator_int[i];
        float_checksum += accumulator_float[i];
        
        /* Additional volatile access */
        volatile int temp = accumulator_int[i];
        (void)temp;
    }
    
    printf("Integer checksum: %lld\n", int_checksum);
    printf("Float checksum: %f\n", float_checksum);
    
    return 0;
}
